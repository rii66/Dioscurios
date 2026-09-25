
#include <Arduino.h>

#include "pid.h"
#include "pwm.h"
#include "tip.h"
#include "ptc.h"
#include "boost.h"

#include "GlobalState.h"

//====================================================================//
// CONFIGS & CALIBRATION
//====================================================================//
#if defined(ARDUINO_ARCH_RP2040)
const uint8_t  ADC_SAMPLES_PID      = 6;      // PID: cepat, cukup stabil
const uint8_t  ADC_SAMPLES_DETECT   = 8;      // Tip detect: sedikit lebih halus
const uint16_t ADC_SAMPLE_DELAY_US  = 50;
const uint16_t SETTLING_DELAY_US    = 150;
#else
const uint8_t  ADC_SAMPLES_PID      = 8;
const uint8_t  ADC_SAMPLES_DETECT   = 10;
const uint16_t ADC_SAMPLE_DELAY_US  = 150;
const uint16_t SETTLING_DELAY_US    = 250;
#endif

// EMA: currentTemp = alpha * new + (1-alpha) * old
// 0.30 = responsif; naikkan (0.4–0.5) jika masih lambat, turunkan jika masih noisy
const float    TEMP_EMA_ALPHA       = 0.30f;

const int      HEATER_HYSTERESIS    = 5;
const int      PID_INTEGRAL_LIMIT   = 500;

//====================================================================//
// PID INTERNAL & TARGET
//====================================================================//
float pidError      = 0;
float pidIntegral   = 0;
float pidDerivative = 0;
float lastError     = 0;

static float tempEma     = 0.0f;  // filter suhu
static bool  tempEmaInit = false;

//====================================================================//
// CORE ADC READER — sample count dipilih pemanggil
//====================================================================//
uint16_t getAverageADC(uint8_t samples) {
    if (samples < 1) samples = 1;
    uint32_t total = 0;
    for (uint8_t i = 0; i < samples; i++) {
        total += analogRead(hermenexConfig.tempPin);
        delayMicroseconds(ADC_SAMPLE_DELAY_US);
    }
    return (uint16_t)(total / samples);
}

// Kompatibilitas pemanggil lama tanpa argumen → PID samples
uint16_t getAverageADC() {
    return getAverageADC(ADC_SAMPLES_PID);
}

//====================================================================//
// MULTI-TIP CALIBRATION ENGINE (UNIVERSAL & DYNAMIC)
//====================================================================//
int adcToTemp(uint16_t rawAdc) {

    if (activeTip == nullptr)
        return 0;

    // Sensor PTC
    if (activeTip->tipID == TIP_CUSTOM) {
        return ptcToTemp(rawAdc);
    }

    // Sensor T12 / C210
    int adc = rawAdc + activeTip->adcOffset;

    float temp =
        (adc * activeTip->slope) +
        activeTip->tempOffset;

    return (int)temp;
}

//====================================================================//
// SAFETY
//====================================================================//
void handleSafety() {
overHeat = (currentTemp > maxTemp);
}

//====================================================================//
// READ TEMP
//====================================================================//
int readTemp() {
    startTempRead();
    uint16_t rawAdc = getAverageADC(ADC_SAMPLES_PID);
    endTempRead();
    return adcToTemp(rawAdc);
}

//====================================================================//
// PID UPDATE
//====================================================================//
void updatePID() {
    int rawTemp = readTemp();

    // EMA filter — meredam noise LMV358/ADC tanpa lag berlebih
    if (!tempEmaInit || tipError || activeTip == nullptr) {
        tempEma = (float)rawTemp;
        tempEmaInit = (activeTip != nullptr && !tipError);
    } else {
        tempEma = TEMP_EMA_ALPHA * (float)rawTemp
                + (1.0f - TEMP_EMA_ALPHA) * tempEma;
    }
    currentTemp = (int)(tempEma + 0.5f);

    handleSafety();

// ===== SLEEP LOCK =====
// CastorS remains able to read temperature, but the heater must stay OFF.
if (sleeping) {
    pwmOut      = 0;
    pidIntegral = 0;
    lastError   = 0;
    heaterOff();
    return;
}

// ===== SAFETY LOCK =====  
if (tipError || overHeat || activeTip == nullptr || heaterState == STATE_TIP) {  
    pwmOut      = 0;  
    pidIntegral = 0;  
    lastError   = 0;  
    heaterOff();  
    return;  
}  

// ===== TARGET SELECT =====  
int activeTarget = targetTemp;  
if (sleeping)  activeTarget = sleepTemp;  
if (boostMode) activeTarget = boostTemp;  

// ===== HEATER STATE =====  
if (currentTemp < (activeTarget - HEATER_HYSTERESIS)) {  
    heaterState = STATE_HEAT;  
} else {  
    heaterState = STATE_HOLD;  
}  

// ===== PID =====  
pidError      = activeTarget - currentTemp;  
  
// Integral limit menggunakan konstanta PID_INTEGRAL_LIMIT  
pidIntegral   = constrain(pidIntegral + pidError, -PID_INTEGRAL_LIMIT, PID_INTEGRAL_LIMIT);  
pidDerivative = pidError - lastError;  

float activeKp = activeTip->kp;
float activeKi = activeTip->ki;
float activeKd = activeTip->kd;

float output =
    (activeKp * pidError) +
    (activeKi * pidIntegral) +
    (activeKd * pidDerivative);

lastError     = pidError;  

pwmOut = constrain((int)output, 0, hermenexConfig.pwmMaxVal);
setPWM(pwmOut);


}

//====================================================================//
// AUTO / CUSTOM TIP DETECT
//====================================================================//
void detectTip() {

    heaterOff();
    delayMicroseconds(SETTLING_DELAY_US);

    uint16_t sensorValue = getAverageADC(ADC_SAMPLES_DETECT);
    TipConfig *foundTip = nullptr;  

    // ====================================================
    // 1. DETEKSI TIDAK ADA TIP (Mendukung Thermocouple & PTC)
    // ====================================================
    // Jika mode Custom/PTC dan nilai melonjak ke atas (>= 4000)
    // ATAU jika mode T12/C210 dan nilai drop ke bawah (<= 260)
    if (sensorValue >= hermenexConfig.adcNoTipPtc || sensorValue <= hermenexConfig.adcNoTip) {

        activeTip = nullptr;
        detectedTip = TIP_AUTO;
        tipError = true;
        tempEmaInit = false;

        pidIntegral = 0;
        lastError = 0;

        heaterState = STATE_TIP;
        heaterOff();
        return;
    }

    // CUSTOM (PTC)
    if (currentTipMode == 3) {
        foundTip = &customTipProfile;
    }

    // AUTO / T12 / C210
    else {
        for (int i = 0; i < TOTAL_SUPPORTED_TIPS; i++) {

            // Mode manual T12
            if (currentTipMode == 0 &&
                tipDatabase[i].tipID != TIP_T12)
                continue;

            // Mode manual C210
            if (currentTipMode == 1 &&
                tipDatabase[i].tipID != TIP_C210)
                continue;

            // AUTO akan memeriksa semua profile
            if (sensorValue >= tipDatabase[i].minADC &&
                sensorValue <= tipDatabase[i].maxADC) {

                foundTip = &tipDatabase[i];
                break;
            }
        }
    }

    // ====================================================  
    // 2. ERROR HANDLING TERPUSAT  
    // ====================================================  
    if (foundTip == nullptr) {  
        activeTip   = nullptr;  
        detectedTip = TIP_AUTO;  
        tipError    = true;  
        heaterState = STATE_TIP;  
          
        pidIntegral = 0;  
        lastError   = 0;  
          
        heaterOff();  
        return;  
    }  

    // ====================================================  
    // 3. APPLY PROFILE  
    // ====================================================  
    detectedTip = foundTip->tipID;  

    if (foundTip != activeTip) {  
        applyTipProfile(foundTip);  
        pidIntegral = 0;  
        lastError   = 0;  
    }  

    tipError = false;  

    if (heaterState == STATE_TIP) {  
        heaterState = STATE_HEAT;  
    }
}

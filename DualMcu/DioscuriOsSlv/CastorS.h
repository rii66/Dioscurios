#ifndef CASTORS_H
#define CASTORS_H

#include <Arduino.h>
#include "config.h"

// ============================================================
// Hermenex / Solder station configuration
// (Slave RP2040 — no OLED; UI on master + Nokia LCD)
// ============================================================
struct HermenexConfig {
    // Pins
    uint8_t tempPin;
    uint8_t pwmPin;
    uint8_t buzzerPin;
    uint8_t motionPin;
    uint8_t encA;
    uint8_t encB;
    uint8_t encSw;

    // PWM
    uint32_t pwmFreq;
    uint8_t  pwmRes;
    int      pwmMaxVal;

    // Temperature limits
    int tempMin;
    int tempMaxT12;
    int tempMaxHotAir;

    // Defaults
    int defaultTemp;
    int defaultBoostTemp;
    int defaultSleepTemp;

    // ADC detect
    int adcNoTip;
    int adcNoTipPtc;
};

static const HermenexConfig hermenexConfig = {
    /* pins */
    TEMP_PIN,
    PWM_PIN,
    BUZZER_PIN,
    MOTION_PIN,
    ENC_A,
    ENC_B,
    ENC_SW,

    /* pwm */
    PWM_FREQ,
    PWM_RES,
    PWM_MAX_VAL,

    /* temp limits */
    TEMP_MIN,
    TEMP_MAX_T12,
    TEMP_MAX_HOTAIR,

    /* defaults */
    DEFAULT_TEMP,
    DEFAULT_BOOST_TEMP,
    DEFAULT_SLEEP_TEMP,

    /* adc */
    ADC_NO_TIP,
    ADC_NO_TIP_PTC
};

// ============================================================
// CastorS — solder station facade
// ============================================================
#include "tip.h"
#include "pid.h"
#include "pwm.h"
#include "ptc.h"
#include "boost.h"
#include "GlobalState.h"

class CastorS {
public:
    void begin() {
        initPWM();
        initPTC();
    }

    void update() {
        updateBoost();
        updatePID();
        handleSafety();
    }

    void encoder(int delta) {
        if (delta == 0) return;
        targetTemp += delta * 5;
        if (targetTemp < hermenexConfig.tempMin) targetTemp = hermenexConfig.tempMin;
        if (targetTemp > maxTemp && maxTemp > 0) targetTemp = maxTemp;
    }
};

extern CastorS castorS;

#endif

#include "GlobalState.h"
#include "CastorS.h"
#include "tip.h"
#include "config.h"
#include "pid.h"
#include "pwm.h"


// ============================================================
// TIP DATABASE
// ============================================================

TipConfig tipDatabase[] = {

    // T12
    {
        TIP_T12,
        600,
        900,
        3.2f,
        0.12f,
        1.8f,
        MAX_PWM_T12,
        TEMP_MAX_T12,
        0.12f,
        0,
        0,
        "T12"
    },

    // C210
    {
        TIP_C210,
        100,
        500,
        2.8f,
        0.10f,
        1.4f,
        MAX_PWM_C210,
        TEMP_MAX_C210,
        0.12f,
        0,
        0,
        "C210"
    }
};

const int TOTAL_SUPPORTED_TIPS =
    sizeof(tipDatabase) / sizeof(tipDatabase[0]);


// ============================================================
// CUSTOM PROFILE
// ============================================================

TipConfig customTipProfile = {
    TIP_CUSTOM,
    0,
    1023,
    2.0f,
    0.05f,
    0.8f,
    MAX_PWM_CUSTOM,
    TEMP_MAX_CUSTOM,
    0.12f,
    0,
    0,
    "CUSTOM"
};


// ============================================================
// ACTIVE / DETECTED
// ============================================================

TipConfig *activeTip = nullptr;

TipID detectedTip = TIP_T12;


// ============================================================
// APPLY PROFILE
// ============================================================

void applyTipProfile(TipConfig *targetTip)
{
    if (!targetTip)
        return;

    activeTip = targetTip;

    currentTip = targetTip->tipID;

    kp = targetTip->kp;
    ki = targetTip->ki;
    kd = targetTip->kd;

    maxTemp = targetTip->maxTemp;

    tempOffset = targetTip->tempOffset;
    adcOffset  = targetTip->adcOffset;

    maxPwmLimit = map(
        targetTip->maxPWM,
        0,
        100,
        0,
        hermenexConfig.pwmMaxVal
    );

    if (targetTemp > maxTemp)
        targetTemp = maxTemp;
}


// ============================================================
// SET TIP PROFILE
// ============================================================

void setTipProfile(int mode)
{
    currentTipMode = mode;

    switch (mode)
    {
        case 0:
            applyTipProfile(&tipDatabase[0]);
            break;

        case 1:
            applyTipProfile(&tipDatabase[1]);
            break;

        case 3:
            applyTipProfile(&customTipProfile);
            break;

        case 2:
        default:
            break;
    }

    if (targetTemp > maxTemp)
        targetTemp = maxTemp;
}
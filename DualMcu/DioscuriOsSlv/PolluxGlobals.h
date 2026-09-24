#ifndef POLLUX_GLOBALS_H
#define POLLUX_GLOBALS_H

#include <Arduino.h>
#include "config.h"

// ============================================================
// PolluxS / Hot-Air station configuration
// Diisi dari macro di config.h
// ============================================================
struct PolluxConfig {
    // Pins
    uint8_t pinTemp;       // Hot-air ADC
    uint8_t pinFan;        // Fan PWM
    uint8_t pinZero;       // Zero-cross detect
    uint8_t pinHotgun;     // Heater AC TRIAC/gate

    // Power / timing
    uint8_t  powerPeriod;
    uint8_t  heaterMaxPower;

    // Fan PWM
    uint32_t fanPwmFreq;
    uint8_t  fanPwmRes;
    uint8_t  fanMinSpeed;

    // Temperature
    int16_t  tempAmbient;
    int16_t  tempMin;
    int16_t  tempMax;
    uint16_t tempTip[3];
};

static const PolluxConfig polluxConfig = {
    /* pins */
    PIN_HOTAIR_ADC,
    PIN_FAN_PWM,
    PIN_ZERO_CROSS,
    PIN_HEATER_AC,

    /* power */
    POWER_PERIOD,
    HEATER_MAX_POWER,

    /* fan */
    FAN_PWM_FREQ,
    FAN_PWM_RES,
    FAN_MIN_SPEED,

    /* temp */
    TEMP_AMBIENT_C,
    TEMP_MIN_C,
    TEMP_MAX_C,
    { TEMP_TIP[0], TEMP_TIP[1], TEMP_TIP[2] }
};

// Runtime state exposed by PolluxS::update()
struct PolluxState {
    uint16_t currentTemp = 0;
    uint16_t targetTemp  = 0;
    uint8_t  pwmOut      = 0;
    bool     overHeat    = false;
};

extern PolluxState polluxState;

#endif

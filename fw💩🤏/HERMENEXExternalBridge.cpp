// HERMENEXExternalBridge.cpp
// Tempel file ini di ROOT sketch utama.
// GlobalState.cpp tetap menjadi pemilik state runtime.
#include <Arduino.h>

#include "config.h"
#include <HERMENEXExternal.h>
#include <PolluxGlobals.h>

// ================= HERMENEX CONFIG (shared pin + limit + network) =================

HERMENEXConfig hermenexConfig = {
    // Pins
    TEMP_PIN,
    PWM_PIN,
    PIN_HOTAIR_ADC,
    PIN_FAN_PWM,
    PIN_ZERO_CROSS,
    PIN_HEATER_AC,
    OLED_SDA,
    OLED_SCL,
    ENC_A,
    ENC_B,
    ENC_SW,
    BUZZER_PIN,
    MOTION_PIN,

    // Temp limits / defaults
    TEMP_MIN,
    TEMP_MAX_HOTAIR,
    TEMP_MAX_T12,
    TEMP_MAX_C210,
    DEFAULT_TEMP,
    DEFAULT_BOOST_TEMP,
    DEFAULT_SLEEP_TEMP,
    DEFAULT_BOOST_TIME,
    DEFAULT_HOTAIR_TEMP,

    // ADC detect
    ADC_NO_TIP,
    ADC_NO_TIP_PTC,

    // Solder PWM
    PWM_FREQ,
    PWM_RES,
    PWM_MAX_VAL,

    // Hot air power / fan
    POWER_PERIOD,
    HEATER_MAX_POWER,
    FAN_PWM_FREQ,
    FAN_PWM_RES,
    FAN_MIN_SPEED,
    MAX_FAN_PWM,

    // Network (Meadows)
    OTA_HOSTNAME,
    WIFI_SSID_1,
    WIFI_PASS_1,
    WIFI_SSID_2,
    WIFI_PASS_2,
    AP_SSID,
    AP_PASS
};

// ================= POLLUX CONFIG (hot air) =================

PolluxConfig polluxConfig = {
    PIN_FAN_PWM,       // pinFan
    PIN_HEATER_AC,     // pinHotgun
    PIN_ZERO_CROSS,    // pinZero
    PIN_HOTAIR_ADC,    // pinTemp

    TEMP_AMBIENT_C,    // tempAmbient
    { TEMP_TIP[0], TEMP_TIP[1], TEMP_TIP[2] }, // tempTip[3]
    TEMP_MIN_C,        // tempMin
    TEMP_MAX_C,        // tempMax

    HEATER_MAX_POWER,  // heaterMaxPower
    POWER_PERIOD,      // powerPeriod

    FAN_PWM_FREQ,      // fanPwmFreq
    FAN_PWM_RES,       // fanPwmRes
    FAN_MIN_SPEED      // fanMinSpeed
};

PolluxState polluxState = {
    0,                   // currentTemp
    DEFAULT_HOTAIR_TEMP, // targetTemp
    0,                   // pwmOut
    false                // overHeat
};

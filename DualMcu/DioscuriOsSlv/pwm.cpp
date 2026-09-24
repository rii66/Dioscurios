#include "GlobalState.h"
#include "CastorS.h"
#include <Arduino.h>
#include "pwm.h"

// RP2040 / Arduino-Pico: analogWrite (0..255 default range)
// Frequency set once in initPWM() via analogWriteFreq when available.

void initPWM() {
#if defined(ARDUINO_ARCH_RP2040)
    // Pico core supports global PWM frequency
    analogWriteFreq(hermenexConfig.pwmFreq);
    analogWriteRange(hermenexConfig.pwmMaxVal);  // typically 255
#endif
    pinMode(hermenexConfig.pwmPin, OUTPUT);
    analogWrite(hermenexConfig.pwmPin, 0);
    heaterOff();
}

void heaterOff() {
    analogWrite(hermenexConfig.pwmPin, 0);
}

void heaterOn() {
    // SLEEP is a hard heater-off state for CastorS.
    if (sleeping) {
        analogWrite(hermenexConfig.pwmPin, 0);
        return;
    }

    analogWrite(hermenexConfig.pwmPin, pwmOut);
}

void setPWM(int pwm) {
    if (pwm < 0)
        pwm = 0;

    // Never allow PID/UI code to re-enable the heater during SLEEP.
    if (sleeping) {
        pwmOut = 0;
        analogWrite(hermenexConfig.pwmPin, 0);
        return;
    }

    if (pwm > hermenexConfig.pwmMaxVal)
        pwm = hermenexConfig.pwmMaxVal;

    pwmOut = pwm;
    analogWrite(hermenexConfig.pwmPin, pwmOut);
}

void startTempRead() {
    heaterOff();
    delayMicroseconds(200);
}

void endTempRead() {
    heaterOn();
}

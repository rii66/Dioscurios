#ifndef FAN_H
#define FAN_H

#include <Arduino.h>
#include "GlobalState.h"
#include "PolluxGlobals.h"

// RP2040 / Arduino-Pico: analogWrite (no LEDC)

class Fan {
public:
  void begin() {
#if defined(ARDUINO_ARCH_RP2040)
    analogWriteFreq(polluxConfig.fanPwmFreq);
    // range stays 0..255 unless changed globally
#endif
    pinMode(polluxConfig.pinFan, OUTPUT);
    analogWrite(polluxConfig.pinFan, 0);
    current_speed = 0;
  }

  void setSpeed(uint8_t speed) {
    speed = constrain(speed, 0, 255);

    if (speed > 0 && speed < polluxConfig.fanMinSpeed) {
      speed = polluxConfig.fanMinSpeed;
    }

    analogWrite(polluxConfig.pinFan, speed);
    current_speed = speed;
  }

  void off() {
    analogWrite(polluxConfig.pinFan, 0);
    current_speed = 0;
  }

  uint8_t getSpeed() const { return current_speed; }

  bool isRunning() const {
    return current_speed >= polluxConfig.fanMinSpeed;
  }

private:
  uint8_t current_speed = 0;
};

#endif

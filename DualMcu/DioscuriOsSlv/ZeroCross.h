#include "platform_compat.h"
#ifndef ZEROCROSS_H
#define ZEROCROSS_H

#include <Arduino.h>
#include "GlobalState.h"
#include "PolluxGlobals.h"

class ZeroCross {
public:
  void begin() {
    pinMode(polluxConfig.pinHotgun, OUTPUT);
    digitalWrite(polluxConfig.pinHotgun, LOW);

    pinMode(polluxConfig.pinZero, INPUT_PULLUP);

    power = 0;
    cnt = 0;
    active = false;
    last_zc = 0;
  }

  void IRAM_ATTR onZeroCross() {
    last_zc = micros();

    if (power == 0) {
      digitalWrite(polluxConfig.pinHotgun, LOW);
      active = false;
      cnt = 0;
      return;
    }

    if (cnt < power) {
      digitalWrite(polluxConfig.pinHotgun, HIGH);
      active = true;
    } else {
      digitalWrite(polluxConfig.pinHotgun, LOW);
      active = false;
    }

    cnt++;

    if (cnt >= polluxConfig.powerPeriod) {
      cnt = 0;
    }
  }

  void setPower(uint8_t p) {
    power = constrain(p, 0, polluxConfig.heaterMaxPower);

    if (power == 0) {
      digitalWrite(polluxConfig.pinHotgun, LOW);
      active = false;
      cnt = 0;
    }
  }

  uint8_t getPower() const { return power; }

  bool hasAC() const {
    return (micros() - last_zc) < 50000UL;
  }

  bool isActive() const { return active; }

private:
  volatile uint8_t power = 0;
  volatile uint8_t cnt = 0;
  volatile bool active = false;
  volatile uint32_t last_zc = 0;
};

#endif

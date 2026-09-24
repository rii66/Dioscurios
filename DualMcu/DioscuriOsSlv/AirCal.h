#ifndef AIRCAL_H
#define AIRCAL_H

#include <Arduino.h>
#include "GlobalState.h"
#include "PolluxGlobals.h"

class Calibration {
public:
  Calibration() {
    tip[0] = 2348;
    tip[1] = 3004;
    tip[2] = 3400;
  }

  void setTip(uint16_t t0, uint16_t t1, uint16_t t2) {
    tip[0] = t0;
    tip[1] = t1;
    tip[2] = constrain(t2, 0, 4095);
  }

  void getTip(uint16_t out[3]) {
    out[0] = tip[0];
    out[1] = tip[1];
    out[2] = tip[2];
  }

  uint16_t toCelsius(uint16_t adc) const {
    if (adc <= 200) return polluxConfig.tempAmbient;

    if (adc < tip[0]) {
      return map(adc, 200, tip[0], polluxConfig.tempAmbient, polluxConfig.tempTip[0]);
    }
    else if (adc <= tip[1]) {
      return map(adc, tip[0], tip[1], polluxConfig.tempTip[0], polluxConfig.tempTip[1]);
    }
    else if (adc <= tip[2]) {
      return map(adc, tip[1], tip[2], polluxConfig.tempTip[1], polluxConfig.tempTip[2]);
    }
    else {
      return map(adc, tip[0], tip[2], polluxConfig.tempTip[0], polluxConfig.tempTip[2]);
    }
  }

  uint16_t toADC(uint16_t celsius) {
    celsius = constrain(celsius, polluxConfig.tempMin, polluxConfig.tempMax);

    uint16_t left = 0;
    uint16_t right = 4095;
    uint16_t mid = map(celsius, polluxConfig.tempTip[0], polluxConfig.tempTip[2], tip[0], tip[2]);

    for (uint8_t i = 0; i < 25; i++) {
      uint16_t h = toCelsius(mid);
      if (h == celsius) return mid;

      if (h < celsius) {
        left = mid;
        mid = (left + right) / 2;
        if (mid == left) mid++;
      } else {
        right = mid;
        mid = (left + right) / 2;
        if (mid == right) mid--;
      }
    }
    return mid;
  }

private:
  uint16_t tip[3];
};

#endif

#ifndef AIRTEMP_H
#define AIRTEMP_H

#include <Arduino.h>
#include "GlobalState.h"
#include "PolluxGlobals.h"

class Temperature {
public:
  void begin() {
    // RP2040 ADC fixed ~12-bit; ESP32 needs attenuation setup
#if defined(ESP32)
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
#elif defined(ARDUINO_ARCH_RP2040)
    analogReadResolution(12);
#else
    analogReadResolution(12);
#endif

    emp_k = 32;
    emp_data = 0;
    hist_len = 0;
    hist_idx = 0;
  }

  void update() {
    int32_t raw = analogRead(polluxConfig.pinTemp);
    emp_data += raw - (emp_data + (emp_k >> 1)) / emp_k;
  }

  uint16_t raw() const {
    return (emp_data + (emp_k >> 1)) / emp_k;
  }

  void putHistory() {
    uint16_t v = raw();
    if (hist_len < HIST_SIZE) {
      hist[hist_len++] = v;
    } else {
      hist[hist_idx] = v;
      hist_idx++;
      if (hist_idx >= HIST_SIZE) hist_idx = 0;
    }
  }

  uint16_t average() const {
    if (hist_len == 0) return 0;

    uint32_t sum = 0;
    for (uint8_t i = 0; i < hist_len; i++) {
      sum += hist[i];
    }
    return (sum + (hist_len >> 1)) / hist_len;
  }

  float dispersion() const {
    if (hist_len < 4) return 9999.0f;

    uint16_t avg = average();
    uint32_t sum = 0;

    for (uint8_t i = 0; i < hist_len; i++) {
      int32_t d = (int32_t)hist[i] - avg;
      sum += d * d;
    }
    return (float)sum / hist_len;
  }

  bool isStable(float max_disp = 80.0f) const {
    return dispersion() < max_disp;
  }

  bool isCold(uint16_t threshold = 400) const {
    return average() < threshold;
  }

private:
  static const uint8_t HIST_SIZE = 16;
  uint8_t emp_k = 32;
  uint32_t emp_data = 0;
  uint16_t hist[HIST_SIZE] = {};
  uint8_t hist_len = 0;
  uint8_t hist_idx = 0;
};

#endif

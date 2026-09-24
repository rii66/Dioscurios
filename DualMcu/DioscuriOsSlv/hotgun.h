#ifndef HOTGUN_H
#define HOTGUN_H

#include <Arduino.h>

#include "GlobalState.h"
#include "ZeroCross.h"
#include "fan.h"
#include "AirTemp.h"
#include "AirPid.h"
#include "AirCal.h"

#include "storage.h"

class HotGun {
public:
  enum Mode {
    MODE_OFF = 0,
    MODE_ON,
    MODE_FIXED,
    MODE_COOLING
  };

  void begin() {
    zc.begin();
    fan.begin();
    temp.begin();
    //storage.begin();

    uint16_t c0, c1, c2;
    storage.loadSiroccroCalibration(c0, c1, c2);
    calib.setTip(c0, c1, c2);

    uint16_t savedTemp;
    uint8_t savedFan;
    storage.loadSiroccroSettings(savedTemp, savedFan);

    float kp, ki, kd;
    storage.loadSiroccroPID(kp, ki, kd);
    pid.setTunings(kp, ki, kd);

    target_temp = savedTemp;
    fan_speed = savedFan;

    mode = MODE_OFF;
    fixed_power = 0;
    last_update = 0;
    over_heat = false;

    pid.reset();
  }

  void update() {
    temp.update();

    if (!zc.isActive()) {
      temp.putHistory();
    }

    uint32_t now = millis();
    if (now - last_update < 100) return;
    last_update = now;

    float current_adc = temp.average();
    float current_c = calib.toCelsius(current_adc);
    float dt = 0.1f;

    switch (mode) {
      case MODE_ON:
        if (!fan.isRunning()) {
          zc.setPower(0);
          break;
        }

        if (current_adc > 3800) {
          mode = MODE_COOLING;
          zc.setPower(0);
          over_heat = true;
          break;
        }

        over_heat = false;
        {
          float output = pid.compute(target_temp, current_c, dt);
          zc.setPower((uint8_t)output);
        }
        break;

      case MODE_FIXED:
        if (fan.isRunning()) zc.setPower(fixed_power);
        else zc.setPower(0);
        break;

      case MODE_COOLING:
        zc.setPower(0);
        fan.setSpeed(255);

        if (temp.isCold()) {
          mode = MODE_OFF;
          fan.off();
        }
        break;

      case MODE_OFF:
      default:
        zc.setPower(0);
        break;
    }
  }

  void setTemp(uint16_t celsius) {
    target_temp = constrain(celsius, polluxConfig.tempMin, polluxConfig.tempMax);
  }

  void setFan(uint8_t speed) {
    fan_speed = speed;
    if (mode == MODE_ON || mode == MODE_FIXED) {
      fan.setSpeed(fan_speed);
    }
  }

  void switchPower(bool on) {
    if (on) {
      if (mode == MODE_OFF || mode == MODE_COOLING) {
        mode = MODE_ON;
        fan.setSpeed(fan_speed);
        pid.reset();
      }
    } else {
      if (mode == MODE_ON || mode == MODE_FIXED) {
        mode = temp.isCold() ? MODE_OFF : MODE_COOLING;
      }
    }
  }

  void setFixedPower(uint8_t p) {
    fixed_power = constrain(p, 0, polluxConfig.heaterMaxPower);
    mode = MODE_FIXED;
    fan.setSpeed(fan_speed);
  }

  void setCalibration(uint16_t t0, uint16_t t1, uint16_t t2) {
    calib.setTip(t0, t1, t2);
    storage.saveSiroccroCalibration(t0, t1, t2);
  }

  void getCalibration(uint16_t out[3]) {
    calib.getTip(out);
  }

  void saveCurrentSettings() {
    storage.saveSiroccroSettings(target_temp, fan_speed);
  }

  uint16_t getTemp() const { return calib.toCelsius(temp.average()); }
  uint16_t getTempRaw() const { return temp.average(); }
  uint16_t getTargetTemp() const { return target_temp; }
  uint8_t getPower() const { return zc.getPower(); }
  uint8_t getFan() const { return fan.getSpeed(); }
  bool isOn() const { return mode == MODE_ON || mode == MODE_FIXED; }
  bool hasAC() const { return zc.hasAC(); }
  Mode getMode() const { return mode; }
  bool isCold() const { return temp.isCold(); }
  bool isOverHeat() const { return over_heat; }

  void IRAM_ATTR onZeroCross() {
    zc.onZeroCross();
  }

private:
  ZeroCross zc;
  Fan fan;
  Temperature temp;
  PIDController pid;
  Calibration calib;

  Mode mode = MODE_OFF;
  uint16_t target_temp = 0;
  uint8_t fan_speed = 0;
  uint8_t fixed_power = 0;
  uint32_t last_update = 0;
  bool over_heat = false;
};

#endif

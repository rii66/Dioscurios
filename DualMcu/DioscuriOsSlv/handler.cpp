#include "platform_compat.h"
#include <Arduino.h>

#include "handler.h"
#include "storage.h"
#include "GlobalState.h"
#include "PolluxS.h"
#include "PolluxGlobals.h"

void initAirHandler() {
    polluxS.begin();

    attachInterrupt(
        digitalPinToInterrupt(polluxConfig.pinZero),
        handleAirZeroCross,
        FALLING
    );
}

void updateAirHandler() {
    polluxS.update();
}

void handleAirEncoder(int delta) {
    polluxS.encoder(delta);
}

void handleAirButton() {
    polluxS.hotgun.switchPower(!polluxS.hotgun.isOn());
}

void IRAM_ATTR handleAirZeroCross() {
    polluxS.hotgun.onZeroCross();
}

void airSetTemp(uint16_t celsius) {
    polluxS.hotgun.setTemp(celsius);
}

void airSetFan(uint8_t speed) {
    polluxS.hotgun.setFan(speed);
}

void airSwitchPower(bool on) {
    polluxS.hotgun.switchPower(on);
}

void airSaveSettings() {
    polluxS.hotgun.saveCurrentSettings();
}

uint16_t airGetTemp() {
    return polluxS.hotgun.getTemp();
}

uint16_t airGetTargetTemp() {
    return polluxS.hotgun.getTargetTemp();
}

uint8_t airGetPower() {
    return polluxS.hotgun.getPower();
}

uint8_t airGetFan() {
    return polluxS.hotgun.getFan();
}

bool airIsOn() {
    return polluxS.hotgun.isOn();
}

bool airHasAC() {
    return polluxS.hotgun.hasAC();
}

const char* airGetModeStr() {
    switch (polluxS.hotgun.getMode()) {
        case HotGun::MODE_ON:      return "ON";
        case HotGun::MODE_FIXED:   return "FIXED";
        case HotGun::MODE_COOLING: return "COOLING";
        case HotGun::MODE_OFF:
        default:                   return "OFF";
    }
}

#include "station.h"
#include "CastorS.h"
#include "PolluxS.h"
#include "handler.h"
#include "GlobalState.h"
#include "pwm.h"
#include "ptc.h"
#include "pid.h"

void initStations() {
    // Solder (CastorS)
    initPWM();
    initPTC();

    // Hot Air (PolluxS)
    initAirHandler();
}

void monitorStations() {
    updatePID();
    updateAirHandler();
}

void updateStations() {
    // dual update handled by monitorStations + loop()
}

void handleStationEncoder(int delta) {
    if (delta == 0) return;

    switch (activeStation) {
        case 0: // Solder
            castorS.encoder(delta);
            break;
        case 1: // Hot Air
            handleAirEncoder(delta);
            break;
    }
}

void handleStationButton() {
    switch (activeStation) {
        case 0:
            break;
        case 1:
            handleAirButton();
            break;
    }
}

void printStationStatus() {
    // Optional serial debug — currently silent
    // Uncomment for diagnostics:
    // static unsigned long last = 0;
    // if (millis() - last < 1000) return;
    // last = millis();
    // Serial.printf("T=%d/%d PWM=%d st=%d\n", currentTemp, targetTemp, pwmOut, activeStation);
}

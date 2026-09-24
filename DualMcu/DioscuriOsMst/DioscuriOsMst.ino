/**
 * DioscuriOs MASTER — ESP32-C3 SuperMini
 * OLED + PS3 stick + RGB + UART → RP2040 slave
 * WiFi / Web: placeholder (meadows pindah ke sini)
 */

#include <Arduino.h>
#include "config.h"
#include "uart_slave.h"
#include "stick.h"
#include "rgb.h"
#include "ui.h"

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println(F("========== DioscuriOs MASTER (ESP32-C3) =========="));

    initUartSlave();
    initStick();
    initRgb();
    initUi();

    cmdPing();
    cmdRequestStatus();

    Serial.println(F("[OK] master ready — waiting Pico telemetry"));
}

void loop() {
    updateUartSlave();

    StickEvent st = readStick();
    handleUiInput(st.dx, st.dy, st.clicked);

    // Extra switch 1: toggle hot-air power
    static bool lastEx1 = false;
    bool ex1 = !digitalRead(PIN_SW_EXTRA1);
    if (ex1 && !lastEx1) {
        cmdHotAirPower(!slave.airOn);
    }
    lastEx1 = ex1;

    drawUi();
    rgbStatusFromSlave();

    delay(20);  // ~50 Hz UI
}

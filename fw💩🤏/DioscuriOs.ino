/**
 * Support Sketch For ArduinoDroid
 * DioscuriOs — Dual Firmware (Solder + Hot Air) Beta1
 * Board : ESP32-C3 Super Mini
 * Author: rii66
 * Reppo : https://github.com/rii66/DiscuriOs
 *
 * Libraries required (userlibraries):
 *   - CastorS   → solder core
 *   - PolluxS   → hot air core
 *   - Cerberus  → UI / menu / storage / input / network
 *
 * Sketch files (root):
 *   - config.h
 *   - GlobalState.h / GlobalState.cpp
 *   - HERMENEXExternalBridge.cpp
 *   - DioscuriOs.ino
 *
 * Extra Arduino library:
 *   - U8g2
 *   - WebSockets (by Markus Sattler)
 */

#include "GlobalState.h"
#include "config.h"
#include <Arduino.h>

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoOTA.h>
//
#include <cerberus.h>
#include <meadows.h>
#include <PolluxS.h>
#include <CastorS.h>
// 
#include <U8g2lib.h>
#include <Wire.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C
u8g2(
    U8G2_R0,
    /* reset = */ U8X8_PIN_NONE
);

extern WebServer server;
extern WebSocketsServer webSocket;

void initWiFi();
void setupOTA();
void initWebServer();
void initWebSocket();
void sendLiveData();


// =====================================================
// HELPER
// 
// buttonClicked dideclare di encoder.h
// tetapi belum diimplementasi di encoder.cpp.
//
// Fungsi ini tetap dipertahankan untuk kompatibilitas.
// Input utama sekarang memakai handleMenu().
bool buttonClicked() {

    static bool last = false;
    bool now = buttonPressed();
    bool clicked =
        (now && !last);
    last = now;
    return clicked;
}

// =====================================================
void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println();
    Serial.println(
        F("========== DioscuriOs Dual Firmware ==========")
    );

    // 1. STORAGE
    storage.begin();

    // 2. INPUT
    initEncoder();
    initMotion();

    // 3. DISPLAY
    initDisplay();
    
    // 4. STATION 
    initStations();

    // 5. LOAD 
    loadSettings();
    loadPID();
    //setTipProfile(0); // aktifkan jika eror di kp/ki/kd
    
    // 6. NETWORK
    initWiFi();
    setupOTA();
    initWebServer();
    initWebSocket();

    // 7. BOOT ANIMATION
    booting = true;
    playBootAnimation();
    booting = false;
    
    // INITIAL ACTIVITY TIMER
    lastMotion   = millis();
    lastActivity = millis();
    sleepTimer   = millis();
    
    // DEBUG
    Serial.println(
        F("[OK] Setup selesai — dual station + network ready")
    );
    Serial.printf(
        "[MODE] activeStation = %d (0=Solder 1=HotAir)\n",
        activeStation
    );
}

// =================================================
void loop() {

    int delta =
        getEncoderDelta();

    bool pressed =
        buttonPressed();

    updateMotion();

    // handleMenu() sekarang menangani:
    //
    // SOLDER DASHBOARD
    //   rotate  -> target temperature
    //   click   -> BOOST
    //   4.9s    -> MENU
    //
    // HOT AIR DASHBOARD
    //   rotate  -> TEMP / FAN
    //   click   -> TEMP <-> FAN
    //   3s      -> ON / OFF
    //   4.9s    -> MENU
    //
    // MENU
    //   rotate  -> navigation / edit
    //   click   -> select / confirm
    //
    handleMenu(
        delta,
        pressed
    );

    detectTip();
    castorS.update();
    // updateBoost + updatePID
    updateAirHandler();

    // NETWORK
    ArduinoOTA.handle();
    server.handleClient();
    webSocket.loop();

    // WEBSOCKET LIVE DATA ~5 Hz
    static unsigned long lastWs = 0;
    if (
        millis() - lastWs >= 200
    ) {
        lastWs = millis();
        sendLiveData();
    }

    // OPTIONAL SERIAL DEBUG Off
    printStationStatus();

    // OLED UI
    drawUI();
}

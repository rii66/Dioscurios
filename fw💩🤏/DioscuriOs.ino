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

// =====================================================
// NETWORK
// =====================================================

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoOTA.h>

// =====================================================
// CERBERUS / STATIONS
// =====================================================

#include <cerberus.h>
#include <meadows.h>

#include <PolluxS.h>
#include <CastorS.h>

#include <U8g2lib.h>
#include <Wire.h>


// =====================================================
// OLED
// =====================================================

// OLED object — library pakai extern,
// jadi wajib di-define di sketch.
U8G2_SSD1306_128X64_NONAME_F_HW_I2C
u8g2(
    U8G2_R0,
    /* reset = */ U8X8_PIN_NONE
);


// =====================================================
// NETWORK OBJECTS
// =====================================================

extern WebServer server;
extern WebSocketsServer webSocket;


// =====================================================
// NETWORK PROTOTYPES
// Jaga-jaga ArduinoDroid / nested include
// =====================================================

void initWiFi();
void setupOTA();
void initWebServer();
void initWebSocket();
void sendLiveData();


// =====================================================
// HELPER
// =====================================================

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
// SETUP
// =====================================================

void setup() {

    Serial.begin(115200);

    delay(200);

    Serial.println();
    Serial.println(
        F("========== DioscuriOs Dual Firmware ==========")
    );


    // =================================================
    // 1. STORAGE
    // =================================================

    storage.begin();


    // =================================================
    // 2. INPUT
    // =================================================

    initEncoder();
    initMotion();


    // =================================================
    // 3. DISPLAY
    // =================================================

    initDisplay();


    // =================================================
    // 4. STATIONS
    // =================================================
    // Solder:
    //   PWM / PTC / temperature
    //
    // Hot Air:
    //   handler + zero-cross interrupt

    initStations();


    // =================================================
    // 5. LOAD SETTINGS
    // =================================================

    loadSettings();
    loadPID();


    // Default tip profile:
    // T12
    //
    // supaya maxTemp / Kp / Ki / Kd
    // terisi dengan benar.

    setTipProfile(0);


    // =================================================
    // 6. NETWORK
    // =================================================

    initWiFi();
    setupOTA();
    initWebServer();
    initWebSocket();


    // =================================================
    // 7. BOOT ANIMATION
    // =================================================

    playBootAnimation();

    booting = false;


    // =================================================
    // INITIAL ACTIVITY TIMER
    // =================================================

    lastMotion   = millis();
    lastActivity = millis();
    sleepTimer   = millis();


    // =================================================
    // DEBUG
    // =================================================

    Serial.println(
        F("[OK] Setup selesai — dual station + network ready")
    );

    Serial.printf(
        "[MODE] activeStation = %d (0=Solder 1=HotAir)\n",
        activeStation
    );
}


// =====================================================
// LOOP
// =====================================================

void loop() {

    // =================================================
    // READ EC11
    // =================================================

    int delta =
        getEncoderDelta();

    bool pressed =
        buttonPressed();


    // =================================================
    // MOTION / AUTO SLEEP
    // =================================================

    updateMotion();


    // =================================================
    // ALL EC11 INPUT
    // =================================================
    //
    // Dashboard + Menu semuanya lewat handleMenu().
    //
    // Jangan panggil lagi:
    //   castorS.encoder()
    //   polluxS.encoder()
    //   handleAirButton()
    //
    // dari sini.
    //
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


    // =================================================
    // RUNTIME UPDATE
    // =================================================

    // -------------------------------------------------
    // SOLDER
    // -------------------------------------------------

    detectTip();

    castorS.update();
    // updateBoost() + updatePID()


    // -------------------------------------------------
    // HOT AIR
    // -------------------------------------------------

    updateAirHandler();
    // polluxS.update()


    // =================================================
    // NETWORK
    // =================================================

    ArduinoOTA.handle();

    server.handleClient();

    webSocket.loop();


    // =================================================
    // WEBSOCKET LIVE DATA
    // ~5 Hz
    // =================================================

    static unsigned long lastWs = 0;

    if (
        millis() - lastWs >= 200
    ) {

        lastWs = millis();

        sendLiveData();
    }


    // =================================================
    // OPTIONAL SERIAL DEBUG
    // =================================================

    // printStationStatus();


    // =================================================
    // OLED UI
    // =================================================

    drawUI();
}
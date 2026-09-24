/**
 * DioscuriOs — Dual Firmware SLAVE (RP2040)
 * Role   : real-time solder + hot-air control
 * Display: Nokia 105 (CS1/CS2) — no U8g2
 * Master : ESP32-C3 via UART (optional — Pico stands alone)
 *
 * Loop tiers:
 *   FAST     — encoder, motion, UART RX
 *   CONTROL  — PID solder + hot-air (~20 Hz)
 *   TIP      — tip detect (~2 Hz, heater-off ADC)
 *   SLOW     — LCD / EEPROM debounce (self-paced)
 */

#include <Arduino.h>

#include "config.h"
#include "GlobalState.h"
#include "CastorS.h"
#include "PolluxS.h"

#include "encoder.h"
#include "motion.h"
#include "buzzer.h"
#include "pages.h"
#include "menuHandlr.h"
#include "station.h"
#include "storage.h"
#include "handler.h"
#include "boost.h"
#include "uart_link.h"
#include "lcd_temps.h"
#include "pid.h"

// ---- timing (ms) ----
static const uint32_t CONTROL_MS = 50;   // solder PID + air  ~20 Hz
static const uint32_t TIP_MS     = 500;  // tip detect         ~2 Hz

static uint32_t lastControl = 0;
static uint32_t lastTip     = 0;

// =====================================================
// ENC2 hot-air (parallel with ENC1 menu)
// =====================================================
static bool hotAirFanMode = false;

static void handleEnc2HotAir(int delta, bool pressed) {
    static bool lastBtn = false;
    static unsigned long pressStart = 0;
    static bool longDone = false;

    if (pressed && !lastBtn) {
        pressStart = millis();
        longDone = false;
        wakeFromSleep();
    }

    if (pressed && !longDone && (millis() - pressStart >= 3000)) {
        longDone = true;
        airSwitchPower(!airIsOn());
        beepSelect();
    }

    if (!pressed && lastBtn) {
        unsigned long held = millis() - pressStart;
        if (held > 50 && held < 800 && !longDone) {
            hotAirFanMode = !hotAirFanMode;
            beepMove();
        }
    }
    lastBtn = pressed;

    if (delta == 0) return;
    wakeFromSleep();

    if (hotAirFanMode) {
        int fan = (int)airGetFan() + delta * 5;
        airSetFan((uint8_t)constrain(fan, 0, 255));
    } else {
        int t = (int)airGetTargetTemp() + delta * 5;
        airSetTemp((uint16_t)constrain(t, TEMP_MIN_C, TEMP_MAX_HOTAIR));
    }
}

// =====================================================
void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println();
    Serial.println(F("========== DioscuriOs SLAVE (RP2040) =========="));

    storage.begin();
    initEncoder();
    initMotion();
    initStations();
    loadSettings();
    loadPID();
    initUartLink();
    initLcdTemps();

    lastMotion   = millis();
    lastActivity = millis();
    sleepTimer   = millis();
    lastControl  = millis();
    lastTip      = millis();

    Serial.println(F("[OK] control loop: FAST + CONTROL50ms + TIP500ms"));
}

// =====================================================
void loop() {
    const uint32_t now = millis();

    // ========== FAST: input + link RX ==========
    // Tidak mematikan heater, tidak blocking lama
    handleMenu(getEncoder1Delta(), button1Pressed());
    handleEnc2HotAir(getEncoder2Delta(), button2Pressed());
    updateMotion();
    updateUartLink();   // RX tiap loop; TX internal ~5 Hz

    // ========== CONTROL: closed-loop heat ==========
    // PID solder (ADC + PWM) dan hot-air handler
    if ((uint32_t)(now - lastControl) >= CONTROL_MS) {
        lastControl = now;
        castorS.update();     // updateBoost + updatePID + handleSafety
        updateAirHandler();   // hotgun internal min 100 ms
    }

    // ========== TIP DETECT: jarang (heater-off sample) ==========
    // Dulu tiap loop → heater sering dipaksa OFF + settle delay
    if ((uint32_t)(now - lastTip) >= TIP_MS) {
        lastTip = now;
        detectTip();
    }

    // ========== SLOW / self-paced ==========
    storageTick();      // EEPROM commit debounce
    updateLcdTemps();   // internal 200 ms + dirty skip
}

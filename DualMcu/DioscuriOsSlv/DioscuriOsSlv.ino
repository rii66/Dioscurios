/**
 * DioscuriOs — Dual Firmware SLAVE (RP2040)
 * Role   : real-time solder + hot-air control
 * Display: Nokia 105 (CS1 solder, CS2 hot-air) — NO U8g2/OLED
 * Master : ESP32-C3 (OLED menu / WiFi / Web) via UART
 *
 * ENC1 → solder (dashboard + menu)
 * ENC2 → hot-air (temp / fan / power)
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

// =====================================================
// ENC2 hot-air local handling (independent of menu)
// =====================================================
static bool hotAirFanMode = false;  // false=temp, true=fan

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
        fan = constrain(fan, 0, 255);
        airSetFan((uint8_t)fan);
    } else {
        int t = (int)airGetTargetTemp() + delta * 5;
        t = constrain(t, TEMP_MIN_C, TEMP_MAX_HOTAIR);
        airSetTemp((uint16_t)t);
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

    Serial.println(F("[OK] dual encoder + Nokia LCD + UART ready"));
    Serial.printf("[PIN] ENC1=%d/%d/%d  ENC2=%d/%d/%d\n",
                  PIN_ENC1_A, PIN_ENC1_B, PIN_ENC1_SW,
                  PIN_ENC2_A, PIN_ENC2_B, PIN_ENC2_SW);
    Serial.printf("[PIN] LCD SCK=%d SDA=%d CS1=%d CS2=%d RST=%d\n",
                  PIN_LCD_SCK, PIN_LCD_SDA, PIN_LCD_CS1, PIN_LCD_CS2, PIN_LCD_RESET);
    Serial.printf("[PIN] UART TX=%d RX=%d baud=%d\n",
                  PIN_UART_TX, PIN_UART_RX, UART_BAUD);
}

// =====================================================
void loop() {
    // ENC1 → menu + solder
    handleMenu(getEncoder1Delta(), button1Pressed());

    // ENC2 → hot-air
    handleEnc2HotAir(getEncoder2Delta(), button2Pressed());

    updateMotion();

    detectTip();
    castorS.update();
    updateAirHandler();

    updateUartLink();
    storageTick();
    updateLcdTemps();

    printStationStatus();
}

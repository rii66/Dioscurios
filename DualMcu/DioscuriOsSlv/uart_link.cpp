#include "uart_link.h"
#include "config.h"
#include "GlobalState.h"
#include "boost.h"
#include "handler.h"
#include "CastorS.h"

// Use Serial1 on RP2040 for link to master (USB stays on Serial)
#if defined(ARDUINO_ARCH_RP2040)
  #define LINK Serial1
#else
  // Fallback: same Serial (debug only)
  #define LINK Serial
#endif

static char rxBuf[64];
static uint8_t rxLen = 0;
static unsigned long lastTelem = 0;
static const unsigned long TELEM_MS = 200;  // 5 Hz

static void sendStatus() {
    LINK.printf(
        "OK,%d,%d,%d,%d,%d,%u,%u,%u,%d,%d,%d,%d\n",
        activeStation,
        currentTemp,
        targetTemp,
        pwmOut,
        tipError ? 1 : 0,
        (unsigned)airGetTemp(),
        (unsigned)airGetTargetTemp(),
        (unsigned)airGetFan(),
        airIsOn() ? 1 : 0,
        airHasAC() ? 1 : 0,
        boostMode ? 1 : 0,
        sleeping ? 1 : 0
    );
}

static void handleLine(char *line) {
    // trim CR
    for (char *p = line; *p; p++) {
        if (*p == '\r') { *p = 0; break; }
    }
    if (line[0] == 0) return;

    char cmd = line[0];
    char *arg = nullptr;
    if (line[1] == ':') arg = &line[2];
    else if (line[1] == 0) arg = nullptr;
    else return;  // unknown format

    switch (cmd) {
        case 'T':  // solder target
            if (arg) {
                int t = atoi(arg);
                t = constrain(t, hermenexConfig.tempMin,
                              maxTemp > 0 ? maxTemp : TEMP_MAX_T12);
                targetTemp = t;
                lastActivity = millis();
            }
            break;

        case 'H':  // hot-air target
            if (arg) {
                int t = atoi(arg);
                t = constrain(t, TEMP_MIN_C, TEMP_MAX_HOTAIR);
                airSetTemp((uint16_t)t);
                lastActivity = millis();
            }
            break;

        case 'F':  // fan
            if (arg) {
                int f = constrain(atoi(arg), 0, 255);
                airSetFan((uint8_t)f);
                lastActivity = millis();
            }
            break;

        case 'P':  // power hot-air
            if (arg) {
                airSwitchPower(atoi(arg) != 0);
                lastActivity = millis();
            }
            break;

        case 'B':  // boost
            startBoost();
            lastActivity = millis();
            break;

        case 'M':  // active station
            if (arg) {
                activeStation = (atoi(arg) != 0) ? 1 : 0;
            }
            break;

        case 'S':  // status now
            sendStatus();
            break;

        case 'Z':  // ping
            LINK.println(F("PONG"));
            break;

        default:
            break;
    }
}

void initUartLink() {
#if defined(ARDUINO_ARCH_RP2040)
    LINK.setTX(PIN_UART_TX);
    LINK.setRX(PIN_UART_RX);
    LINK.begin(UART_BAUD);
#else
    // Non-Pico: assume already begun or use Serial
    LINK.begin(UART_BAUD);
#endif
    rxLen = 0;
    lastTelem = millis();
    Serial.println(F("[UART] link ready → master C3"));
}

void updateUartLink() {
    // ---- RX ----
    while (LINK.available()) {
        char c = (char)LINK.read();
        if (c == '\n') {
            rxBuf[rxLen] = 0;
            handleLine(rxBuf);
            rxLen = 0;
        } else if (rxLen < sizeof(rxBuf) - 1) {
            rxBuf[rxLen++] = c;
        } else {
            rxLen = 0;  // overflow → resync
        }
    }

    // ---- TX periodic telemetry ----
    if (millis() - lastTelem >= TELEM_MS) {
        lastTelem = millis();
        sendStatus();
    }
}

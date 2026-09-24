#include "platform_compat.h"
#include "config.h"
#include "encoder.h"

// ============================================================
// Shared quadrature decode
// ============================================================
static inline void decodeStep(uint8_t a, uint8_t b,
                              volatile uint8_t &lastState,
                              volatile int &pos,
                              volatile bool &moved) {
    uint8_t encoded = (a << 1) | b;
    uint8_t sum = (lastState << 2) | encoded;
    bool did = false;

    // CW
    if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
        pos++;
        did = true;
    }
    // CCW
    if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
        pos--;
        did = true;
    }

    if (did) moved = true;
    lastState = encoded;
}

// ============================================================
// ENC1 — Solder
// ============================================================
volatile int  encoder1Pos = 0;
volatile bool encoder1Moved = false;
volatile uint8_t enc1LastState = 0;

void IRAM_ATTR encoder1ISR() {
    uint8_t a = digitalRead(PIN_ENC1_A);
    uint8_t b = digitalRead(PIN_ENC1_B);
    decodeStep(a, b, enc1LastState, encoder1Pos, encoder1Moved);
}

// ============================================================
// ENC2 — Hot Air
// ============================================================
volatile int  encoder2Pos = 0;
volatile bool encoder2Moved = false;
volatile uint8_t enc2LastState = 0;

void IRAM_ATTR encoder2ISR() {
    uint8_t a = digitalRead(PIN_ENC2_A);
    uint8_t b = digitalRead(PIN_ENC2_B);
    decodeStep(a, b, enc2LastState, encoder2Pos, encoder2Moved);
}

// Legacy name kept for any remaining refs
volatile int encoderPos = 0;

// ============================================================
// INIT
// ============================================================
void initEncoder() {
    // ENC1
    pinMode(PIN_ENC1_A,  INPUT_PULLUP);
    pinMode(PIN_ENC1_B,  INPUT_PULLUP);
    pinMode(PIN_ENC1_SW, INPUT_PULLUP);
    enc1LastState = (digitalRead(PIN_ENC1_A) << 1) | digitalRead(PIN_ENC1_B);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC1_A), encoder1ISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC1_B), encoder1ISR, CHANGE);

    // ENC2
    pinMode(PIN_ENC2_A,  INPUT_PULLUP);
    pinMode(PIN_ENC2_B,  INPUT_PULLUP);
    pinMode(PIN_ENC2_SW, INPUT_PULLUP);
    enc2LastState = (digitalRead(PIN_ENC2_A) << 1) | digitalRead(PIN_ENC2_B);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC2_A), encoder2ISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC2_B), encoder2ISR, CHANGE);

    encoder1Pos = 0;
    encoder2Pos = 0;
}

// ============================================================
// DELTA — 1 physical detent ≈ 1 logic step (divide by 4)
// ============================================================
static int logicDelta(volatile int &rawPos, int &lastLogic) {
    int current = rawPos / 4;
    int diff = current - lastLogic;
    if (diff != 0) {
        lastLogic = current;
        return diff;
    }
    return 0;
}

int getEncoder1Delta() {
    static int last = 0;
    return logicDelta(encoder1Pos, last);
}

int getEncoder2Delta() {
    static int last = 0;
    return logicDelta(encoder2Pos, last);
}

// ============================================================
// BUTTONS (active low)
// ============================================================
bool button1Pressed() {
    return !digitalRead(PIN_ENC1_SW);
}

bool button2Pressed() {
    return !digitalRead(PIN_ENC2_SW);
}

bool button1Clicked() {
    static bool last = false;
    bool now = button1Pressed();
    bool clicked = (now && !last);
    last = now;
    return clicked;
}

bool button2Clicked() {
    static bool last = false;
    bool now = button2Pressed();
    bool clicked = (now && !last);
    last = now;
    return clicked;
}

bool buttonClicked() {
    return button1Clicked();
}

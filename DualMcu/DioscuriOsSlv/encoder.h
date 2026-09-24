#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

// ============================================================
// Dual encoder — ENC1 solder, ENC2 hot-air
// ============================================================

void initEncoder();

// ENC1 (solder) — also drives menu when inMenu
int  getEncoder1Delta();
bool button1Pressed();
bool button1Clicked();

// ENC2 (hot-air)
int  getEncoder2Delta();
bool button2Pressed();
bool button2Clicked();

// ---- Legacy aliases (menuHandlr / old code) ----
// Map to ENC1 so existing handleMenu() keeps working
inline int  getEncoderDelta()  { return getEncoder1Delta(); }
inline bool buttonPressed()    { return button1Pressed(); }
bool buttonClicked();  // defined in .ino or here as ENC1 edge

// Raw positions (debug / UART)
extern volatile int encoder1Pos;
extern volatile int encoder2Pos;

#endif

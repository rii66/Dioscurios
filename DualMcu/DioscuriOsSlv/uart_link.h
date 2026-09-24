#ifndef UART_LINK_H
#define UART_LINK_H

#include <Arduino.h>

// ============================================================
// UART link — RP2040 SLAVE ↔ ESP32-C3 MASTER
// Baud: UART_BAUD (config.h)
// Pins: PIN_UART_TX / PIN_UART_RX
//
// Protocol (text line, '\n' terminated):
//
// Master → Slave commands:
//   T:<temp>       set solder target °C
//   H:<temp>       set hot-air target °C
//   F:<0-255>      set fan speed
//   P:<0|1>        hot-air power off/on
//   B              start solder boost
//   M:<0|1>        set activeStation (0=solder 1=hotair)
//   S              request status now
//   Z              ping
//
// Slave → Master telemetry (~5 Hz or on S):
//   OK,st,ct,tt,pwm,err,air_ct,air_tt,fan,pwr,ac,boost,sleep\n
//   fields:
//     st     activeStation
//     ct/tt  solder current / target
//     pwm    solder PWM
//     err    tipError
//     air_*  hot-air current / target / fan / power / hasAC
//     boost  boostMode
//     sleep  sleeping
// ============================================================

void initUartLink();
void updateUartLink();   // call every loop — rx parse + periodic tx

#endif

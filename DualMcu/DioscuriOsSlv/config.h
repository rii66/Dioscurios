#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// DioscuriOs — RP2040 SLAVE (real-time control)
// Master ESP32-C3: UI / menu / WiFi / Web (UART link)
// ============================================================

// ================= ROLE =================
#define FIRMWARE_ROLE_SLAVE     1
// WiFi / OTA / Web hidup di master C3 — tidak di-compile di slave
#define ENABLE_WIFI             0
#define ENABLE_OTA              0
#define ENABLE_WEB              0

// ================= UART → ESP32-C3 MASTER =================
// Pico TX → C3 RX (GPIO20), Pico RX ← C3 TX (GPIO21)
#define PIN_UART_TX             20      // Phys 26
#define PIN_UART_RX             21      // Phys 27
#define UART_BAUD               115200

// ================= LCD Nokia 105 (slave local display) =================
#define PIN_LCD_SCK             0       // Phys 1
#define PIN_LCD_SDA             1       // Phys 2
#define PIN_LCD_CS1             2       // Phys 4   — e.g. solder temp
#define PIN_LCD_CS2             3       // Phys 5   — e.g. hotair temp
#define PIN_LCD_CS3             4       // Phys 6
#define PIN_LCD_CS4             5       // Phys 7
#define PIN_LCD_RESET           6       // Phys 9

// ================= Encoder =================
// Encoder 1 — Solder station
#define PIN_ENC1_A              7       // Phys 10
#define PIN_ENC1_B              8       // Phys 11
#define PIN_ENC1_SW             9       // Phys 12

// Encoder 2 — Hot Air station
#define PIN_ENC2_A              10      // Phys 14
#define PIN_ENC2_B              11      // Phys 15
#define PIN_ENC2_SW             12      // Phys 16

// Legacy aliases (single-encoder path still used by current code)
#define ENC_A                   PIN_ENC1_A
#define ENC_B                   PIN_ENC1_B
#define ENC_SW                  PIN_ENC1_SW

// ================= Analog / Sensor =================
#define PIN_POT_FAN             14      // Phys 19  — fan pot (optional)
#define PIN_SLEEP1              15      // Phys 20  — motion/sleep solder
#define PIN_SLEEP2              16      // Phys 21  — motion/sleep hotair

#define PIN_ADC_TIP1            26      // Phys 31 / ADC0 — solder tip
#define PIN_ADC_TIP2            27      // Phys 32 / ADC1 — hotair sensor

// Legacy aliases
#define TEMP_PIN                PIN_ADC_TIP1
#define PIN_HOTAIR_ADC          PIN_ADC_TIP2
#define MOTION_PIN              PIN_SLEEP1

// ================= PWM / Control =================
#define PIN_PWM_SOLDER          17      // Phys 22
#define PIN_AC_HOTAIR           18      // Phys 24  — TRIAC / heater AC
#define PIN_PWM_FAN             19      // Phys 25
#define PIN_ZERO_CROSS          22      // Phys 29

// Legacy aliases
#define PWM_PIN                 PIN_PWM_SOLDER
#define PIN_HEATER_AC           PIN_AC_HOTAIR
#define PIN_FAN_PWM             PIN_PWM_FAN

// ================= Other =================
#define PIN_BUZZER              28      // Phys 34 / ADC2
#define BUZZER_PIN              PIN_BUZZER


// ================= PID SOLDER =================
#define PID_KP_T12              3.2f
#define PID_KI_T12              0.12f
#define PID_KD_T12              1.8f

#define PID_KP_C210             2.8f
#define PID_KI_C210             0.10f
#define PID_KD_C210             1.4f

#define PID_KP_CUSTOM           3.0f
#define PID_KI_CUSTOM           0.11f
#define PID_KD_CUSTOM           1.6f

// ================= TEMP LIMIT (single source of truth) =================
// Solder: limit runtime = tip profile maxTemp (tip.cpp), macros di bawah = default profile
#define TEMP_MIN                100
#define TEMP_MAX_T12            450
#define TEMP_MAX_C210            380
#define TEMP_MAX_CUSTOM          600

// Hot-air: SATU nilai untuk Pollux / UART / encoder / menu
#define TEMP_MAX_HOTAIR         500

// ================= DEFAULT TEMP =================
#define DEFAULT_TEMP            320
#define DEFAULT_BOOST_TEMP      470
#define DEFAULT_SLEEP_TEMP      195
#define DEFAULT_BOOST_TIME      12
#define DEFAULT_HOTAIR_TEMP     300

// ================= OFFSET =================
#define OFFSET_TEMP_T12         0
#define OFFSET_TEMP_C210        0
#define OFFSET_TEMP_CUSTOM      0
#define OFFSET_ADC_T12          0
#define OFFSET_ADC_C210         0
#define OFFSET_ADC_CUSTOM       0

// ================= ADC DETECT =================
#define ADC_NO_TIP              150
#define ADC_NO_TIP_PTC          4000
#define ADC_CUSTOM_MIN          501
#define ADC_CUSTOM_MAX          599

// ================= PWM CONFIG =================
#define PWM_FREQ                20000   // 20 kHz solder heater
#define PWM_RES                 8       // 8-bit
#define PWM_MAX_VAL             255

#define MAX_PWM_T12             255
#define MAX_PWM_C210            71
#define MAX_PWM_CUSTOM          255

// ================= HOT AIR =================
#define POWER_PERIOD            100
#define HEATER_MAX_POWER        100

#define FAN_PWM_FREQ            25000
#define FAN_PWM_RES             8
#define FAN_MIN_SPEED           60

#define MAX_PWM_HOTAIR          255
#define MAX_FAN_PWM             255

#define TEMP_AMBIENT_C          28
// Alias — jangan definisi angka terpisah lagi
#define TEMP_MIN_C              TEMP_MIN
#define TEMP_MAX_C              TEMP_MAX_HOTAIR

const uint16_t TEMP_TIP[3] = {200, 300, 400};

#endif

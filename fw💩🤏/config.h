#ifndef CONFIG_H
#define CONFIG_H

// ================= WIFI & OTA =================
#define WIFI_STA_TX_POWER WIFI_POWER_8_5dBm
#define WIFI_AP_TX_POWER  WIFI_POWER_17dBm

#define WIFI_SSID_1 "ICA ELEK"
#define WIFI_PASS_1 "koyok tekek"

#define WIFI_SSID_2 "Indirom"
#define WIFI_PASS_2 "1sampai8"

#define AP_SSID "DioscuriStationOS"
#define AP_PASS "12345678"

#define OTA_HOSTNAME    "EspDual_Station"


// ================= PIN CONFIG (ESP32-C3 Super Mini) =================

// ----- Solder (nama lama dipertahankan) -----
#define TEMP_PIN            0       // Solder ADC
#define PWM_PIN             2       // Solder PWM

// ----- Hot Air (baru) -----
#define PIN_HOTAIR_ADC      1
#define PIN_FAN_PWM         3
#define PIN_ZERO_CROSS      4
#define PIN_HEATER_AC       10

// ----- OLED -----
#define OLED_SDA            6
#define OLED_SCL            7

// ----- Encoder -----
#define ENC_A               20
#define ENC_B               21
#define ENC_SW              5

// ----- Other -----
#define BUZZER_PIN          8
#define MOTION_PIN        9
// #define MOTION_PIN_solder. 

// gpio abiss ;(
// #define RGB_RED          9
// #define RGB_BLUE         10


// ================= PID SOLDER =================
#define PID_KP_T12          3.2f
#define PID_KI_T12          0.12f
#define PID_KD_T12          1.8f

#define PID_KP_C210         2.8f
#define PID_KI_C210         0.10f
#define PID_KD_C210         1.4f

#define PID_KP_CUSTOM       3.0f
#define PID_KI_CUSTOM       0.11f
#define PID_KD_CUSTOM       1.6f


// ================= TEMP LIMIT =================
#define TEMP_MIN            100
#define TEMP_MAX_T12        450
#define TEMP_MAX_C210       380
#define TEMP_MAX_CUSTOM     600

// Hot Air (bisa disesuaikan nanti)
#define TEMP_MAX_HOTAIR     550


// ================= DEFAULT TEMP =================
#define DEFAULT_TEMP            320
#define DEFAULT_BOOST_TEMP      470
#define DEFAULT_SLEEP_TEMP      195
#define DEFAULT_BOOST_TIME      12

#define DEFAULT_HOTAIR_TEMP     300     // default hot air


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
#define PWM_FREQ                20000   // 20kHz
#define PWM_RES                 8       // 8-bit
#define PWM_MAX_VAL             255

#define MAX_PWM_T12             255
#define MAX_PWM_C210            71
#define MAX_PWM_CUSTOM          255

// ================= SIROCCRO / HOT AIR =================
#define POWER_PERIOD       100
#define HEATER_MAX_POWER   100

#define FAN_PWM_FREQ       25000
#define FAN_PWM_RES        8
#define FAN_MIN_SPEED      60

// Hot Air PWM limit (bisa disesuaikan)
#define MAX_PWM_HOTAIR          255
#define MAX_FAN_PWM             255

// Hot Temp
#define TEMP_AMBIENT_C        28
#define TEMP_MIN_C            100
#define TEMP_MAX_C            500

const uint16_t TEMP_TIP[3] = {200, 300, 400};


#endif
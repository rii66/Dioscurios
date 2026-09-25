# DioscuriOs — Port notes for RP2040 (Arduino-Pico)

## Done in this restructure
- Flat single folder (no carberus/castors/nimfa/meadows/polluxs/pandora)
- No umbrella headers (`cerberus.h`, `meadows.h`, old `CastorS.h` paths)
- Web / OTA / WiFi fully removed from build path
- `storage` is RAM-defaults stub (no Preferences / NVS yet)
- `PolluxGlobals.h` added (was missing)

## Still ESP32-specific (must adapt for RP2040)

| Location | API | RP2040 replacement |
|----------|-----|--------------------|
| `pwm.cpp`, `fan.h` | `ledcAttach` / `ledcWrite` | `analogWrite` or Pico PWM (`pwm_set_gpio_level`) |
| `encoder.cpp`, `ZeroCross.h`, `handler.cpp` | `IRAM_ATTR` | optional; define empty if missing |
| `AirTemp.h` | `analogSetAttenuation`, `analogReadResolution` | Pico ADC is 12-bit by default; remove ESP calls |
| `display.cpp` | `Wire.begin(sda, scl)` | works on Arduino-Pico |
| Pins in `config.h` | ESP32-C3 GPIO numbers | remap to your RP2040 board |

## Suggested next steps
1. Remap pins in `config.h` for your RP2040 board.
2. Replace LEDC with analogWrite / Pico PWM in `pwm.cpp` + `fan.h`.
3. Soften `IRAM_ATTR` (e.g. `#ifndef IRAM_ATTR #define IRAM_ATTR #endif`).
4. Implement real storage with EEPROM or LittleFS when needed.
5. Build with **Arduino Mbed OS RP2040** or **Earle Philhower Arduino-Pico** core.

## Folder = sketch root
Copy entire `DioscuriOs/` folder as the sketch directory. All sources are sibling files.

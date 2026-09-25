#ifndef PLATFORM_COMPAT_H
#define PLATFORM_COMPAT_H

// ESP32 has IRAM_ATTR; RP2040 / others may not
#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

#endif

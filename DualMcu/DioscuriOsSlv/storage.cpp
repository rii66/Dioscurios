#include <string.h>
/**
 * storage.cpp — port dari:
 * https://github.com/rii66/Carberus/blob/main/src/pandora/storage.cpp
 *
 * RP2040 slave: EEPROM (Preferences NVS tidak ada di Pico)
 * ESP32: tetap bisa pakai Preferences jika di-compile di sana
 */

#include "storage.h"
#include "config.h"
#include "GlobalState.h"
#include "CastorS.h"
#include "tip.h"
#include "pages.h"
#include "boost.h"

Storage storage;

// ============================================================
// Backend
// ============================================================
#if defined(ARDUINO_ARCH_RP2040)

#include <EEPROM.h>

static const uint32_t STORAGE_MAGIC = 0x44314F53; // 'D1OS'
static const int EEPROM_SIZE = 512;

struct TipPersist {
    float kp, ki, kd;
    int   tempOffset;
    int   adcOffset;
};

struct PersistBlob {
    uint32_t magic;

    // Patri / solder
    int  targetTemp;
    int  boostTemp;
    int  boostTimeSec;
    int  sleepTemp;
    int  sleepTimeSec;
    bool buzzerEnabled;
    int  tipMode;

    TipPersist tips[2];   // T12, C210
    TipPersist custom;

    // Siroccro / hot-air
    uint16_t sc_c0, sc_c1, sc_c2;
    uint16_t sc_temp;
    uint8_t  sc_fan;
    float    sc_kp, sc_ki, sc_kd;
};

static PersistBlob blob;           // working copy (RAM)
static PersistBlob blobCommitted;  // last image written to flash
static bool dirty = false;
static unsigned long dirtySince = 0;

// Debounce: gabung banyak save*() jadi 1x commit
#ifndef STORAGE_COMMIT_DELAY_MS
#define STORAGE_COMMIT_DELAY_MS  3000UL
#endif

static void defaultsBlob() {
    memset(&blob, 0, sizeof(blob));
    blob.magic = STORAGE_MAGIC;

    blob.targetTemp    = DEFAULT_TEMP;
    blob.boostTemp     = DEFAULT_BOOST_TEMP;
    blob.boostTimeSec  = DEFAULT_BOOST_TIME;
    blob.sleepTemp     = DEFAULT_SLEEP_TEMP;
    blob.sleepTimeSec  = 240;
    blob.buzzerEnabled = true;
    blob.tipMode       = TIP_ITEM_AUTO;

    blob.sc_c0 = 2348;
    blob.sc_c1 = 3004;
    blob.sc_c2 = 3400;
    blob.sc_temp = DEFAULT_HOTAIR_TEMP;
    blob.sc_fan  = 0;
    blob.sc_kp = 12.0f;
    blob.sc_ki = 0.6f;
    blob.sc_kd = 25.0f;
}

static void eepromLoad() {
    EEPROM.get(0, blob);
    if (blob.magic != STORAGE_MAGIC) {
        defaultsBlob();
        dirty = true;
        dirtySince = millis();
        memset(&blobCommitted, 0, sizeof(blobCommitted));
    } else {
        blobCommitted = blob;
        dirty = false;
    }
}

// Tandai dirty saja — TIDAK commit flash
static void markDirty() {
    if (!dirty) {
        dirty = true;
        dirtySince = millis();
    } else {
        // perpanjang jendela debounce setiap perubahan baru
        dirtySince = millis();
    }
}

// Commit ke flash hanya jika perlu
static void eepromFlush(bool force = false) {
    if (!dirty && !force) return;

    // Skip write jika isi sama dengan yang sudah di flash
    if (memcmp(&blob, &blobCommitted, sizeof(blob)) == 0) {
        dirty = false;
        return;
    }

    EEPROM.put(0, blob);
    if (EEPROM.commit()) {
        blobCommitted = blob;
        dirty = false;
#if defined(STORAGE_DEBUG)
        Serial.println(F("[Storage] EEPROM commit OK"));
#endif
    } else {
        Serial.println(F("[Storage] EEPROM commit FAILED"));
    }
}

// Alias lama: jangan commit langsung
static void eepromSave() {
    markDirty();
}

#elif defined(ESP32)

#include <Preferences.h>
static Preferences prefs;

#else
#error "storage: unsupported architecture"
#endif

// ============================================================
// BEGIN
// ============================================================
void Storage::begin()
{
#if defined(ARDUINO_ARCH_RP2040)
    EEPROM.begin(EEPROM_SIZE);
    eepromLoad();
    // First boot / corrupt: tulis defaults sekali saja
    if (dirty) eepromFlush(true);
    Serial.println(F("[Storage] EEPROM ready (RP2040, deferred commit)"));
#elif defined(ESP32)
    prefs.begin("iron", false);
    Serial.println(F("[Storage] Preferences ready (ESP32)"));
#endif
}

// ============================================================
// POLLUXS CALIBRATION
// ============================================================
void Storage::loadSiroccroCalibration(uint16_t &c0, uint16_t &c1, uint16_t &c2)
{
#if defined(ARDUINO_ARCH_RP2040)
    c0 = blob.sc_c0;
    c1 = blob.sc_c1;
    c2 = blob.sc_c2;
#elif defined(ESP32)
    c0 = prefs.getUShort("sc_c0", 2348);
    c1 = prefs.getUShort("sc_c1", 3004);
    c2 = prefs.getUShort("sc_c2", 3400);
#endif
}

void Storage::saveSiroccroCalibration(uint16_t c0, uint16_t c1, uint16_t c2)
{
#if defined(ARDUINO_ARCH_RP2040)
    blob.sc_c0 = c0; blob.sc_c1 = c1; blob.sc_c2 = c2;
    eepromSave();
#elif defined(ESP32)
    prefs.putUShort("sc_c0", c0);
    prefs.putUShort("sc_c1", c1);
    prefs.putUShort("sc_c2", c2);
#endif
}

// ============================================================
// POLLUXS SETTINGS
// ============================================================
void Storage::loadSiroccroSettings(uint16_t &temp, uint8_t &fan)
{
#if defined(ARDUINO_ARCH_RP2040)
    temp = blob.sc_temp;
    fan  = blob.sc_fan;
#elif defined(ESP32)
    temp = prefs.getUShort("sc_temp", DEFAULT_HOTAIR_TEMP);
    fan  = prefs.getUChar("sc_fan", 0);
#endif
}

void Storage::saveSiroccroSettings(uint16_t temp, uint8_t fan)
{
#if defined(ARDUINO_ARCH_RP2040)
    blob.sc_temp = temp;
    blob.sc_fan  = fan;
    eepromSave();
#elif defined(ESP32)
    prefs.putUShort("sc_temp", temp);
    prefs.putUChar("sc_fan", fan);
#endif
}

// ============================================================
// POLLUXS PID
// ============================================================
void Storage::loadSiroccroPID(float &kp, float &ki, float &kd)
{
#if defined(ARDUINO_ARCH_RP2040)
    kp = blob.sc_kp; ki = blob.sc_ki; kd = blob.sc_kd;
#elif defined(ESP32)
    kp = prefs.getFloat("sc_kp", 12.0f);
    ki = prefs.getFloat("sc_ki", 0.6f);
    kd = prefs.getFloat("sc_kd", 25.0f);
#endif
}

void Storage::saveSiroccroPID(float kp, float ki, float kd)
{
#if defined(ARDUINO_ARCH_RP2040)
    blob.sc_kp = kp; blob.sc_ki = ki; blob.sc_kd = kd;
    eepromSave();
#elif defined(ESP32)
    prefs.putFloat("sc_kp", kp);
    prefs.putFloat("sc_ki", ki);
    prefs.putFloat("sc_kd", kd);
#endif
}

// ============================================================
// CASTORS SETTINGS (solder)
// ============================================================
void Storage::loadSettings()
{
#if defined(ARDUINO_ARCH_RP2040)
    targetTemp   = blob.targetTemp;
    boostTemp    = blob.boostTemp;
    boostTimeSec = blob.boostTimeSec;
    sleepTemp    = blob.sleepTemp;
    sleepTimeSec = blob.sleepTimeSec;
    buzzerEnabled = blob.buzzerEnabled;
    currentTipMode = blob.tipMode;

    if (currentTipMode < TIP_ITEM_T12 || currentTipMode > TIP_ITEM_CUSTOM)
        currentTipMode = TIP_ITEM_AUTO;

    for (int i = 0; i < TOTAL_SUPPORTED_TIPS && i < 2; i++) {
        TipConfig &tip = tipDatabase[i];
        TipPersist &p = blob.tips[i];
        // if never written, keep compile-time defaults (kp!=0 from tip.cpp)
        if (p.kp != 0.0f || p.ki != 0.0f || p.kd != 0.0f) {
            tip.kp = p.kp; tip.ki = p.ki; tip.kd = p.kd;
            tip.tempOffset = p.tempOffset;
            tip.adcOffset  = p.adcOffset;
        }
    }
    if (blob.custom.kp != 0.0f || blob.custom.ki != 0.0f || blob.custom.kd != 0.0f) {
        customTipProfile.kp = blob.custom.kp;
        customTipProfile.ki = blob.custom.ki;
        customTipProfile.kd = blob.custom.kd;
        customTipProfile.tempOffset = blob.custom.tempOffset;
        customTipProfile.adcOffset  = blob.custom.adcOffset;
    }

#elif defined(ESP32)
    targetTemp = prefs.getInt("pt_temp", hermenexConfig.defaultTemp);
    boostTemp  = prefs.getInt("pt_boostT", hermenexConfig.defaultBoostTemp);
    boostTimeSec = prefs.getInt("pt_boostS", DEFAULT_BOOST_TIME);
    sleepTemp  = prefs.getInt("pt_sleepT", hermenexConfig.defaultSleepTemp);
    sleepTimeSec = prefs.getInt("pt_sleepS", 240);
    buzzerEnabled = prefs.getBool("pt_buzzer", true);
    currentTipMode = prefs.getInt("pt_tipMode", TIP_ITEM_AUTO);

    if (currentTipMode < TIP_ITEM_T12 || currentTipMode > TIP_ITEM_CUSTOM)
        currentTipMode = TIP_ITEM_AUTO;

    for (int i = 0; i < TOTAL_SUPPORTED_TIPS; i++) {
        TipConfig &tip = tipDatabase[i];
        String base = String("pt_") + tip.name;
        tip.kp = prefs.getFloat((base + "_kp").c_str(), tip.kp);
        tip.ki = prefs.getFloat((base + "_ki").c_str(), tip.ki);
        tip.kd = prefs.getFloat((base + "_kd").c_str(), tip.kd);
        tip.tempOffset = prefs.getInt((base + "_tOff").c_str(), tip.tempOffset);
        tip.adcOffset  = prefs.getInt((base + "_aOff").c_str(), tip.adcOffset);
    }
    {
        String base = String("pt_") + customTipProfile.name;
        customTipProfile.kp = prefs.getFloat((base + "_kp").c_str(), customTipProfile.kp);
        customTipProfile.ki = prefs.getFloat((base + "_ki").c_str(), customTipProfile.ki);
        customTipProfile.kd = prefs.getFloat((base + "_kd").c_str(), customTipProfile.kd);
        customTipProfile.tempOffset = prefs.getInt((base + "_tOff").c_str(), customTipProfile.tempOffset);
        customTipProfile.adcOffset  = prefs.getInt((base + "_aOff").c_str(), customTipProfile.adcOffset);
    }
#endif

    // Apply active tip
    switch (currentTipMode) {
        case TIP_ITEM_T12:
            applyTipProfile(&tipDatabase[0]);
            break;
        case TIP_ITEM_C210:
            applyTipProfile(&tipDatabase[1]);
            break;
        case TIP_ITEM_CUSTOM:
            applyTipProfile(&customTipProfile);
            break;
        case TIP_ITEM_AUTO:
        default:
            activeTip = nullptr;
            break;
    }
}

void Storage::saveSettings()
{
#if defined(ARDUINO_ARCH_RP2040)
    blob.targetTemp = targetTemp;
    eepromSave();
#elif defined(ESP32)
    prefs.putInt("pt_temp", targetTemp);
#endif
}

// ============================================================
// PID
// ============================================================
void Storage::loadPID()
{
    if (!activeTip) return;

#if defined(ARDUINO_ARCH_RP2040)
    // already applied via tip profile; refresh globals
    kp = activeTip->kp;
    ki = activeTip->ki;
    kd = activeTip->kd;
#elif defined(ESP32)
    String base = String("pt_") + activeTip->name;
    kp = prefs.getFloat((base + "_kp").c_str(), activeTip->kp);
    ki = prefs.getFloat((base + "_ki").c_str(), activeTip->ki);
    kd = prefs.getFloat((base + "_kd").c_str(), activeTip->kd);
#endif
}

void Storage::savePID()
{
    if (!activeTip) return;

#if defined(ARDUINO_ARCH_RP2040)
    activeTip->kp = kp;
    activeTip->ki = ki;
    activeTip->kd = kd;
    // write into blob slot
    if (activeTip == &tipDatabase[0]) {
        blob.tips[0] = {kp, ki, kd, activeTip->tempOffset, activeTip->adcOffset};
    } else if (TOTAL_SUPPORTED_TIPS > 1 && activeTip == &tipDatabase[1]) {
        blob.tips[1] = {kp, ki, kd, activeTip->tempOffset, activeTip->adcOffset};
    } else if (activeTip == &customTipProfile) {
        blob.custom = {kp, ki, kd, activeTip->tempOffset, activeTip->adcOffset};
    }
    eepromSave();
#elif defined(ESP32)
    String base = String("pt_") + activeTip->name;
    prefs.putFloat((base + "_kp").c_str(), kp);
    prefs.putFloat((base + "_ki").c_str(), ki);
    prefs.putFloat((base + "_kd").c_str(), kd);
#endif
}

void Storage::saveActivePID()
{
    if (!activeTip) return;

#if defined(ARDUINO_ARCH_RP2040)
    kp = activeTip->kp;
    ki = activeTip->ki;
    kd = activeTip->kd;
    savePID();
#elif defined(ESP32)
    String base = String("pt_") + activeTip->name;
    prefs.putFloat((base + "_kp").c_str(), activeTip->kp);
    prefs.putFloat((base + "_ki").c_str(), activeTip->ki);
    prefs.putFloat((base + "_kd").c_str(), activeTip->kd);
#endif
}

// ============================================================
// BOOST / SLEEP / CAL / TIP / BUZZER
// ============================================================
void Storage::saveBoost()
{
#if defined(ARDUINO_ARCH_RP2040)
    blob.boostTemp = boostTemp;
    blob.boostTimeSec = boostTimeSec;
    eepromSave();
#elif defined(ESP32)
    prefs.putInt("pt_boostT", boostTemp);
    prefs.putInt("pt_boostS", boostTimeSec);
#endif
}

void Storage::saveSleep()
{
#if defined(ARDUINO_ARCH_RP2040)
    blob.sleepTemp = sleepTemp;
    blob.sleepTimeSec = sleepTimeSec;
    eepromSave();
#elif defined(ESP32)
    prefs.putInt("pt_sleepT", sleepTemp);
    prefs.putInt("pt_sleepS", sleepTimeSec);
#endif
}

void Storage::saveCal()
{
    if (!activeTip) return;

    activeTip->tempOffset = tempOffset;
    activeTip->adcOffset  = adcOffset;

#if defined(ARDUINO_ARCH_RP2040)
    if (activeTip == &tipDatabase[0]) {
        blob.tips[0].tempOffset = tempOffset;
        blob.tips[0].adcOffset  = adcOffset;
    } else if (TOTAL_SUPPORTED_TIPS > 1 && activeTip == &tipDatabase[1]) {
        blob.tips[1].tempOffset = tempOffset;
        blob.tips[1].adcOffset  = adcOffset;
    } else if (activeTip == &customTipProfile) {
        blob.custom.tempOffset = tempOffset;
        blob.custom.adcOffset  = adcOffset;
    }
    eepromSave();
#elif defined(ESP32)
    String base = String("pt_") + activeTip->name;
    prefs.putInt((base + "_tOff").c_str(), tempOffset);
    prefs.putInt((base + "_aOff").c_str(), adcOffset);
#endif
}

void Storage::saveTip()
{
#if defined(ARDUINO_ARCH_RP2040)
    blob.tipMode = currentTipMode;
    eepromSave();
#elif defined(ESP32)
    prefs.putInt("pt_tipMode", currentTipMode);
#endif
}

void Storage::saveBuzzer()
{
#if defined(ARDUINO_ARCH_RP2040)
    blob.buzzerEnabled = buzzerEnabled;
    eepromSave();
#elif defined(ESP32)
    prefs.putBool("pt_buzzer", buzzerEnabled);
#endif
}

void Storage::factoryReset()
{
#if defined(ARDUINO_ARCH_RP2040)
    defaultsBlob();
    dirty = true;
    eepromFlush(true);  // reset harus langsung ke flash
#elif defined(ESP32)
    prefs.clear();
#endif
}


// ============================================================
// DEFERRED COMMIT (RP2040)
// ============================================================
void Storage::tick()
{
#if defined(ARDUINO_ARCH_RP2040)
    if (!dirty) return;
    if (millis() - dirtySince < STORAGE_COMMIT_DELAY_MS) return;
    eepromFlush(false);
#endif
}

void Storage::flush()
{
#if defined(ARDUINO_ARCH_RP2040)
    eepromFlush(true);
#endif
}

// ============================================================
// BARE WRAPPERS
// ============================================================
void loadSettings()  { storage.loadSettings(); }
void saveSettings()  { storage.saveSettings(); }
void loadPID()       { storage.loadPID(); }
void savePID()       { storage.savePID(); }
void saveActivePID() { storage.saveActivePID(); }
void saveBoost()     { storage.saveBoost(); }
void saveSleep()     { storage.saveSleep(); }
void saveCal()       { storage.saveCal(); }
void saveTip()       { storage.saveTip(); }
void saveBuzzer()    { storage.saveBuzzer(); }

void storageTick()   { storage.tick(); }
void storageFlush()  { storage.flush(); }

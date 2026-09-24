#include "CastorS.h"
#include "GlobalState.h"
#include "menuHandlr.h"
#include "encoder.h"
#include "pages.h"
#include "motion.h"
#include "buzzer.h"

#include "storage.h"


//====================================//
//             MENU HANDLER            //
//====================================//

// CURSOR TIAP HALAMAN

int boostMenuIndex = 0;
int sleepMenuIndex = 0;
int pidMenuIndex   = 0;
int tipMenuIndex   = 0;
int buzMenuIndex   = 0;
int calMenuIndex   = 0;

// CURSOR KHUSUS SUBMENU STATION

static int stationItem = STATION_SOLDER;
static bool stationMenu = false;
static bool hotAirFanEdit = false;

// SIMPAN POSISI CURSOR SUBMENU

void saveMenuCursor() {

    switch (page) {

        case PAGE_BOOST:
            boostMenuIndex = item;
            break;

        case PAGE_SLEEP:
            sleepMenuIndex = item;
            break;

        case PAGE_CAL:
            calMenuIndex = item;
            break;

        case PAGE_PID:
            pidMenuIndex = item;
            break;

        case PAGE_TIP:
            tipMenuIndex = item;
            break;

        case PAGE_BUZZER:
            buzMenuIndex = item;
            break;
    }
}


// EKSEKUSI PILIHAN MENU

void executePageSelect() {

    saveMenuCursor();

    switch (page) {

        // =====================================================
        // SETTING
        // =====================================================

        case PAGE_SET:

            if (item == SET_SAVE || item == SET_EXIT) {

                // saveSettings();

                beepSave();
                inEdit = false;
            }

            else if (item == SET_STATION) {

                // Masuk submenu Station
                stationItem =
                    (activeStation == STATION_HOTAIR)
                    ? STATION_HOTAIR
                    : STATION_SOLDER;

                stationMenu = true;
            }

            else if (item == SET_TEMP) {

                isEditingValue = true;
            }

            break;


        // =====================================================
        // BOOST
        // =====================================================

        case PAGE_BOOST:

            if (item == BOOST_SAVE || item == BOOST_EXIT) {

                saveBoost();

                beepSave();
                inEdit = false;
            }

            else {

                isEditingValue = true;
            }

            break;


        // =====================================================
        // SLEEP
        // =====================================================

        case PAGE_SLEEP:

            if (item == SLEEP_SAVE || item == SLEEP_EXIT) {

                saveSleep();

                beepSave();
                inEdit = false;
            }

            else {

                isEditingValue = true;
            }

            break;


        // =====================================================
        // CALIBRATION
        // =====================================================

        case PAGE_CAL:

            if (item == CAL_SAVE || item == CAL_EXIT) {

                saveCal();

                beepSave();
                inEdit = false;
            }

            else if (item == CAL_SOLDER || item == CAL_HOTAIR) {

                isEditingValue = true;
            }

            break;


        // =====================================================
        // PID
        // =====================================================

        case PAGE_PID:

            if (item == PID_SAVE || item == PID_EXIT) {

                saveActivePID();

                beepSave();
                inEdit = false;
            }

            else if (
                item == PID_KP ||
                item == PID_KI ||
                item == PID_KD
            ) {

                isEditingValue = true;
            }

            break;


        // =====================================================
        // TIP
        // =====================================================

        case PAGE_TIP:

            if (item == TIP_ITEM_SAVE) {

                saveTip();

                beepSave();
                inEdit = false;
            }

            else if (item == TIP_ITEM_EXIT) {

                inEdit = false;
            }

            else if (item == TIP_ITEM_T12) {

                setTipProfile(TIP_ITEM_T12);
            }

            else if (item == TIP_ITEM_C210) {

                setTipProfile(TIP_ITEM_C210);
            }

            else if (item == TIP_ITEM_AUTO) {

                currentTipMode = TIP_ITEM_AUTO;
                detectTip();
            }

            else if (item == TIP_ITEM_CUSTOM) {

                setTipProfile(TIP_ITEM_CUSTOM);
            }

            break;


        // =====================================================
        // BUZZER
        // =====================================================

        case PAGE_BUZZER:

            if (item == BUZ_SAVE || item == BUZ_EXIT) {

                saveBuzzer();

                beepSave();
                inEdit = false;
            }

            else if (item == BUZ_ON) {

                buzzerEnabled = true;
            }

            else if (item == BUZ_OFF) {

                buzzerEnabled = false;
            }

            break;
    }
}


// =========================================================
// HANDLE MENU
// =========================================================

void handleMenu(int direction, bool pressed) {

    static bool lastBtn = false;
    static bool long3sDone = false;
    static bool menu4s9Done = false;

    // =====================================================
    // BUTTON PRESS START
    // =====================================================

    if (pressed && !lastBtn) {

        wakeFromSleep();

        btnPressStart = millis();
        btnHolding = true;

        long3sDone = false;
        menu4s9Done = false;
    }

    // =====================================================
    // LONG PRESS ACTIONS WHILE BUTTON IS HELD
    // =====================================================

    if (pressed && btnHolding) {

        unsigned long heldTime = millis() - btnPressStart;

        // -------------------------------------------------
        // 3 SECOND: HOT AIR POWER ON / OFF
        // -------------------------------------------------
        if (!long3sDone && heldTime >= 3000) {

            long3sDone = true;

            if (!inMenu && activeStation == STATION_HOTAIR) {
                airSwitchPower(!airIsOn());
                beepSelect();
            }
        }

        // -------------------------------------------------
        // 4.9 SECOND: ENTER / EXIT MENU
        // -------------------------------------------------
        if (!menu4s9Done && heldTime >= 4900) {

            menu4s9Done = true;

            // IMPORTANT:
            // Long press must TOGGLE menu, like the original code.
            // Otherwise once inMenu=true, there is no way back
            // to dashboard and the UI appears stuck in menu.
            inMenu = !inMenu;

            beepLong();

            // Always clear edit/submenu state when entering OR
            // leaving the main menu.
            inEdit = false;
            isEditingValue = false;
            stationMenu = false;
            hotAirFanEdit = false;

            if (inMenu) {
                page = PAGE_SET;
                item = 0;
            }
        }
    }

    // =====================================================
    // BUTTON RELEASE
    // =====================================================

    if (!pressed && lastBtn) {

        unsigned long holdTime = millis() - btnPressStart;

        // Long actions were already executed while held.
        // Never execute them again on release.
        bool longActionDone = long3sDone || menu4s9Done;

        if (holdTime > 50 && !longActionDone) {

            // =================================================
            // DASHBOARD
            // =================================================

            if (!inMenu) {
                // Dual encoder: ENC1 always solder → short click = Boost
                // (Hot-air power / fan toggle handled by ENC2 in main loop)
                startBoost();
            }

            // =================================================
            // MENU
            // =================================================

            else {

                // =============================================
                // SUBMENU STATION
                // =============================================

                if (stationMenu) {

                    if (stationItem == STATION_SOLDER) {

                        activeStation = STATION_SOLDER;
                        hotAirFanEdit = false;
                        beepSelect();
                    }

                    else if (stationItem == STATION_HOTAIR) {

                        activeStation = STATION_HOTAIR;
                        hotAirFanEdit = false;
                        beepSelect();
                    }

                    else if (stationItem == STATION_SAVE) {

                        beepSave();
                        stationMenu = false;
                    }

                    else if (stationItem == STATION_EXIT) {

                        stationMenu = false;
                        beepSelect();
                    }
                }

                // =============================================
                // MASUK EDIT MENU
                // =============================================

                else if (!inEdit) {

                    beepSelect();

                    inEdit = true;
                    item = 0;
                    stationMenu = false;
                }

                // =============================================
                // PILIH ITEM
                // =============================================

                else if (!isEditingValue) {

                    executePageSelect();
                }

                // =============================================
                // SELESAI EDIT VALUE
                // =============================================

                else {

                    isEditingValue = false;
                }
            }
        }

        btnHolding = false;
    }

    lastBtn = pressed;

    // =====================================================
    // ENCODER LOGIC
    // =====================================================

    if (direction != 0) {
        wakeFromSleep();
    }

    // =====================================================
    // SUBMENU STATION
    // =====================================================

    if (stationMenu) {

        if (direction != 0) {

            stationItem += direction;

            if (stationItem < 0) {
                stationItem = STATION_COUNT - 1;
            }

            if (stationItem >= STATION_COUNT) {
                stationItem = 0;
            }

            beepMove();
        }

        return;
    }

    if (direction == 0) {
        return;
    }

    // =====================================================
    // DASHBOARD
    // =====================================================

    if (!inMenu) {
        // Dual encoder: ENC1 dashboard always adjusts solder target
        // (Hot-air is controlled by ENC2 in main loop)
        targetTemp += direction * 5;
        targetTemp = constrain(
            targetTemp,
            hermenexConfig.tempMin,
            maxTemp > 0 ? maxTemp : TEMP_MAX_T12
        );
        return;
    }

    // =====================================================
    // LEVEL 1 : GANTI PAGE
    // =====================================================

    if (!inEdit) {

        page += direction;

        if (page >= PAGE_TOTAL) {
            page = 0;
        }

        if (page < 0) {
            page = PAGE_TOTAL - 1;
        }

        beepMove();
    }

    // =====================================================
    // LEVEL 2 : PINDAH CURSOR
    // =====================================================

    else if (!isEditingValue) {

        item += direction;

        int maxItems = 1;

        switch (page) {

            case PAGE_SET:
                maxItems = SET_COUNT;
                break;

            case PAGE_BOOST:
                maxItems = BOOST_COUNT;
                break;

            case PAGE_SLEEP:
                maxItems = SLEEP_COUNT;
                break;

            case PAGE_CAL:
                maxItems = CAL_COUNT;
                break;

            case PAGE_PID:
                maxItems = PID_COUNT;
                break;

            case PAGE_TIP:
                maxItems = TIP_COUNT;
                break;

            case PAGE_BUZZER:
                maxItems = BUZ_COUNT;
                break;
        }

        if (item >= maxItems) {
            item = 0;
        }

        if (item < 0) {
            item = maxItems - 1;
        }
    }

    // =====================================================
    // LEVEL 3 : EDIT VALUE
    // =====================================================

    else {

        switch (page) {

            // =============================================
            // SETTING
            // =============================================

            case PAGE_SET:

                if (item == SET_TEMP) {

                    targetTemp += direction * 5;

                    targetTemp = constrain(
                        targetTemp,
                        hermenexConfig.tempMin,
                        maxTemp
                    );
                }

                break;

            // =============================================
            // BOOST
            // =============================================

            case PAGE_BOOST:

                if (item == BOOST_TEMP) {

                    boostTemp += direction * 5;

                    boostTemp = constrain(
                        boostTemp,
                        hermenexConfig.tempMin,
                        maxTemp
                    );
                }

                else if (item == BOOST_TIME) {

                    boostTimeSec += direction;

                    boostTimeSec = constrain(
                        boostTimeSec,
                        10,
                        300
                    );
                }

                break;

            // =============================================
            // SLEEP
            // =============================================

            case PAGE_SLEEP:

                if (item == SLEEP_TEMP) {

                    sleepTemp += direction * 5;

                    sleepTemp = constrain(
                        sleepTemp,
                        hermenexConfig.tempMin,
                        250
                    );
                }

                else if (item == SLEEP_TIME) {

                    sleepTimeSec += direction * 10;

                    sleepTimeSec = constrain(
                        sleepTimeSec,
                        10,
                        999
                    );
                }

                break;

            // =============================================
            // PID
            // =============================================

            case PAGE_PID:

                if (item == PID_KP) {

                    kp = constrain(
                        kp + (direction * 0.1),
                        0.0,
                        999.0
                    );
                }

                else if (item == PID_KI) {

                    ki = constrain(
                        ki + (direction * 0.01),
                        0.0,
                        999.0
                    );
                }

                else if (item == PID_KD) {

                    kd = constrain(
                        kd + (direction * 1.0),
                        0.0,
                        999.0
                    );
                }

                break;

            // =============================================
            // CALIBRATION
            // =============================================

            case PAGE_CAL:

                // CAL_SOLDER -> tempOffset solder
                // CAL_HOTAIR -> offset Hot Air
                // jika variabel offset Hot Air sudah tersedia,
                // tambahkan di sini.

                if (item == CAL_SOLDER) {

                    tempOffset += direction;

                    tempOffset = constrain(
                        tempOffset,
                        -50,
                        50
                    );
                }

                break;
        }
    }
}

// =========================================================
// GETTER DASHBOARD HOT AIR
// =========================================================

bool isHotAirFanEdit() {
    return hotAirFanEdit;
}

// =========================================================
// GETTER CURSOR STATION
// =========================================================

int getStationItem() {

    return stationItem;
}

bool isStationMenu() {

    return stationMenu;
}

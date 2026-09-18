#include "GlobalState.h"
#include "config.h"

#include <Arduino.h>
#include <CastorS.h>

// ================= STATUS & STATE MACHINE =================

uint8_t heaterState = STATE_IDLE;

// ================= UI CORE =================

int item = 0;
int page = 0;
bool inMenu = false;
bool inEdit = false;
bool menuClick = false;
bool booting = true;
uint8_t pidStage = 0;
bool isEditingValue = false;

// ================= SYSTEM FLAGS =================

bool sleeping = false;
bool boostMode = false;
bool manualPWMMode = false;
bool buzzerEnabled = true;
bool webControl = false;
bool motionDetected = false;
bool btnHolding = false;

// ================= SAFETY & LIMITS =================

int maxPwmLimit = 255;
bool tipError = false;
bool overHeat = false;

// ================= TIP =================

TipID currentTip = TIP_T12;
uint8_t currentTipMode = 0;

// ================= BOOST =================
// boostTemp / boostTimeSec → CastorS/boost.cpp

// ================= SLEEP =================
// sleepTemp → CastorS/boost.cpp

int sleepTimeSec = 250;
unsigned long sleepTimer = 0;
int savedSleepTemp = 0;

// ================= CALIBRATION =================

int tempOffset = 0;
int adcOffset = 0;
int adcRaw = 0;

// ================= EDIT MODES =================

bool boostEditMode = false;
bool pidEditMode = false;
bool sleepEditMode = false;
bool buzzerEditMode = false;
bool calEditMode = false;

// ================= ENCODER / MENU CACHE =================

int lastEncoder = 0;
int lastMenuPos = 0;

// ================= TIMERS =================

unsigned long lastMotion = 0;
unsigned long btnPressStart = 0;
unsigned long lastActivity = 0;

// ================= ACTIVE TIP LIMIT =================

int maxTemp = 0;

// ================= TEMPERATURE =================

int targetTemp = DEFAULT_TEMP;
int currentTemp = 0;
int pwmOut = 0;

// ================= PID =================
// Nilai sebenarnya diisi setelah profile tip aktif (tip.cpp)

float kp = 0.0f;
float ki = 0.0f;
float kd = 0.0f;

// ================= STATION =================
// 0 = SOLDER (CastorS), 1 = HOTAIR (PolluxS)
// Harus int supaya cocok dengan HERMENEXExternal + menuHandlr

int activeStation = 0;

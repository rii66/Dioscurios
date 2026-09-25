#include "GlobalState.h"
#include "CastorS.h"
#include "boost.h"
#include "buzzer.h"
#include "pid.h"

extern void beepBoost();

unsigned long boostStart = 0;

int boostTimeSec = 15;         
int boostTemp = hermenexConfig.defaultBoostTemp;
int sleepTemp = hermenexConfig.defaultSleepTemp;

int savedTemp = hermenexConfig.defaultTemp;

// ================= BOOST START =================
void startBoost() {
  if (!boostMode) {

    savedTemp = targetTemp;
    targetTemp = boostTemp;

    boostMode = true;
    boostStart = millis();
    beepBoost();
  }
}

// ================= BOOST UPDATE =================
void updateBoost() {

  if (boostMode && (millis() - boostStart > (boostTimeSec * 1000))) {

    boostMode = false;
    targetTemp = savedTemp;
  }
}
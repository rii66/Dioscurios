#include "CastorS.h"
#include "GlobalState.h"
#include <Arduino.h>

#include "motion.h"
#include "buzzer.h" 

void initMotion() {
  pinMode(hermenexConfig.motionPin, INPUT_PULLUP);
}
// Motion //
void wakeFromSleep() {
  motionDetected = true;
  lastMotion = millis();

  if (sleeping) {
    sleeping = false;
    beepWake();
  }
}

void updateMotion() {
  bool motion = digitalRead(hermenexConfig.motionPin);

  /* Bangun Universal*/
  if (motion == LOW) {
    wakeFromSleep();
  }

  /* AUTO SLEEP */
  if (
    sleepTimer > 0 &&
    millis() - lastMotion > (unsigned long)sleepTimeSec * 1000UL
  ) {
    if (!sleeping) {
      sleeping = true;

      beepSleep();  // sleep beep
    }
  }
}

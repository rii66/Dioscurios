#include "PolluxS.h"

PolluxS polluxS;
PolluxState polluxState;

PolluxS::PolluxS() {
}

void PolluxS::begin() {
  hotgun.begin();
}

void PolluxS::update() {
  hotgun.update();

  // State station diekspos ke firmware utama.
  polluxState.currentTemp = hotgun.getTemp();
  polluxState.targetTemp  = hotgun.getTargetTemp();
  polluxState.pwmOut      = hotgun.getPower();
  polluxState.overHeat    = hotgun.isOverHeat();
}

void PolluxS::encoder(int delta) {
  if (delta == 0) return;

  int temp = hotgun.getTargetTemp();
  temp += delta * 5;
  hotgun.setTemp((uint16_t)temp);
}

#ifndef POLLUXS_H
#define POLLUXS_H

#include "PolluxGlobals.h"
#include "fan.h"
#include "hotgun.h"

class PolluxS {
public:
  PolluxS();

  void begin();
  void update();
  void encoder(int delta);

  Fan fan;
  HotGun hotgun;
};

extern PolluxS polluxS;

#endif

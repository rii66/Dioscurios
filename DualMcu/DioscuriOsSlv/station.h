#ifndef STATION_H
#define STATION_H

#include <Arduino.h>

void initStations();
void updateStations();
void monitorStations();


// belum ada rencana pake tombol ganti station 
// void handleStationEncoder(int delta);
// void handleStationButton();

#endif

// Serial debug (optional)
void printStationStatus();

#ifndef MENU_H
#define MENU_H

void handleButton(bool click);
void handleMenu(int dir, bool click);

void saveMenuCursor();
void executePageSelect();

int getStationItem();
bool isStationMenu();
bool isHotAirFanEdit();

#endif

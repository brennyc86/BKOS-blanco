#pragma once
#include "platform.h"

#if PLATFORM_XPT2046
  #include <XPT2046_Touchscreen.h>
  #include <SPI.h>
#elif PLATFORM_ESP32
  #include <TAMC_GT911.h>
  #define TS_SDA  19
  #define TS_SCK  20
  #define TS_RST  38
#endif

extern bool actieve_touch;
extern int  ts_x;
extern int  ts_y;

void ts_setup();
bool ts_touched();
int  touch_x();
int  touch_y();

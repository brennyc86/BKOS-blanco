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

// Gedeelde HSPI bus voor display + touch (WROOM / CYD28)
// HSPI native pins op ESP32: SCK=14, MISO=12, MOSI=13 — matchen exact onze hardware
#if (PLATFORM_WROOM || PLATFORM_CYD28) && PLATFORM_ESP32
  extern SPIClass shared_hspi;
#endif

extern bool actieve_touch;
extern int  ts_x;
extern int  ts_y;

void ts_setup();
bool ts_touched();
int  touch_x();
int  touch_y();

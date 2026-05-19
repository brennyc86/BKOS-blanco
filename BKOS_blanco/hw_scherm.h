#pragma once
#include <Arduino_GFX_Library.h>
#include "platform.h"
#include "ui_colors.h"

#if PLATFORM_XPT2046
  #include <SPI.h>
#endif

extern Arduino_GFX *tft_p;
#define tft (*tft_p)

#define TFT_MIN_HELDER 3

extern int           tft_helderheid;
extern long          scherm_timer;
extern bool          tft_actief;
extern long          scherm_touched;
extern bool          scherm_net_gewekt;
extern bool          tft_bijna_uit;
extern unsigned long tft_dim_ms;

void tft_setup();
void tft_loop();
void tft_helderheid_zet(int pct);

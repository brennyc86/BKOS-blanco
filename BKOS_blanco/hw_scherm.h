#pragma once
#include <Arduino_GFX_Library.h>
#include "ui_colors.h"

#define TFT_BL         2
#define TFT_W          800
#define TFT_H          480
#define TFT_MIN_HELDER 3

Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    41, 40, 39, 42,
    14, 21, 47, 48, 45,
    9,  46,  3,  8, 16, 1,
    15,  7,  6,  5,  4,
    0, 210, 30, 16,
    0,  22, 13, 10,
    1, 16000000);

Arduino_RGB_Display tft(800, 480, rgbpanel, 0, true);

int           tft_helderheid    = 75;
long          scherm_timer      = 60;
bool          tft_actief        = true;
long          scherm_touched    = 0;
bool          scherm_net_gewekt = false;
bool          tft_bijna_uit     = false;
unsigned long tft_dim_ms        = 0;

void tft_setup();
void tft_loop();
void tft_helderheid_zet(int pct);

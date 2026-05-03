#pragma once
#include <Arduino.h>

#define BLANCO_VERSIE "0.0.260503.1"

enum Scherm { SCHERM_KIES = 0, SCHERM_WIFI, SCHERM_FLASH };

extern Scherm        actief_scherm;
extern bool          scherm_bouwen;
extern volatile bool wifi_verbonden;

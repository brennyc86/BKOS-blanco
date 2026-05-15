#pragma once
#include <Arduino.h>
#include "ota.h"

enum Scherm { SCHERM_KIES = 0, SCHERM_WIFI, SCHERM_FLASH };

extern Scherm        actief_scherm;
extern bool          scherm_bouwen;
extern volatile bool wifi_verbonden;

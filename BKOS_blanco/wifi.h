#pragma once
#include <WiFi.h>
#include <Preferences.h>

bool wifi_verbind(const char* ssid, const char* wachtwoord);
bool wifi_verbind_opgeslagen();
void wifi_reset();

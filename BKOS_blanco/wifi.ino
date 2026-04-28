#include "wifi.h"
#include "app_state.h"

bool wifi_verbind(const char* ssid, const char* wachtwoord) {
    WiFi.disconnect(true);
    delay(200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, wachtwoord);
    unsigned long t = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t < 15000) delay(300);
    wifi_verbonden = (WiFi.status() == WL_CONNECTED);
    if (wifi_verbonden) {
        Preferences p;
        p.begin("blanco_wifi", false);
        p.putString("ssid", ssid);
        p.putString("pass", wachtwoord);
        p.end();
    }
    return wifi_verbonden;
}

bool wifi_verbind_opgeslagen() {
    // Probeer eerst NVS-instellingen van ESP32 zelf
    WiFi.mode(WIFI_STA);
    WiFi.begin();
    for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) delay(300);
    if (WiFi.status() == WL_CONNECTED) { wifi_verbonden = true; return true; }

    // Fallback: Preferences
    Preferences p;
    p.begin("blanco_wifi", true);
    String ssid = p.getString("ssid", "");
    String pass = p.getString("pass", "");
    p.end();
    if (ssid.length() == 0) { wifi_verbonden = false; return false; }
    WiFi.begin(ssid.c_str(), pass.c_str());
    for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; i++) delay(300);
    wifi_verbonden = (WiFi.status() == WL_CONNECTED);
    return wifi_verbonden;
}

void wifi_reset() {
    Preferences p;
    p.begin("blanco_wifi", false);
    p.clear();
    p.end();
    WiFi.disconnect(true, true);
    wifi_verbonden = false;
}

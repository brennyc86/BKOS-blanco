#include "hardware.h"

static bool          vorige_touch     = false;
static bool          touch_verwerkt  = false;
static unsigned long laatste_touch   = 0;
#define TOUCH_DEBOUNCE_MS 300

void hw_setup() {
    Serial.begin(115200);
    tft_setup();
    ts_setup();

    // Splash
    tft.fillScreen(C_BG);
    tft.setTextSize(3); tft.setTextColor(C_CYAN);
    tft.setCursor(220, 140); tft.print("BKOS Firmware Kiezer");
    tft.setTextSize(1); tft.setTextColor(C_TEXT_DIM);
    tft.setCursor(340, 190); tft.print("v"); tft.print(BLANCO_VERSIE);

    // Initialiseer versie-strings
    for (int i = 0; i < FW_COUNT; i++) kies_versie_github[i][0] = '\0';

    // WiFi proberen
    tft.setTextSize(1); tft.setTextColor(C_TEXT_DIM);
    tft.setCursor(310, 250); tft.print("WiFi verbinden...");
    wifi_verbind_opgeslagen();

    if (wifi_verbonden) {
        tft.fillRect(260, 240, 280, 20, C_BG);
        tft.setTextColor(C_GREEN);
        tft.setCursor(310, 250); tft.print("WiFi verbonden!");
        delay(600);
    } else {
        tft.fillRect(260, 240, 280, 20, C_BG);
        tft.setTextColor(C_AMBER);
        tft.setCursor(290, 250); tft.print("Geen WiFi — stel in via het kiesscherm");
        delay(1200);
    }

    actief_scherm = SCHERM_KIES;
    scherm_bouwen = true;
}

void hw_loop() {
    bool aanraking = ts_touched();
    tft_loop();

    if (scherm_bouwen) {
        scherm_bouwen = false;
        touch_verwerkt = false;
        switch (actief_scherm) {
            case SCHERM_KIES:  screen_kies_teken();  break;
            case SCHERM_WIFI:  screen_wifi_teken();  break;
            case SCHERM_FLASH: screen_flash_teken(); break;
        }
        return;
    }

    if (aanraking && !vorige_touch) touch_verwerkt = false;

    if (scherm_net_gewekt && aanraking) {
        scherm_net_gewekt = false;
        touch_verwerkt = true;
        laatste_touch = millis();
    } else if (aanraking && !touch_verwerkt) {
        if (millis() - laatste_touch >= TOUCH_DEBOUNCE_MS) {
            touch_verwerkt = true;
            laatste_touch  = millis();
            switch (actief_scherm) {
                case SCHERM_KIES:  screen_kies_run(ts_x, ts_y, true);  break;
                case SCHERM_WIFI:  screen_wifi_run(ts_x, ts_y, true);  break;
                case SCHERM_FLASH: screen_flash_run(ts_x, ts_y, true); break;
            }
        } else {
            touch_verwerkt = true;
        }
    }

    if (!aanraking) touch_verwerkt = false;

    vorige_touch = aanraking;
}

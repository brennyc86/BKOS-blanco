#include "hw_scherm.h"
#include "hw_touch.h"

void tft_setup() {
    pinMode(TFT_BL, OUTPUT);
    tft.begin();
    tft.setRotation(0);
    tft_helderheid_zet(tft_helderheid);
}

void tft_helderheid_zet(int pct) {
    analogWrite(TFT_BL, map(constrain(pct, 0, 100), 0, 100, 0, 255));
}

void tft_loop() {
    if (tft_actief) {
        if (!actieve_touch && scherm_timer > 0 &&
            millis() > (unsigned long)(scherm_touched + (long)scherm_timer * 1000)) {
            tft_actief    = false;
            tft_bijna_uit = true;
            tft_dim_ms    = millis();
            tft_helderheid_zet(TFT_MIN_HELDER);
        }
    } else if (tft_bijna_uit) {
        if (actieve_touch) {
            tft_bijna_uit     = false;
            tft_actief        = true;
            scherm_net_gewekt = true;
            tft_helderheid_zet(tft_helderheid);
        } else if (millis() - tft_dim_ms > 5000UL) {
            tft_bijna_uit = false;
            tft_helderheid_zet(0);
        }
    } else {
        if (actieve_touch) {
            tft_actief        = true;
            scherm_net_gewekt = true;
            tft_helderheid_zet(tft_helderheid);
        }
    }
}

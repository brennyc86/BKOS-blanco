#include "screen_flash.h"
#include <HTTPClient.h>
#include <Update.h>

static int   fl_pct_last = -1;

static void voortgang_teken(int pct) {
    if (pct == fl_pct_last) return;
    fl_pct_last = pct;
    int bar_w = (int)((long)(TFT_W - 100) * pct / 100);
    tft.fillRect(50, 220, TFT_W - 100, 36, C_SURFACE);
    if (bar_w > 0) tft.fillRect(50, 220, bar_w, 36, C_GREEN);
    tft.drawRect(50, 220, TFT_W - 100, 36, C_SURFACE2);
    tft.fillRect(50, 268, 120, 20, C_BG);
    tft.setTextSize(2); tft.setTextColor(C_TEXT);
    tft.setCursor(50, 268);
    char buf[10]; snprintf(buf, sizeof(buf), "%d%%", pct);
    tft.print(buf);
}

static void status_teken(const char* tekst, uint16_t kleur) {
    tft.fillRect(50, 300, TFT_W - 100, 20, C_BG);
    tft.setTextSize(1); tft.setTextColor(kleur);
    tft.setCursor(50, 300); tft.print(tekst);
}

void screen_flash_teken() {
    tft.fillScreen(C_BG);
    int idx = kies_doel;
    if (idx < 0 || idx >= FW_COUNT) { actief_scherm = SCHERM_KIES; scherm_bouwen = true; return; }

    tft.setTextSize(3); tft.setTextColor(C_CYAN);
    tft.setCursor(50, 60); tft.print("Firmware installeren");

    tft.setTextSize(2); tft.setTextColor(FW_KLEUR[idx]);
    tft.setCursor(50, 108);
    tft.print(FW_NAAM[idx]);
    tft.setTextColor(C_TEXT_DIM);
    tft.print(" — "); tft.print(FW_AUTEUR[idx]);

    tft.setTextSize(1); tft.setTextColor(C_TEXT_DIM);
    tft.setCursor(50, 136); tft.print("Niet uitschakelen tijdens het installeren!");
    tft.setCursor(50, 152); tft.print(FW_BIN_URL[idx]);

    tft.fillRect(50, 220, TFT_W - 100, 36, C_SURFACE);
    tft.drawRect(50, 220, TFT_W - 100, 36, C_SURFACE2);
    fl_pct_last = -1;

    // Kleine vertraging zodat gebruiker het scherm ziet
    delay(800);
    screen_flash_start();
}

void screen_flash_start() {
    int idx = kies_doel;
    const char* url = FW_BIN_URL[idx];

    status_teken("Verbinding maken...", C_AMBER);

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    WiFiClientSecure client;
    client.setInsecure();
    http.begin(client, url);
    http.setTimeout(30000);

    status_teken("Downloaden...", C_AMBER);
    int code = http.GET();
    if (code != HTTP_CODE_OK) {
        char buf[40]; snprintf(buf, sizeof(buf), "Download fout: HTTP %d", code);
        status_teken(buf, C_RED_BRIGHT);
        http.end();
        delay(3000);
        actief_scherm = SCHERM_KIES;
        scherm_bouwen = true;
        return;
    }

    int len = http.getSize();
    if (len <= 0) {
        status_teken("Fout: onbekende bestandsgrootte", C_RED_BRIGHT);
        http.end();
        delay(3000);
        actief_scherm = SCHERM_KIES;
        scherm_bouwen = true;
        return;
    }

    if (!Update.begin(len)) {
        status_teken("Fout: onvoldoende flashruimte", C_RED_BRIGHT);
        http.end();
        delay(3000);
        actief_scherm = SCHERM_KIES;
        scherm_bouwen = true;
        return;
    }

    WiFiClient* stream = http.getStreamPtr();
    size_t written = 0;
    uint8_t buf[512];
    unsigned long last_data = millis();

    while (written < (size_t)len) {
        if (stream->available()) {
            size_t rd = stream->read(buf, sizeof(buf));
            if (rd > 0) {
                Update.write(buf, rd);
                written += rd;
                last_data = millis();
                voortgang_teken((int)(written * 100UL / len));
            }
        }
        if (millis() - last_data > 20000) {
            Update.abort();
            http.end();
            status_teken("Fout: timeout — verbinding verloren", C_RED_BRIGHT);
            delay(3000);
            actief_scherm = SCHERM_KIES;
            scherm_bouwen = true;
            return;
        }
        yield();
    }
    http.end();

    if (!Update.end()) {
        status_teken("Fout bij afronden van update", C_RED_BRIGHT);
        delay(3000);
        actief_scherm = SCHERM_KIES;
        scherm_bouwen = true;
        return;
    }

    voortgang_teken(100);
    status_teken("Klaar! Apparaat herstart...", C_GREEN);
    tft.setTextSize(2); tft.setTextColor(C_GREEN);
    tft.setCursor(50, 330); tft.print("Installatie geslaagd!");
    delay(2000);
    ESP.restart();
}

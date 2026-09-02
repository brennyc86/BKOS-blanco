#include "screen_flash.h"
#include <HTTPClient.h>
#if PLATFORM_ESP32
  #include <WiFiClientSecure.h>
  #include <Update.h>
#endif

static int fl_pct_last = -1;

#if SCREEN_SMALL
  #define FL_MARGIN   20
  #define FL_BAR_Y    90
  #define FL_BAR_H    20
  #define FL_PCT_Y   116
  #define FL_STAT_Y  136
  #define FL_OK_Y    160
#else
  #define FL_MARGIN   50
  #define FL_BAR_Y   220
  #define FL_BAR_H    36
  #define FL_PCT_Y   268
  #define FL_STAT_Y  300
  #define FL_OK_Y    330
#endif

#define FL_BAR_X  FL_MARGIN
#define FL_BAR_W  (TFT_W - FL_MARGIN * 2)

static void voortgang_teken(int pct) {
    if (pct == fl_pct_last) return;
    fl_pct_last = pct;
    int bar_w = (int)((long)FL_BAR_W * pct / 100);
    tft.fillRect(FL_BAR_X, FL_BAR_Y, FL_BAR_W, FL_BAR_H, C_SURFACE);
    if (bar_w > 0) tft.fillRect(FL_BAR_X, FL_BAR_Y, bar_w, FL_BAR_H, C_GREEN);
    tft.drawRect(FL_BAR_X, FL_BAR_Y, FL_BAR_W, FL_BAR_H, C_SURFACE2);
    tft.fillRect(FL_BAR_X, FL_PCT_Y, 80, 14, C_BG);
#if SCREEN_SMALL
    tft.setTextSize(1);
#else
    tft.setTextSize(2);
#endif
    tft.setTextColor(C_TEXT);
    tft.setCursor(FL_BAR_X, FL_PCT_Y);
    char buf[10]; snprintf(buf, sizeof(buf), "%d%%", pct);
    tft.print(buf);
}

static void status_teken(const char* tekst, uint16_t kleur) {
    tft.fillRect(FL_BAR_X, FL_STAT_Y, FL_BAR_W, 14, C_BG);
    tft.setTextSize(1); tft.setTextColor(kleur);
    tft.setCursor(FL_BAR_X, FL_STAT_Y);
    tft.print(tekst);
}

static bool fl_bevestigd = false;

// Bevestig-knoppen delen de FL_BAR-rij (die pas na bevestiging de
// voortgangsbalk wordt). Links = ANNULEREN, rechts = BEVESTIGEN.
#define FL_BTN_GAP  10
#define FL_BTN_W    ((FL_BAR_W - FL_BTN_GAP) / 2)
#define FL_ANN_X    FL_BAR_X
#define FL_BEV_X    (FL_BAR_X + FL_BTN_W + FL_BTN_GAP)

void screen_flash_teken() {
    tft.fillScreen(C_BG);
    int idx = kies_doel;
    if (idx < 0 || idx >= FW_COUNT) { actief_scherm = SCHERM_KIES; scherm_bouwen = true; return; }
    if (kies_flash_url.length() == 0) kies_flash_url = FW_BIN_URL[idx];
    fl_bevestigd = false;

#if SCREEN_SMALL
    tft.setTextSize(1); tft.setTextColor(C_CYAN);
    tft.setCursor(FL_BAR_X, 10); tft.print("Firmware installeren?");

    tft.setTextSize(1); tft.setTextColor(FW_KLEUR[idx]);
    tft.setCursor(FL_BAR_X, 28);
    tft.print(FW_NAAM[idx]);
    if (kies_flash_versie.length()) { tft.setTextColor(C_TEXT_DIM); tft.print(" v"); tft.print(kies_flash_versie); }

    tft.setTextSize(1); tft.setTextColor(C_TEXT_DIM);
    tft.setCursor(FL_BAR_X, 46); tft.print("Controleer het adres:");

    tft.setCursor(FL_BAR_X, 62);
    char urlbuf[31]; strncpy(urlbuf, kies_flash_url.c_str(), 30); urlbuf[30] = '\0';
    tft.print(urlbuf);
#else
    tft.setTextSize(2); tft.setTextColor(C_CYAN);
    tft.setCursor(FL_BAR_X, 60); tft.print("Firmware installeren?");

    tft.setTextSize(2); tft.setTextColor(FW_KLEUR[idx]);
    tft.setCursor(FL_BAR_X, 108);
    tft.print(FW_NAAM[idx]);
    if (kies_flash_versie.length()) { tft.setTextColor(C_TEXT_DIM); tft.print(" v"); tft.print(kies_flash_versie); }

    tft.setTextSize(1); tft.setTextColor(C_TEXT_DIM);
    tft.setCursor(FL_BAR_X, 136); tft.print("Controleer het adres voordat je bevestigt:");
    tft.setCursor(FL_BAR_X, 152); tft.print(kies_flash_url);
#endif

    ui_knop(FL_ANN_X, FL_BAR_Y, FL_BTN_W, FL_BAR_H, "ANNULEREN", C_SURFACE2, C_TEXT_DIM);
    ui_knop(FL_BEV_X, FL_BAR_Y, FL_BTN_W, FL_BAR_H, "BEVESTIGEN", C_SURFACE2, C_GREEN);
    fl_pct_last = -1;

    // Op kleine schermen kan de BEVESTIGEN-knop op dezelfde plek vallen als de
    // INSTALLEREN-knop van de kaart waar net op getikt is — een vinger die nog
    // vasthangt van die tik mag hier niet meteen als een (verkeerde) bevestiging
    // tellen. Consumeer daarom de eerstvolgende aanraking zonder te reageren
    // (zelfde mechanisme als bij het wekken van een gedimd scherm).
    scherm_net_gewekt = true;
}

void screen_flash_run(int x, int y, bool aanraking) {
    if (!aanraking || fl_bevestigd) return;
    if (y < FL_BAR_Y || y >= FL_BAR_Y + FL_BAR_H) return;

    if (x >= FL_ANN_X && x < FL_ANN_X + FL_BTN_W) {
        actief_scherm = SCHERM_KIES;
        scherm_bouwen = true;
    } else if (x >= FL_BEV_X && x < FL_BEV_X + FL_BTN_W) {
        fl_bevestigd = true;
        tft.fillRect(FL_BAR_X, FL_BAR_Y, FL_BAR_W, FL_BAR_H, C_SURFACE);
        tft.drawRect(FL_BAR_X, FL_BAR_Y, FL_BAR_W, FL_BAR_H, C_SURFACE2);
        fl_pct_last = -1;
        screen_flash_start();
    }
}

void screen_flash_start() {
    int idx = kies_doel;
    if (kies_flash_url.length() == 0) kies_flash_url = FW_BIN_URL[idx];
    const char* url = kies_flash_url.c_str();

    status_teken("Verbinding maken...", C_AMBER);

#if PLATFORM_ESP32
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
    uint8_t buf2[512];
    unsigned long last_data = millis();

    while (written < (size_t)len) {
        if (stream->available()) {
            size_t rd = stream->read(buf2, sizeof(buf2));
            if (rd > 0) {
                Update.write(buf2, rd);
                written += rd;
                last_data = millis();
                voortgang_teken((int)(written * 100UL / len));
            }
        }
        if (millis() - last_data > 20000) {
            Update.abort();
            http.end();
            status_teken("Fout: timeout \x97 verbinding verloren", C_RED_BRIGHT);
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
#if SCREEN_SMALL
    tft.setTextSize(1);
#else
    tft.setTextSize(2);
#endif
    tft.setTextColor(C_GREEN);
    tft.setCursor(FL_BAR_X, FL_OK_Y);
    tft.print("Installatie geslaagd!");
    delay(2000);
    PLATFORM_REBOOT();

#else
    // Pico: OTA flash wordt niet ondersteund in deze versie
    status_teken("OTA niet ondersteund op Pico", C_RED_BRIGHT);
    delay(3000);
    actief_scherm = SCHERM_KIES;
    scherm_bouwen = true;
#endif
}

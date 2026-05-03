#include "screen_kies.h"
#include <HTTPClient.h>

// ─── Firmware tabel ─────────────────────────────────────────────────────────
const char* FW_NAAM[FW_COUNT] = {
    "BKOS-NUI",
    "BKOS4",
    "BKOS5a",
    "BKOS-blanco"
};

const char* FW_AUTEUR[FW_COUNT] = {
    "Claude Code",
    "Brendan Koster",
    "Agent Zero",
    "Claude Code"
};

const char* FW_INFO[FW_COUNT] = {
    "Volledig systeem: IO, meteo, kleurpaletten, navigatie",
    "Originele handgeschreven versie",
    "Agent Zero gegenereerde versie",
    "Firmware kiezer (dit systeem) — zelf updaten"
};

// Controleer deze URLs in de repo's als ze niet werken!
const char* FW_VERSIE_URL[FW_COUNT] = {
    "https://raw.githubusercontent.com/brennyc86/BKOS-NUI/main/BKOS_NUI/versie.txt",
    "https://raw.githubusercontent.com/BrendanKoster86/BKOS4/main/versie.txt",
    "https://raw.githubusercontent.com/brennyc86/BKOS5a/main/versie.txt",
    "https://raw.githubusercontent.com/brennyc86/BKOS-blanco/main/BKOS_blanco/versie.txt"
};

const char* FW_BIN_URL[FW_COUNT] = {
    "https://raw.githubusercontent.com/brennyc86/BKOS-NUI/main/BKOS_NUI/firmware.bin",
    "https://raw.githubusercontent.com/BrendanKoster86/BKOS4/main/build/esp32.esp32.esp32s3/BKOS4.ino.bin",
    "https://raw.githubusercontent.com/brennyc86/BKOS5a/main/build/esp32.esp32.esp32s3/BKOS5a.ino.bin",
    "https://raw.githubusercontent.com/brennyc86/BKOS-blanco/main/BKOS_blanco/firmware.bin"
};

uint16_t FW_KLEUR[FW_COUNT] = {
    0x07FF,  // C_CYAN  — BKOS-NUI
    0xFD20,  // C_AMBER — BKOS4
    0x07E0,  // C_GREEN — BKOS5a
    0x4208   // C_DARK_GRAY — BKOS-blanco
};

int  kies_doel = -1;
char kies_versie_github[FW_COUNT][32];

// ─── Layout ─────────────────────────────────────────────────────────────────
#define CARD_H    96
#define CARD_GAP   5
#define CARD_X    10
#define CARD_W    (TFT_W - 20)
#define CARD_Y(i) (HEADER_H + 2 + (i) * (CARD_H + CARD_GAP))

#define BTN_INSTALL_W  158
#define BTN_INSTALL_X  (CARD_X + CARD_W - BTN_INSTALL_W - 8)

// ─── Hulpfuncties ───────────────────────────────────────────────────────────
static void wifi_status_teken() {
    // Rechts in header
    uint16_t dot_c = wifi_verbonden ? C_GREEN : C_RED_BRIGHT;
    int dx = TFT_W - 318, dy = (HEADER_H - 10) / 2;
    tft.fillCircle(dx, dy + 5, 5, dot_c);
    tft.setTextSize(1);
    tft.setTextColor(dot_c);
    tft.setCursor(dx + 10, dy + 1);
    tft.print(wifi_verbonden ? "Verbonden" : "Geen WiFi");

    ui_knop(TFT_W - 232, 7, 100, 30, "VERSIES", C_SURFACE2, C_CYAN);
    ui_knop(TFT_W - 124, 7, 112, 30, "WIFI",    C_SURFACE2,
            wifi_verbonden ? C_GREEN : C_RED_BRIGHT);
}

static void kaart_teken(int idx) {
    int cy = CARD_Y(idx);
    uint16_t ac = FW_KLEUR[idx];

    tft.fillRoundRect(CARD_X, cy, CARD_W, CARD_H, 8, C_SURFACE);
    tft.drawRoundRect(CARD_X, cy, CARD_W, CARD_H, 8, C_SURFACE2);
    // Kleuraccent linksbalk
    tft.fillRoundRect(CARD_X, cy, 6, CARD_H, 4, ac);

    // Naam
    tft.setTextSize(2);
    tft.setTextColor(ac);
    tft.setCursor(CARD_X + 16, cy + 10);
    tft.print(FW_NAAM[idx]);

    // Auteur badge
    int nx = CARD_X + 16 + strlen(FW_NAAM[idx]) * 12 + 8;
    int by = cy + 12;
    int bw = strlen(FW_AUTEUR[idx]) * 6 + 8;
    tft.fillRoundRect(nx, by, bw, 14, 4, C_SURFACE2);
    tft.setTextSize(1); tft.setTextColor(C_TEXT_DIM);
    tft.setCursor(nx + 4, by + 3);
    tft.print(FW_AUTEUR[idx]);

    // Beschrijving
    tft.setTextSize(1); tft.setTextColor(C_TEXT_DIM);
    tft.setCursor(CARD_X + 16, cy + 34);
    tft.print(FW_INFO[idx]);

    // GitHub versie
    tft.setTextSize(1); tft.setTextColor(C_TEXT_DIM);
    tft.setCursor(CARD_X + 16, cy + 56);
    tft.print("GitHub: ");
    const char* vstr = kies_versie_github[idx];
    uint16_t vc = (strlen(vstr) == 0) ? C_TEXT_DIM :
                  (strncmp(vstr, "fout", 4) == 0) ? C_RED_BRIGHT :
                  (strncmp(vstr, "...", 3) == 0) ? C_AMBER : C_GREEN;
    tft.setTextColor(vc);
    tft.print((strlen(vstr) == 0) ? "onbekend" : vstr);

    // Huidig versie (alleen BKOS-blanco toont zijn eigen versie)
    if (idx == FW_COUNT - 1) {
        tft.setTextSize(1); tft.setTextColor(C_TEXT_DIM);
        tft.setCursor(CARD_X + 16, cy + 70);
        tft.print("Huidig: "); tft.setTextColor(C_TEXT); tft.print(BLANCO_VERSIE);
    }

    // INSTALLEREN knop
    uint16_t btn_c = (idx == FW_COUNT - 1) ? C_SURFACE2 : C_SURFACE2;
    uint16_t btn_t = (idx == FW_COUNT - 1) ? C_TEXT_DIM : ac;
    tft.fillRoundRect(BTN_INSTALL_X, cy + CARD_H - 46, BTN_INSTALL_W, 38, 6, btn_c);
    tft.drawRoundRect(BTN_INSTALL_X, cy + CARD_H - 46, BTN_INSTALL_W, 38, 6, btn_t);
    tft.setTextSize(1); tft.setTextColor(btn_t);
    int tw = strlen("INSTALLEREN") * 6;
    tft.setCursor(BTN_INSTALL_X + (BTN_INSTALL_W - tw) / 2, cy + CARD_H - 31);
    tft.print("INSTALLEREN");
}

// ─── Versies ophalen van GitHub ──────────────────────────────────────────────
void kies_versies_ophalen() {
    if (!wifi_verbonden) return;
    HTTPClient http;
    for (int i = 0; i < FW_COUNT; i++) {
        strcpy(kies_versie_github[i], "...");
        kaart_teken(i);  // toon "..."
        http.begin(FW_VERSIE_URL[i]);
        int code = http.GET();
        if (code == HTTP_CODE_OK) {
            String v = http.getString();
            v.trim();
            strncpy(kies_versie_github[i], v.c_str(), 31);
            kies_versie_github[i][31] = '\0';
        } else {
            snprintf(kies_versie_github[i], 32, "fout %d", code);
        }
        http.end();
        kaart_teken(i);  // update met werkelijke versie
    }
}

// ─── Publieke functies ────────────────────────────────────────────────────────
void screen_kies_teken() {
    tft.fillScreen(C_BG);

    // Header
    tft.fillRect(0, 0, TFT_W, HEADER_H, C_SURFACE);
    tft.drawFastHLine(0, HEADER_H - 1, TFT_W, C_SURFACE2);
    tft.setTextSize(2); tft.setTextColor(C_CYAN);
    tft.setCursor(12, (HEADER_H - 16) / 2);
    tft.print("BKOS FIRMWARE KIEZER");
    wifi_status_teken();

    for (int i = 0; i < FW_COUNT; i++) kaart_teken(i);
}

void screen_kies_run(int x, int y, bool aanraking) {
    if (!aanraking) return;

    // Header knoppen
    if (y < HEADER_H) {
        if (x >= TFT_W - 232 && x < TFT_W - 124) {
            // VERSIES ophalen
            kies_versies_ophalen();
        } else if (x >= TFT_W - 124) {
            // WIFI scherm
            actief_scherm = SCHERM_WIFI;
            scherm_bouwen = true;
        }
        return;
    }

    // Kaart aantikken: controleer INSTALLEREN knop
    for (int i = 0; i < FW_COUNT; i++) {
        int cy = CARD_Y(i);
        if (y >= cy && y < cy + CARD_H) {
            int btn_y = cy + CARD_H - 46;
            if (x >= BTN_INSTALL_X && x < BTN_INSTALL_X + BTN_INSTALL_W &&
                y >= btn_y && y < btn_y + 38) {
                if (!wifi_verbonden) {
                    // Geen WiFi — toon melding
                    tft.fillRoundRect(CARD_X + 4, cy + 4, CARD_W - 8, CARD_H - 8, 6, C_SURFACE);
                    tft.setTextSize(1); tft.setTextColor(C_RED_BRIGHT);
                    tft.setCursor(CARD_X + 16, cy + CARD_H / 2 - 4);
                    tft.print("Geen WiFi — ga naar WIFI om te verbinden");
                    delay(2000);
                    kaart_teken(i);
                    return;
                }
                kies_doel = i;
                actief_scherm = SCHERM_FLASH;
                scherm_bouwen = true;
            }
            return;
        }
    }
}

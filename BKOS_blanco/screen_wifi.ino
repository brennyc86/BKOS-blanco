#include "screen_wifi.h"

// ─── State ──────────────────────────────────────────────────────────────────
#define WIFI_ST_IDLE       0
#define WIFI_ST_SCANNING   1
#define WIFI_ST_LIJST      2
#define WIFI_ST_WACHTWOORD 3
#define WIFI_ST_VERBINDEN  4

static byte  wf_staat          = WIFI_ST_IDLE;
static int   wf_n_netwerken    = 0;
static int   wf_scroll         = 0;
static char  wf_ssid[33]       = "";
static char  wf_pass[64]       = "";
static bool  kb_hoofd          = true;   // hoofdletters
static bool  kb_sym            = false;  // speciale tekens
static unsigned long wf_sloot  = 0;

// ─── Netwerk lijst ──────────────────────────────────────────────────────────
#define WF_RIJ_H    52
#define WF_RIJEN_N   6
#define WF_LIST_Y   (CONTENT_Y + 46)

// ─── Toetsenbord layout ──────────────────────────────────────────────────────
static const char* kb_groot[4]    = {"1234567890", "QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM_*@"};
static const char* kb_klein[4]    = {"1234567890", "qwertyuiop", "asdfghjkl", "zxcvbnm_*@"};
static const char* kb_sym_r[4]    = {"!\"#$%&'()*", "+,-./:;<=>", "?@[\\]^_{|}~", ""};

#define KB_X      30
#define KB_W      (TFT_W - 60)
#define KB_INV_Y  (CONTENT_Y + 6)
#define KB_INV_H  46
#define KB_KEYS_Y (KB_INV_Y + KB_INV_H + 8)
#define KB_TH     44
#define KB_BTN_Y  (KB_KEYS_Y + 4 * (KB_TH + 4) + 6)
#define KB_BTN_H  42

static void kb_teken() {
    tft.fillRect(0, CONTENT_Y, TFT_W, CONTENT_H, C_SURFACE);

    // Invoerveld
    tft.fillRoundRect(KB_X, KB_INV_Y, KB_W, KB_INV_H, 6, C_SURFACE2);
    tft.drawRoundRect(KB_X, KB_INV_Y, KB_W, KB_INV_H, 6, C_CYAN);
    tft.setTextSize(1);
    tft.setTextColor(C_TEXT_DIM);
    tft.setCursor(KB_X + 10, KB_INV_Y + 5);
    tft.print("Wachtwoord voor: ");
    tft.setTextColor(C_CYAN);
    tft.print(wf_ssid);
    tft.setTextSize(2);
    tft.setTextColor(C_WHITE);
    tft.setCursor(KB_X + 10, KB_INV_Y + 22);
    size_t passlen = strlen(wf_pass);
    for (size_t i = 0; i < passlen; i++) tft.print("*");
    tft.print("_");

    // Toetsenbord
    const char** rijen = kb_sym ? kb_sym_r : (kb_hoofd ? kb_groot : kb_klein);
    for (int rij = 0; rij < 4; rij++) {
        const char* keys = rijen[rij];
        int cnt = strlen(keys);
        if (cnt == 0) continue;
        int tw = KB_W / cnt;
        for (int k = 0; k < cnt; k++) {
            int kx = KB_X + k * tw;
            int ky = KB_KEYS_Y + rij * (KB_TH + 4);
            tft.fillRoundRect(kx + 2, ky + 2, tw - 4, KB_TH - 4, 5, C_SURFACE2);
            tft.drawRoundRect(kx + 2, ky + 2, tw - 4, KB_TH - 4, 5, C_SURFACE3);
            tft.setTextSize(2);
            tft.setTextColor(C_TEXT);
            tft.setCursor(kx + (tw - 12) / 2, ky + (KB_TH - 16) / 2);
            tft.print(keys[k]);
        }
    }

    // Actie-knoppen
    int bx = KB_X;
    // DEL
    tft.fillRoundRect(bx,       KB_BTN_Y, 80,  KB_BTN_H, 6, C_SURFACE2);
    tft.drawRoundRect(bx,       KB_BTN_Y, 80,  KB_BTN_H, 6, C_RED_BRIGHT);
    tft.setTextSize(1); tft.setTextColor(C_RED_BRIGHT);
    tft.setCursor(bx + 10, KB_BTN_Y + 17); tft.print("< DEL");

    // CLR
    tft.fillRoundRect(bx + 86,  KB_BTN_Y, 70,  KB_BTN_H, 6, C_SURFACE2);
    tft.drawRoundRect(bx + 86,  KB_BTN_Y, 70,  KB_BTN_H, 6, C_AMBER);
    tft.setTextSize(1); tft.setTextColor(C_AMBER);
    tft.setCursor(bx + 101, KB_BTN_Y + 17); tft.print("CLR");

    // CAPS / KLEIN
    uint16_t caps_c = kb_hoofd ? C_CYAN : C_TEXT_DIM;
    tft.fillRoundRect(bx + 162, KB_BTN_Y, 82,  KB_BTN_H, 6, C_SURFACE2);
    tft.drawRoundRect(bx + 162, KB_BTN_Y, 82,  KB_BTN_H, 6, caps_c);
    tft.setTextSize(1); tft.setTextColor(caps_c);
    tft.setCursor(bx + 170, KB_BTN_Y + 17);
    tft.print(kb_hoofd ? "HOOFD" : "klein");

    // SYM / ABC
    uint16_t sym_c = kb_sym ? C_CYAN : C_TEXT_DIM;
    tft.fillRoundRect(bx + 250, KB_BTN_Y, 68,  KB_BTN_H, 6, C_SURFACE2);
    tft.drawRoundRect(bx + 250, KB_BTN_Y, 68,  KB_BTN_H, 6, sym_c);
    tft.setTextSize(1); tft.setTextColor(sym_c);
    tft.setCursor(bx + 262, KB_BTN_Y + 17);
    tft.print(kb_sym ? "ABC" : "SYM");

    // SPATIE
    tft.fillRoundRect(bx + 324, KB_BTN_Y, 110, KB_BTN_H, 6, C_SURFACE2);
    tft.drawRoundRect(bx + 324, KB_BTN_Y, 110, KB_BTN_H, 6, C_SURFACE3);
    tft.setTextSize(1); tft.setTextColor(C_TEXT);
    tft.setCursor(bx + 355, KB_BTN_Y + 17); tft.print("SPATIE");

    // VERBINDEN
    tft.fillRoundRect(bx + 440, KB_BTN_Y, 150, KB_BTN_H, 6, C_GREEN);
    tft.setTextSize(1); tft.setTextColor(C_TEXT_DARK);
    tft.setCursor(bx + 466, KB_BTN_Y + 17); tft.print("VERBINDEN");

    // CANCEL
    tft.fillRoundRect(bx + 596, KB_BTN_Y, 110, KB_BTN_H, 6, C_SURFACE2);
    tft.drawRoundRect(bx + 596, KB_BTN_Y, 110, KB_BTN_H, 6, C_SURFACE3);
    tft.setTextSize(1); tft.setTextColor(C_TEXT_DIM);
    tft.setCursor(bx + 621, KB_BTN_Y + 17); tft.print("ANNULEER");
}

static void kb_run(int x, int y) {
    const char** rijen = kb_sym ? kb_sym_r : (kb_hoofd ? kb_groot : kb_klein);
    for (int rij = 0; rij < 4; rij++) {
        const char* keys = rijen[rij];
        int cnt = strlen(keys);
        if (cnt == 0) continue;
        int tw = KB_W / cnt;
        int ky = KB_KEYS_Y + rij * (KB_TH + 4);
        if (y >= ky && y < ky + KB_TH) {
            int k = (x - KB_X) / tw;
            if (k >= 0 && k < cnt) {
                int len = strlen(wf_pass);
                if (len < 63) { wf_pass[len] = keys[k]; wf_pass[len+1] = '\0'; }
                kb_teken();
                return;
            }
        }
    }

    if (y >= KB_BTN_Y && y < KB_BTN_Y + KB_BTN_H) {
        int bx = KB_X;
        if (x >= bx && x < bx + 80) {
            // DEL
            int len = strlen(wf_pass);
            if (len > 0) wf_pass[len - 1] = '\0';
            kb_teken();
        } else if (x >= bx + 86 && x < bx + 156) {
            // CLR
            wf_pass[0] = '\0';
            kb_teken();
        } else if (x >= bx + 162 && x < bx + 244) {
            // CAPS/KLEIN toggle
            kb_hoofd = !kb_hoofd;
            if (kb_hoofd) kb_sym = false;
            kb_teken();
        } else if (x >= bx + 250 && x < bx + 318) {
            // SYM/ABC toggle
            kb_sym = !kb_sym;
            if (kb_sym) kb_hoofd = false;
            kb_teken();
        } else if (x >= bx + 324 && x < bx + 434) {
            // SPATIE
            int len = strlen(wf_pass);
            if (len < 63) { wf_pass[len] = ' '; wf_pass[len+1] = '\0'; }
            kb_teken();
        } else if (x >= bx + 440 && x < bx + 590) {
            // VERBINDEN
            wf_staat = WIFI_ST_VERBINDEN;
            tft.fillRect(0, CONTENT_Y, TFT_W, CONTENT_H, C_SURFACE);
            tft.setTextSize(2); tft.setTextColor(C_CYAN);
            tft.setCursor(40, CONTENT_Y + 80);
            tft.print("Verbinden met: "); tft.print(wf_ssid);
            tft.setTextColor(C_TEXT_DIM);
            tft.setCursor(40, CONTENT_Y + 110);
            tft.print("Even geduld...");
            bool ok = wifi_verbind(wf_ssid, wf_pass);
            tft.fillRect(40, CONTENT_Y + 80, TFT_W - 80, 80, C_SURFACE);
            if (ok) {
                tft.setTextColor(C_GREEN);
                tft.setCursor(40, CONTENT_Y + 80);
                tft.print("Verbonden! Ga terug naar kiezer.");
                delay(1500);
                wf_staat = WIFI_ST_IDLE;
                actief_scherm = SCHERM_KIES;
                scherm_bouwen = true;
            } else {
                tft.setTextColor(C_RED_BRIGHT);
                tft.setCursor(40, CONTENT_Y + 80);
                tft.print("Verbinding mislukt. Probeer opnieuw.");
                delay(2000);
                wf_staat = WIFI_ST_LIJST;
                scherm_bouwen = true;
            }
        } else if (x >= bx + 596) {
            // ANNULEER
            wf_staat = WIFI_ST_LIJST;
            wf_pass[0] = '\0';
            scherm_bouwen = true;
        }
    }
}

// ─── Netwerk lijst ──────────────────────────────────────────────────────────
static void lijst_teken() {
    tft.fillRect(0, CONTENT_Y, TFT_W, CONTENT_H, C_BG);

    tft.fillRoundRect(8, CONTENT_Y + 4, TFT_W - 16, 36, 6, C_SURFACE);
    tft.setTextSize(2); tft.setTextColor(C_TEXT_DIM);
    tft.setCursor(16, CONTENT_Y + 12);
    char hdr[40]; snprintf(hdr, sizeof(hdr), "%d netwerken gevonden", wf_n_netwerken);
    tft.print(hdr);
    tft.setTextSize(1); tft.setTextColor(C_TEXT_DIM);
    tft.setCursor(TFT_W - 170, CONTENT_Y + 16);
    tft.print("Tik om wachtwoord in te voeren");

    int max_rij = min(wf_n_netwerken - wf_scroll, WF_RIJEN_N);
    for (int r = 0; r < max_rij; r++) {
        int idx = wf_scroll + r;
        int ry  = WF_LIST_Y + r * WF_RIJ_H;
        tft.fillRoundRect(10, ry + 2, TFT_W - 20, WF_RIJ_H - 4, 6,
                          (r % 2 == 0) ? C_SURFACE : C_BG);
        tft.setTextSize(2); tft.setTextColor(C_TEXT);
        tft.setCursor(18, ry + (WF_RIJ_H - 16) / 2);
        tft.print(WiFi.SSID(idx));
        int rssi  = WiFi.RSSI(idx);
        uint16_t sc = (rssi > -50) ? C_GREEN : (rssi > -70) ? C_AMBER : C_RED_BRIGHT;
        int bars  = (rssi > -50) ? 4 : (rssi > -65) ? 3 : (rssi > -80) ? 2 : 1;
        int bx = TFT_W - 60, by = ry + 10;
        for (int b = 0; b < 4; b++) {
            int bh = 6 + b * 5;
            tft.fillRect(bx + b * 12, by + (20 - bh), 8, bh,
                         (b < bars) ? sc : C_SURFACE3);
        }
        bool open = (WiFi.encryptionType(idx) == WIFI_AUTH_OPEN);
        tft.setTextSize(1); tft.setTextColor(open ? C_GREEN : C_TEXT_DIM);
        tft.setCursor(bx - 50, ry + (WF_RIJ_H - 8) / 2);
        tft.print(open ? "OPEN" : "WPA");
    }

    if (wf_n_netwerken > WF_RIJEN_N) {
        tft.setTextSize(1); tft.setTextColor(C_TEXT_DIM);
        tft.setCursor(10, WF_LIST_Y + WF_RIJEN_N * WF_RIJ_H + 6);
        tft.print("< links = vorige pagina   rechts = volgende pagina >");
    }
}

// ─── Publieke functies ───────────────────────────────────────────────────────
void screen_wifi_teken() {
    tft.fillScreen(C_BG);
    ui_header("WIFI NETWERKEN");
    ui_knop(TFT_W - 130, 7, 118, 30, "< TERUG", C_SURFACE2, C_TEXT_DIM);

    if (wf_staat == WIFI_ST_WACHTWOORD) {
        kb_teken();
    } else if (wf_staat == WIFI_ST_LIJST) {
        lijst_teken();
    } else {
        // IDLE
        tft.fillRect(0, CONTENT_Y, TFT_W, CONTENT_H, C_BG);
        if (wifi_verbonden) {
            tft.setTextSize(2); tft.setTextColor(C_GREEN);
            tft.setCursor(40, CONTENT_Y + 50);
            tft.print("Verbonden: "); tft.print(WiFi.SSID());
            tft.setTextSize(1); tft.setTextColor(C_TEXT_DIM);
            tft.setCursor(40, CONTENT_Y + 76);
            tft.print("IP: "); tft.print(WiFi.localIP().toString());
        }
        ui_knop(60, CONTENT_Y + 110, TFT_W - 120, 56, "SCANNEN", C_SURFACE, C_CYAN);
        ui_knop(60, CONTENT_Y + 178, TFT_W - 120, 56, "WIFI VERGETEN", C_SURFACE, C_RED_BRIGHT);
    }
}

void screen_wifi_run(int x, int y, bool aanraking) {
    if (!aanraking) return;
    if (millis() - wf_sloot < 300) return;
    wf_sloot = millis();

    if (y < HEADER_H) {
        // TERUG knop
        if (x >= TFT_W - 130) {
            wf_staat = WIFI_ST_IDLE;
            actief_scherm = SCHERM_KIES;
            scherm_bouwen = true;
        }
        return;
    }

    if (wf_staat == WIFI_ST_WACHTWOORD) {
        kb_run(x, y);
        return;
    }

    if (wf_staat == WIFI_ST_IDLE) {
        if (y >= CONTENT_Y + 110 && y < CONTENT_Y + 166) {
            // SCANNEN
            wf_staat = WIFI_ST_SCANNING;
            tft.fillRect(0, CONTENT_Y, TFT_W, CONTENT_H, C_BG);
            tft.setTextSize(2); tft.setTextColor(C_CYAN);
            tft.setCursor(60, CONTENT_Y + 100); tft.print("Scannen...");
            WiFi.mode(WIFI_STA);
            wf_n_netwerken = WiFi.scanNetworks();
            wf_scroll  = 0;
            wf_staat   = WIFI_ST_LIJST;
            lijst_teken();
        } else if (y >= CONTENT_Y + 178 && y < CONTENT_Y + 234) {
            // VERGETEN
            tft.fillRect(0, CONTENT_Y, TFT_W, CONTENT_H, C_BG);
            tft.setTextSize(2); tft.setTextColor(C_RED_BRIGHT);
            tft.setCursor(40, CONTENT_Y + 80);
            tft.print("WiFi wordt gewist...");
            delay(1000);
            wifi_reset();
            scherm_bouwen = true;
        }
        return;
    }

    if (wf_staat == WIFI_ST_LIJST) {
        if (y >= WF_LIST_Y && y < WF_LIST_Y + WF_RIJEN_N * WF_RIJ_H) {
            int rij = (y - WF_LIST_Y) / WF_RIJ_H;
            int idx = wf_scroll + rij;
            if (idx >= 0 && idx < wf_n_netwerken) {
                String ssid = WiFi.SSID(idx);
                strncpy(wf_ssid, ssid.c_str(), 32); wf_ssid[32] = '\0';
                wf_pass[0] = '\0';
                kb_hoofd = true; kb_sym = false;
                wf_staat = WIFI_ST_WACHTWOORD;
                kb_teken();
            }
        } else {
            int hint_y = WF_LIST_Y + WF_RIJEN_N * WF_RIJ_H;
            if (y >= hint_y) {
                if (x < TFT_W / 2) wf_scroll = max(0, wf_scroll - WF_RIJEN_N);
                else wf_scroll = min(max(0, wf_n_netwerken - WF_RIJEN_N), wf_scroll + WF_RIJEN_N);
                lijst_teken();
            } else if (y < WF_LIST_Y) {
                wf_staat = WIFI_ST_IDLE;
                scherm_bouwen = true;
            }
        }
    }
}

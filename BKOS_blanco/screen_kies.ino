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

const char* FW_VERSIE_URL[FW_COUNT] = {
    NUI_VERSIE_URL,
    "https://raw.githubusercontent.com/BrendanKoster86/BKOS4/main/versie.txt",
    "https://raw.githubusercontent.com/brennyc86/BKOS5a/main/versie.txt",
    BLANCO_VERSIE_URL
};

const char* FW_BIN_URL[FW_COUNT] = {
    NUI_BIN_URL,
    "https://raw.githubusercontent.com/BrendanKoster86/BKOS4/main/build/esp32.esp32.esp32s3/BKOS4.ino.bin",
    "https://raw.githubusercontent.com/brennyc86/BKOS5a/main/build/esp32.esp32.esp32s3/BKOS5a.ino.bin",
    BLANCO_BIN_URL
};

uint16_t FW_KLEUR[FW_COUNT] = {
    0x07FF,  // C_CYAN  — BKOS-NUI
    0xFD20,  // C_AMBER — BKOS4
    0x07E0,  // C_GREEN — BKOS5a
    0x4208   // C_DARK_GRAY — BKOS-blanco
};

bool FW_HEEFT_STABIEL[FW_COUNT] = { true, false, false, false };

int  kies_doel = -1;
char kies_versie_github[FW_COUNT][32];

KiesRelease kies_stabiel[KIES_STABIEL_MAX];
int         kies_stabiel_cnt     = 0;
bool        kies_stabiel_geladen = false;

String kies_flash_url;
String kies_flash_versie;

// ─── Versie-overzicht overlay (stabiel/beta) ─────────────────────────────────
static bool kies_ov_actief  = false;
static int  kies_ov_idx     = -1;
static bool kies_ov_beta    = false;
static int  kies_ov_pagina  = 0;

// ─── Layout per platform ────────────────────────────────────────────────────
// SMALL   (240×320): Pico / WROOM / CYD28
// CYD40H  (480×320): breed, hoogte-beperkt zoals SMALL
// CYD40V  (320×480): smallere breedte, hoogte als groot
// LARGE   (800×480): ESP32-S3

#if SCREEN_SMALL
  #define CARD_H       62
  #define CARD_GAP      4
  #define CARD_X        4
  #define CARD_W       (TFT_W - 8)
  #define BTN_INSTALL_W  58
  #define BTN_INSTALL_X  (CARD_X + CARD_W - BTN_INSTALL_W - 4)

#elif PLATFORM_CYD40H
  // 480×320: kompakte kaarten maar volle breedte
  #define CARD_H       62
  #define CARD_GAP      5
  #define CARD_X        8
  #define CARD_W       (TFT_W - 16)
  #define BTN_INSTALL_W  130
  #define BTN_INSTALL_X  (CARD_X + CARD_W - BTN_INSTALL_W - 8)
  #define BTN_Y_OFFSET   6
  #define BTN_HEIGHT     (CARD_H - 12)

#elif PLATFORM_CYD40V
  // 320×480: iets kleiner dan groot, kaarten verdelen de hoogte
  #define CARD_H       96
  #define CARD_GAP     16
  #define CARD_X        6
  #define CARD_W       (TFT_W - 12)
  #define BTN_INSTALL_W  90
  #define BTN_INSTALL_X  (CARD_X + CARD_W - BTN_INSTALL_W - 6)
  #define BTN_Y_OFFSET   (CARD_H - 42)
  #define BTN_HEIGHT     36

#else
  // 800×480: volledig groot
  #define CARD_H       96
  #define CARD_GAP      5
  #define CARD_X       10
  #define CARD_W       (TFT_W - 20)
  #define BTN_INSTALL_W  158
  #define BTN_INSTALL_X  (CARD_X + CARD_W - BTN_INSTALL_W - 8)
  #define BTN_Y_OFFSET   (CARD_H - 46)
  #define BTN_HEIGHT     38
#endif

#define CARD_Y(i) (HEADER_H + 2 + (i) * (CARD_H + CARD_GAP))

// ─── Layout versie-overzicht overlay ─────────────────────────────────────────
#if SCREEN_SMALL
  #define OV_ROW_H     34
  #define OV_ROW_GAP    4
  #define OV_TXT_SZ     1
  #define OV_HDR_BTN_W  66
#elif PLATFORM_CYD40H
  #define OV_ROW_H     34
  #define OV_ROW_GAP    4
  #define OV_TXT_SZ     1
  #define OV_HDR_BTN_W  110
#elif PLATFORM_CYD40V
  #define OV_ROW_H     44
  #define OV_ROW_GAP    6
  #define OV_TXT_SZ     2
  #define OV_HDR_BTN_W  110
#else
  #define OV_ROW_H     54
  #define OV_ROW_GAP    8
  #define OV_TXT_SZ     2
  #define OV_HDR_BTN_W  150
#endif

#define OV_ROW_X     8
#define OV_ROW_W     (TFT_W - 16)
#define OV_FOOT_H    46
#define OV_LIST_Y    (HEADER_H + 4)
#define OV_LIST_H    (TFT_H - OV_LIST_Y - OV_FOOT_H)
#define OV_ROWS_PP   (OV_LIST_H / (OV_ROW_H + OV_ROW_GAP))

// ─── Versie kleur/tekst helper ───────────────────────────────────────────────
static uint16_t vkleur(const char* v) {
    if (strlen(v) == 0)              return C_TEXT_DIM;
    if (strncmp(v, "fout", 4) == 0) return C_RED_BRIGHT;
    if (strncmp(v, "...",  3) == 0) return C_AMBER;
    return C_GREEN;
}

// ─── Header WiFi-status ──────────────────────────────────────────────────────
static void wifi_status_teken() {
    uint16_t dot_c = wifi_verbonden ? C_GREEN : C_RED_BRIGHT;

#if SCREEN_SMALL
    tft.fillCircle(TFT_W - 70, (HEADER_H - 10) / 2 + 5, 4, dot_c);
    ui_knop(TFT_W - 64, 6, 58, HEADER_H - 12, "WIFI", C_SURFACE2,
            wifi_verbonden ? C_GREEN : C_RED_BRIGHT);

#elif PLATFORM_CYD40H
    // 480px: VERSIES + WIFI, geen wifi-tekst (niet genoeg ruimte links van knoppen)
    tft.fillCircle(TFT_W - 136, (HEADER_H - 10) / 2 + 5, 4, dot_c);
    ui_knop(TFT_W - 232, 7, 100, 30, "VERSIES", C_SURFACE2, C_CYAN);
    ui_knop(TFT_W - 124, 7, 112, 30, "WIFI",    C_SURFACE2,
            wifi_verbonden ? C_GREEN : C_RED_BRIGHT);

#elif PLATFORM_CYD40V
    // 320px: compacte VERSIES + WIFI naast titel
    tft.fillCircle(TFT_W - 84, (HEADER_H - 10) / 2 + 5, 4, dot_c);
    ui_knop(136, 6, 80, HEADER_H - 12, "VERSIES", C_SURFACE2, C_CYAN);
    ui_knop(TFT_W - 78, 6, 72, HEADER_H - 12, "WIFI", C_SURFACE2,
            wifi_verbonden ? C_GREEN : C_RED_BRIGHT);

#else
    // 800px: wifi-tekst + VERSIES + WIFI
    int dx = TFT_W - 318, dy = (HEADER_H - 10) / 2;
    tft.fillCircle(dx, dy + 5, 5, dot_c);
    tft.setTextSize(1); tft.setTextColor(dot_c);
    tft.setCursor(dx + 10, dy + 1);
    tft.print(wifi_verbonden ? "Verbonden" : "Geen WiFi");
    ui_knop(TFT_W - 232, 7, 100, 30, "VERSIES", C_SURFACE2, C_CYAN);
    ui_knop(TFT_W - 124, 7, 112, 30, "WIFI",    C_SURFACE2,
            wifi_verbonden ? C_GREEN : C_RED_BRIGHT);
#endif
}

// ─── Kaart tekenen ───────────────────────────────────────────────────────────
static void kaart_teken(int idx) {
    int cy = CARD_Y(idx);
    uint16_t ac = FW_KLEUR[idx];
    const char* vstr = kies_versie_github[idx];

    tft.fillRoundRect(CARD_X, cy, CARD_W, CARD_H, 6, C_SURFACE);
    tft.drawRoundRect(CARD_X, cy, CARD_W, CARD_H, 6, C_SURFACE2);
    tft.fillRoundRect(CARD_X, cy, 5, CARD_H, 3, ac);

    uint16_t btn_t = (idx == FW_COUNT - 1) ? C_TEXT_DIM : ac;

#if SCREEN_SMALL
    // 240×320: naam, beschrijving (afgekapt), versie — INST rechts
    tft.setTextSize(1); tft.setTextColor(ac);
    tft.setCursor(CARD_X + 10, cy + 8); tft.print(FW_NAAM[idx]);

    tft.setTextColor(C_TEXT_DIM);
    tft.setCursor(CARD_X + 10, cy + 22);
    char desc[30]; strncpy(desc, FW_INFO[idx], 28); desc[28] = '\0'; tft.print(desc);

    tft.setCursor(CARD_X + 10, cy + 36); tft.print("v");
    tft.setTextColor(vkleur(vstr));
    char vs[24]; strncpy(vs, strlen(vstr) ? vstr : "onbekend", 23); vs[23] = '\0';
    tft.print(vs);

    tft.fillRoundRect(BTN_INSTALL_X, cy + 8, BTN_INSTALL_W, CARD_H - 16, 5, C_SURFACE2);
    tft.drawRoundRect(BTN_INSTALL_X, cy + 8, BTN_INSTALL_W, CARD_H - 16, 5, btn_t);
    tft.setTextColor(btn_t);
    tft.setCursor(BTN_INSTALL_X + (BTN_INSTALL_W - 24) / 2, cy + CARD_H / 2 - 4);
    tft.print("INST");

#elif PLATFORM_CYD40H
    // 480×320: naam + auteur, volledige beschrijving, versie — INSTALLEREN rechts
    tft.setTextSize(1); tft.setTextColor(ac);
    tft.setCursor(CARD_X + 10, cy + 8); tft.print(FW_NAAM[idx]);
    tft.setTextColor(C_TEXT_DIM); tft.print("  "); tft.print(FW_AUTEUR[idx]);

    tft.setTextColor(C_TEXT_DIM);
    tft.setCursor(CARD_X + 10, cy + 22); tft.print(FW_INFO[idx]);

    tft.setCursor(CARD_X + 10, cy + 36); tft.print("v");
    tft.setTextColor(vkleur(vstr));
    tft.print(strlen(vstr) ? vstr : "onbekend");

    tft.fillRoundRect(BTN_INSTALL_X, cy + BTN_Y_OFFSET, BTN_INSTALL_W, BTN_HEIGHT, 5, C_SURFACE2);
    tft.drawRoundRect(BTN_INSTALL_X, cy + BTN_Y_OFFSET, BTN_INSTALL_W, BTN_HEIGHT, 5, btn_t);
    tft.setTextColor(btn_t);
    { int tw = strlen("INSTALLEREN") * 6;
      tft.setCursor(BTN_INSTALL_X + (BTN_INSTALL_W - tw) / 2,
                    cy + BTN_Y_OFFSET + BTN_HEIGHT / 2 - 4);
      tft.print("INSTALLEREN"); }

#elif PLATFORM_CYD40V
    // 320×480: naam + auteur, beschrijving (afgekapt), versie, huidig — INSTALL rechts
    tft.setTextSize(1); tft.setTextColor(ac);
    tft.setCursor(CARD_X + 10, cy + 8); tft.print(FW_NAAM[idx]);
    tft.setTextColor(C_TEXT_DIM); tft.print("  "); tft.print(FW_AUTEUR[idx]);

    tft.setCursor(CARD_X + 10, cy + 22);
    char desc40v[34]; strncpy(desc40v, FW_INFO[idx], 33); desc40v[33] = '\0';
    tft.print(desc40v);

    tft.setCursor(CARD_X + 10, cy + 36); tft.print("v");
    tft.setTextColor(vkleur(vstr));
    tft.print(strlen(vstr) ? vstr : "onbekend");

    if (idx == FW_COUNT - 1) {
        tft.setTextColor(C_TEXT_DIM);
        tft.setCursor(CARD_X + 10, cy + 52);
        tft.print("Huidig: "); tft.setTextColor(C_TEXT); tft.print(BLANCO_VERSIE);
    }

    tft.fillRoundRect(BTN_INSTALL_X, cy + BTN_Y_OFFSET, BTN_INSTALL_W, BTN_HEIGHT, 5, C_SURFACE2);
    tft.drawRoundRect(BTN_INSTALL_X, cy + BTN_Y_OFFSET, BTN_INSTALL_W, BTN_HEIGHT, 5, btn_t);
    tft.setTextColor(btn_t);
    { int tw = strlen("INSTALL") * 6;
      tft.setCursor(BTN_INSTALL_X + (BTN_INSTALL_W - tw) / 2,
                    cy + BTN_Y_OFFSET + BTN_HEIGHT / 2 - 4);
      tft.print("INSTALL"); }

#else
    // 800×480: volledige kaart met auteur-badge
    tft.setTextSize(2); tft.setTextColor(ac);
    tft.setCursor(CARD_X + 16, cy + 10); tft.print(FW_NAAM[idx]);

    int nx = CARD_X + 16 + strlen(FW_NAAM[idx]) * 12 + 8;
    int bw = strlen(FW_AUTEUR[idx]) * 6 + 8;
    tft.fillRoundRect(nx, cy + 12, bw, 14, 4, C_SURFACE2);
    tft.setTextSize(1); tft.setTextColor(C_TEXT_DIM);
    tft.setCursor(nx + 4, cy + 15); tft.print(FW_AUTEUR[idx]);

    tft.setCursor(CARD_X + 16, cy + 34); tft.print(FW_INFO[idx]);

    tft.setCursor(CARD_X + 16, cy + 56); tft.print("GitHub: ");
    tft.setTextColor(vkleur(vstr));
    tft.print(strlen(vstr) ? vstr : "onbekend");

    if (idx == FW_COUNT - 1) {
        tft.setTextColor(C_TEXT_DIM);
        tft.setCursor(CARD_X + 16, cy + 70);
        tft.print("Huidig: "); tft.setTextColor(C_TEXT); tft.print(BLANCO_VERSIE);
    }

    tft.fillRoundRect(BTN_INSTALL_X, cy + BTN_Y_OFFSET, BTN_INSTALL_W, BTN_HEIGHT, 6, C_SURFACE2);
    tft.drawRoundRect(BTN_INSTALL_X, cy + BTN_Y_OFFSET, BTN_INSTALL_W, BTN_HEIGHT, 6, btn_t);
    tft.setTextColor(btn_t);
    { int tw = strlen("INSTALLEREN") * 6;
      tft.setCursor(BTN_INSTALL_X + (BTN_INSTALL_W - tw) / 2,
                    cy + BTN_Y_OFFSET + BTN_HEIGHT / 2 - 4);
      tft.print("INSTALLEREN"); }
#endif
}

// ─── Versies ophalen ─────────────────────────────────────────────────────────
void kies_versies_ophalen() {
    if (!wifi_verbonden) return;
    HTTPClient http;
    for (int i = 0; i < FW_COUNT; i++) {
        strcpy(kies_versie_github[i], "...");
        kaart_teken(i);
        http.begin(FW_VERSIE_URL[i]);
        int code = http.GET();
        if (code == HTTP_CODE_OK) {
            String v = http.getString(); v.trim();
            strncpy(kies_versie_github[i], v.c_str(), 31);
            kies_versie_github[i][31] = '\0';
        } else {
            snprintf(kies_versie_github[i], 32, "fout %d", code);
        }
        http.end();
        kaart_teken(i);
    }
}

// ─── Eén versiestring ophalen (zonder kaart-tekening, voor gebruik in de overlay) ─
static void _kies_versie_ophalen_1(int idx) {
    if (!wifi_verbonden) return;
    HTTPClient http;
    http.begin(FW_VERSIE_URL[idx]);
    int code = http.GET();
    if (code == HTTP_CODE_OK) {
        String v = http.getString(); v.trim();
        strncpy(kies_versie_github[idx], v.c_str(), 31);
        kies_versie_github[idx][31] = '\0';
    } else {
        snprintf(kies_versie_github[idx], 32, "fout %d", code);
    }
    http.end();
}

// ─── Stabiele releases van BKOS-NUI ophalen (releases.json) ─────────────────
#if PLATFORM_ESP32
#include <WiFiClientSecure.h>

static void _ov_json_veld(const String& json, int van, int tot,
                           const char* sleutel, char* uit, int max_len) {
    uit[0] = '\0';
    char zoek[24];
    snprintf(zoek, sizeof(zoek), "\"%s\":\"", sleutel);
    int k = json.indexOf(zoek, van);
    if (k < 0 || k >= tot) return;
    int vs = k + strlen(zoek);
    int ve = json.indexOf('"', vs);
    if (ve < 0 || ve > tot) return;
    int n = min(ve - vs, max_len - 1);
    for (int i = 0; i < n; i++) uit[i] = json[vs + i];
    uit[n] = '\0';
}

bool kies_stabiel_ophalen() {
    kies_stabiel_cnt = 0;
    kies_stabiel_geladen = false;
    if (!wifi_verbonden) return false;

    WiFiClientSecure sc;
    sc.setInsecure();
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(15000);
    http.begin(sc, NUI_RELEASES_URL);
    http.useHTTP10(true);
    int code = http.GET();
    if (code != HTTP_CODE_OK) { http.end(); kies_stabiel_geladen = true; return false; }
    String json = http.getString();
    http.end();

    int pos = 0;
    while (kies_stabiel_cnt < KIES_STABIEL_MAX) {
        int vi = json.indexOf("\"versie\":\"", pos);
        if (vi < 0) break;
        int start = json.lastIndexOf('{', vi);
        if (start < 0) { pos = vi + 10; continue; }
        int eind = json.indexOf('}', vi);
        if (eind < 0) break;
        KiesRelease& r = kies_stabiel[kies_stabiel_cnt];
        _ov_json_veld(json, start, eind, "versie", r.versie, sizeof(r.versie));
        _ov_json_veld(json, start, eind, "datum",  r.datum,  sizeof(r.datum));
        _ov_json_veld(json, start, eind, NUI_RELEASES_VELD, r.url, sizeof(r.url));
        if (strlen(r.versie) > 0 && strlen(r.url) > 0) kies_stabiel_cnt++;
        pos = eind + 1;
    }
    kies_stabiel_geladen = true;
    return kies_stabiel_cnt > 0;
}
#else
bool kies_stabiel_ophalen() {
    kies_stabiel_cnt = 0;
    kies_stabiel_geladen = true;
    return false;
}
#endif

// ─── Installatie starten (altijd via deze functie — zet de doel-URL/versie) ──
void kies_start_installatie(int idx, const char* url, const char* versie) {
    kies_doel          = idx;
    kies_flash_url      = url;
    kies_flash_versie  = versie;
    actief_scherm       = SCHERM_FLASH;
    scherm_bouwen       = true;
}

// ─── Versie-overzicht overlay: header/rijen/footer ───────────────────────────
static void _ov_header_teken() {
    tft.fillRect(0, 0, TFT_W, HEADER_H, C_SURFACE);
    tft.drawFastHLine(0, HEADER_H - 1, TFT_W, C_SURFACE2);
    int idx = kies_ov_idx;

    ui_knop(4, 6, 66, HEADER_H - 12, "TERUG", C_SURFACE2, C_TEXT);

    tft.setTextSize(OV_TXT_SZ); tft.setTextColor(FW_KLEUR[idx]);
    tft.setCursor(78, (HEADER_H - 8 * OV_TXT_SZ) / 2);
    tft.print(FW_NAAM[idx]);

    const char* mode_lbl = kies_ov_beta ? "STABIEL" : "BETA";
    uint16_t    mode_kl  = kies_ov_beta ? C_CYAN : C_AMBER;
    ui_knop(TFT_W - OV_HDR_BTN_W - 6, 6, OV_HDR_BTN_W, HEADER_H - 12,
            mode_lbl, C_SURFACE2, mode_kl);
}

static void _ov_rijen_teken() {
    tft.fillRect(0, OV_LIST_Y, TFT_W, OV_LIST_H, C_BG);
    int idx = kies_ov_idx;

    if (!kies_ov_beta) {
        if (kies_stabiel_cnt == 0) {
            tft.setTextSize(1); tft.setTextColor(C_TEXT_DIM);
            tft.setCursor(OV_ROW_X, OV_LIST_Y + 10);
            tft.print(kies_stabiel_geladen ? "Geen stabiele releases gevonden" : "Ophalen...");
            return;
        }
        int start = kies_ov_pagina * OV_ROWS_PP;
        int eind  = min(start + OV_ROWS_PP, kies_stabiel_cnt);
        for (int i = start; i < eind; i++) {
            int ry = OV_LIST_Y + (i - start) * (OV_ROW_H + OV_ROW_GAP);
            tft.fillRoundRect(OV_ROW_X, ry, OV_ROW_W, OV_ROW_H, 5, C_SURFACE);
            tft.drawRoundRect(OV_ROW_X, ry, OV_ROW_W, OV_ROW_H, 5, C_SURFACE2);
            int ty = ry + (OV_ROW_H - 8 * OV_TXT_SZ) / 2;
            tft.setTextSize(OV_TXT_SZ); tft.setTextColor(C_GREEN);
            tft.setCursor(OV_ROW_X + 10, ty);
            tft.print("v"); tft.print(kies_stabiel[i].versie);
            tft.setTextColor(C_TEXT_DIM);
            tft.setCursor(OV_ROW_X + OV_ROW_W - 90, ty);
            tft.print(kies_stabiel[i].datum);
        }
    } else {
        int ry = OV_LIST_Y;
        tft.fillRoundRect(OV_ROW_X, ry, OV_ROW_W, OV_ROW_H, 5, C_SURFACE);
        tft.drawRoundRect(OV_ROW_X, ry, OV_ROW_W, OV_ROW_H, 5, C_AMBER);
        int ty = ry + (OV_ROW_H - 8 * OV_TXT_SZ) / 2;
        tft.setTextSize(OV_TXT_SZ); tft.setTextColor(vkleur(kies_versie_github[idx]));
        tft.setCursor(OV_ROW_X + 10, ty);
        tft.print("v"); tft.print(strlen(kies_versie_github[idx]) ? kies_versie_github[idx] : "onbekend");
        tft.setTextColor(C_TEXT_DIM);
        tft.setCursor(OV_ROW_X + 10, ry + OV_ROW_H - 12);
        tft.print("Huidige main-branch build — wijzigt continu");
    }
}

static void _ov_footer_teken() {
    if (kies_ov_beta || kies_stabiel_cnt <= OV_ROWS_PP) return;
    int foot_y  = TFT_H - OV_FOOT_H + 4;
    int max_pag = (kies_stabiel_cnt - 1) / OV_ROWS_PP;
    tft.fillRect(0, foot_y - 4, TFT_W, OV_FOOT_H, C_BG);
    ui_knop(OV_ROW_X, foot_y, 90, OV_FOOT_H - 10, "VORIGE", C_SURFACE2,
            kies_ov_pagina > 0 ? C_TEXT : C_TEXT_DARK);
    ui_knop(TFT_W - OV_ROW_X - 90, foot_y, 90, OV_FOOT_H - 10, "VOLGENDE", C_SURFACE2,
            kies_ov_pagina < max_pag ? C_TEXT : C_TEXT_DARK);
}

static void _ov_teken() {
    _ov_header_teken();
    _ov_rijen_teken();
    _ov_footer_teken();
}

static void _ov_run(int x, int y, bool aanraking) {
    if (!aanraking) return;
    int idx = kies_ov_idx;

    if (y < HEADER_H) {
        if (x < 74) { kies_ov_actief = false; scherm_bouwen = true; return; }
        if (x >= TFT_W - OV_HDR_BTN_W - 6) {
            kies_ov_beta   = !kies_ov_beta;
            kies_ov_pagina = 0;
            if (kies_ov_beta && strlen(kies_versie_github[idx]) == 0) _kies_versie_ophalen_1(idx);
            scherm_bouwen = true;
        }
        return;
    }

    if (!kies_ov_beta && kies_stabiel_cnt > OV_ROWS_PP) {
        int foot_y = TFT_H - OV_FOOT_H + 4;
        if (y >= foot_y) {
            int max_pag = (kies_stabiel_cnt - 1) / OV_ROWS_PP;
            if (x < OV_ROW_X + 90 && kies_ov_pagina > 0) { kies_ov_pagina--; scherm_bouwen = true; }
            else if (x >= TFT_W - OV_ROW_X - 90 && kies_ov_pagina < max_pag) { kies_ov_pagina++; scherm_bouwen = true; }
            return;
        }
    }

    if (!kies_ov_beta) {
        if (kies_stabiel_cnt == 0 || !wifi_verbonden) return;
        int start = kies_ov_pagina * OV_ROWS_PP;
        int eind  = min(start + OV_ROWS_PP, kies_stabiel_cnt);
        for (int i = start; i < eind; i++) {
            int ry = OV_LIST_Y + (i - start) * (OV_ROW_H + OV_ROW_GAP);
            if (y >= ry && y < ry + OV_ROW_H) {
                kies_ov_actief = false;
                kies_start_installatie(idx, kies_stabiel[i].url, kies_stabiel[i].versie);
                return;
            }
        }
    } else {
        if (!wifi_verbonden) return;
        if (y >= OV_LIST_Y && y < OV_LIST_Y + OV_ROW_H) {
            kies_ov_actief = false;
            kies_start_installatie(idx, FW_BIN_URL[idx], kies_versie_github[idx]);
        }
    }
}

// ─── Scherm opbouwen ─────────────────────────────────────────────────────────
void screen_kies_teken() {
    if (kies_ov_actief) { _ov_teken(); return; }
    tft.fillScreen(C_BG);
    tft.fillRect(0, 0, TFT_W, HEADER_H, C_SURFACE);
    tft.drawFastHLine(0, HEADER_H - 1, TFT_W, C_SURFACE2);

#if SCREEN_SMALL || PLATFORM_CYD40V
    tft.setTextSize(1); tft.setTextColor(C_CYAN);
    tft.setCursor(8, (HEADER_H - 8) / 2);
    tft.print("BKOS FIRMWARE KIEZER");
#else
    // CYD40H en S3: tekstgrootte 2 past op breedte (240px < 464px of 800px)
    tft.setTextSize(2); tft.setTextColor(C_CYAN);
    tft.setCursor(12, (HEADER_H - 16) / 2);
    tft.print("BKOS KIEZER");
#endif

    wifi_status_teken();
    for (int i = 0; i < FW_COUNT; i++) kaart_teken(i);
}

// ─── Touch verwerken ─────────────────────────────────────────────────────────
void screen_kies_run(int x, int y, bool aanraking) {
    if (!aanraking) return;
    if (kies_ov_actief) { _ov_run(x, y, aanraking); return; }

    // Header touch
    if (y < HEADER_H) {
#if SCREEN_SMALL
        if (x >= TFT_W - 64) {
            actief_scherm = SCHERM_WIFI;
            scherm_bouwen = true;
        }
#elif PLATFORM_CYD40V
        if (x >= 136 && x < 216) {
            kies_versies_ophalen();
        } else if (x >= TFT_W - 78) {
            actief_scherm = SCHERM_WIFI;
            scherm_bouwen = true;
        }
#else
        // CYD40H en S3: VERSIES en WIFI op zelfde posities
        if (x >= TFT_W - 232 && x < TFT_W - 124) {
            kies_versies_ophalen();
        } else if (x >= TFT_W - 124) {
            actief_scherm = SCHERM_WIFI;
            scherm_bouwen = true;
        }
#endif
        return;
    }

    // Kaart touch
    for (int i = 0; i < FW_COUNT; i++) {
        int cy = CARD_Y(i);
        if (y < cy || y >= cy + CARD_H) continue;

#if SCREEN_SMALL
        bool op_knop = (x >= BTN_INSTALL_X);
#else
        int btn_y = cy + BTN_Y_OFFSET;
        bool op_knop = (x >= BTN_INSTALL_X && x < BTN_INSTALL_X + BTN_INSTALL_W &&
                        y >= btn_y && y < btn_y + BTN_HEIGHT);
#endif

        if (op_knop) {
            if (!wifi_verbonden) {
                tft.fillRoundRect(CARD_X + 4, cy + 4, CARD_W - 8, CARD_H - 8, 6, C_SURFACE);
                tft.setTextSize(1); tft.setTextColor(C_RED_BRIGHT);
                tft.setCursor(CARD_X + 10, cy + CARD_H / 2 - 4);
                tft.print("Geen WiFi");
                delay(1500);
                kaart_teken(i);
                return;
            }
            // INSTALLEREN: bij een stabiel/beta-onderscheid altijd de nieuwste
            // STABIELE versie (ophalen indien nog niet gecached); anders het
            // bestaande gedrag (huidige main-branch bin).
            if (FW_HEEFT_STABIEL[i]) {
                if (!kies_stabiel_geladen) {
                    tft.fillRoundRect(CARD_X + 4, cy + 4, CARD_W - 8, CARD_H - 8, 6, C_SURFACE);
                    tft.setTextSize(1); tft.setTextColor(C_AMBER);
                    tft.setCursor(CARD_X + 10, cy + CARD_H / 2 - 4);
                    tft.print("Stabiele versie ophalen...");
                    kies_stabiel_ophalen();
                }
                if (kies_stabiel_cnt > 0) {
                    kies_start_installatie(i, kies_stabiel[0].url, kies_stabiel[0].versie);
                } else {
                    kaart_teken(i);
                    kies_start_installatie(i, FW_BIN_URL[i], kies_versie_github[i]);
                }
            } else {
                kies_start_installatie(i, FW_BIN_URL[i], kies_versie_github[i]);
            }
        } else if (FW_HEEFT_STABIEL[i]) {
            // Tik op het kaart-lichaam (niet de knop): open het versie-overzicht.
            kies_ov_actief  = true;
            kies_ov_idx     = i;
            kies_ov_beta    = false;
            kies_ov_pagina  = 0;
            if (!kies_stabiel_geladen) kies_stabiel_ophalen();
            scherm_bouwen = true;
        }
#if SCREEN_SMALL
        // Op small scherm, kaarten zonder stabiel-kanaal: tik naast knop = versies ophalen
        else {
            kies_versies_ophalen();
        }
#endif
        return;
    }
}

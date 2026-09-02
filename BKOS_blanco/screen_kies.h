#pragma once
#include "ui_draw.h"
#include "app_state.h"
#include "ota.h"

// ─── Firmware tabel ─────────────────────────────────────────────────────────
// Pas de URLs aan als de repo-structuur van BKOS4 of BKOS5a afwijkt.
#define FW_COUNT 4

extern const char* FW_NAAM[FW_COUNT];
extern const char* FW_AUTEUR[FW_COUNT];
extern const char* FW_INFO[FW_COUNT];
extern const char* FW_VERSIE_URL[FW_COUNT];
extern const char* FW_BIN_URL[FW_COUNT];
extern uint16_t    FW_KLEUR[FW_COUNT];

extern int   kies_doel;    // geselecteerde firmware-index voor SCHERM_FLASH
extern char  kies_versie_github[FW_COUNT][32];

// Welke firmware-kaarten hebben een stabiel/beta-onderscheid (releases.json)?
// Voorlopig alleen BKOS-NUI (index 0) — BKOS4/BKOS5a/BKOS-blanco hebben maar
// één versie op de main-branch, geen stabiele releases-index.
extern bool FW_HEEFT_STABIEL[FW_COUNT];

// ─── Stabiele releases van BKOS-NUI (releases.json) ──────────────────────────
#define KIES_STABIEL_MAX 10
struct KiesRelease {
    char versie[16];
    char datum[12];
    char url[160];
};
extern KiesRelease kies_stabiel[KIES_STABIEL_MAX];
extern int         kies_stabiel_cnt;
extern bool        kies_stabiel_geladen;

// Doel-URL/versie voor SCHERM_FLASH — altijd gezet vóór het omschakelen naar
// SCHERM_FLASH, zodat dat scherm nooit zelf hoeft te kiezen welke bron.
extern String kies_flash_url;
extern String kies_flash_versie;

void screen_kies_teken();
void screen_kies_run(int x, int y, bool aanraking);
void kies_versies_ophalen();
void kies_start_installatie(int idx, const char* url, const char* versie);
bool kies_stabiel_ophalen();

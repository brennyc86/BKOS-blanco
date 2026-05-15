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

void screen_kies_teken();
void screen_kies_run(int x, int y, bool aanraking);
void kies_versies_ophalen();

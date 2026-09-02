#pragma once
#include "platform.h"

#define BLANCO_VERSIE "0.0.260902.5"

// ─── BKOS-NUI firmware URLs (platform-afhankelijk) ───────────────────────────
#if PLATFORM_WROOM
  #define NUI_VERSIE_URL "https://raw.githubusercontent.com/brennyc86/BKOS-NUI/main/firmware/versie_wroom.txt"
  #define NUI_BIN_URL    "https://raw.githubusercontent.com/brennyc86/BKOS-NUI/main/firmware/bkos_esp32wroom2432.bin"
#elif PLATFORM_CYD28
  #define NUI_VERSIE_URL "https://raw.githubusercontent.com/brennyc86/BKOS-NUI/main/firmware/versie_cyd28.txt"
  #define NUI_BIN_URL    "https://raw.githubusercontent.com/brennyc86/BKOS-NUI/main/firmware/bkos_esp32cyd28.bin"
#elif PLATFORM_CYD40H
  #define NUI_VERSIE_URL "https://raw.githubusercontent.com/brennyc86/BKOS-NUI/main/firmware/versie_cyd40h.txt"
  #define NUI_BIN_URL    "https://raw.githubusercontent.com/brennyc86/BKOS-NUI/main/firmware/bkos_esp32cyd40h.bin"
#elif PLATFORM_CYD40V
  #define NUI_VERSIE_URL "https://raw.githubusercontent.com/brennyc86/BKOS-NUI/main/firmware/versie_cyd40v.txt"
  #define NUI_BIN_URL    "https://raw.githubusercontent.com/brennyc86/BKOS-NUI/main/firmware/bkos_esp32cyd40v.bin"
#elif PLATFORM_PICO
  #define NUI_VERSIE_URL "https://raw.githubusercontent.com/brennyc86/BKOS-NUI/main/firmware/versie_pico.txt"
  #define NUI_BIN_URL    "https://raw.githubusercontent.com/brennyc86/BKOS-NUI/main/firmware/bkos_pico1w2432.uf2"
#else
  // ESP32-S3
  #define NUI_VERSIE_URL "https://raw.githubusercontent.com/brennyc86/BKOS-NUI/main/firmware/versie_esp32s3.txt"
  #define NUI_BIN_URL    "https://raw.githubusercontent.com/brennyc86/BKOS-NUI/main/firmware/bkos_esp32s3_8048s070.bin"
#endif

// ─── BKOS-NUI stabiele releases-index (voor het versie-overzicht in de kiezer) ─
#define NUI_RELEASES_URL "https://raw.githubusercontent.com/brennyc86/BKOS-NUI/main/firmware/releases.json"
#if PLATFORM_WROOM
  #define NUI_RELEASES_VELD "url_wroom"
#elif PLATFORM_CYD28
  #define NUI_RELEASES_VELD "url_cyd28"
#elif PLATFORM_CYD40H
  #define NUI_RELEASES_VELD "url_cyd40h"
#elif PLATFORM_CYD40V
  #define NUI_RELEASES_VELD "url_cyd40v"
#elif PLATFORM_PICO
  #define NUI_RELEASES_VELD "url_pico"
#else
  #define NUI_RELEASES_VELD "url_s3"
#endif

// ─── BKOS-blanco self-update URLs (platform-afhankelijk) ─────────────────────
#if PLATFORM_WROOM
  #define BLANCO_VERSIE_URL "https://raw.githubusercontent.com/brennyc86/BKOS-blanco/main/firmware/versie_wroom.txt"
  #define BLANCO_BIN_URL    "https://raw.githubusercontent.com/brennyc86/BKOS-blanco/main/firmware/bkos_blanco_wroom.bin"
#elif PLATFORM_CYD28
  #define BLANCO_VERSIE_URL "https://raw.githubusercontent.com/brennyc86/BKOS-blanco/main/firmware/versie_cyd28.txt"
  #define BLANCO_BIN_URL    "https://raw.githubusercontent.com/brennyc86/BKOS-blanco/main/firmware/bkos_blanco_cyd28.bin"
#elif PLATFORM_CYD40H
  #define BLANCO_VERSIE_URL "https://raw.githubusercontent.com/brennyc86/BKOS-blanco/main/firmware/versie_cyd40h.txt"
  #define BLANCO_BIN_URL    "https://raw.githubusercontent.com/brennyc86/BKOS-blanco/main/firmware/bkos_blanco_cyd40h.bin"
#elif PLATFORM_CYD40V
  #define BLANCO_VERSIE_URL "https://raw.githubusercontent.com/brennyc86/BKOS-blanco/main/firmware/versie_cyd40v.txt"
  #define BLANCO_BIN_URL    "https://raw.githubusercontent.com/brennyc86/BKOS-blanco/main/firmware/bkos_blanco_cyd40v.bin"
#elif PLATFORM_PICO
  #define BLANCO_VERSIE_URL "https://raw.githubusercontent.com/brennyc86/BKOS-blanco/main/firmware/versie_pico.txt"
  #define BLANCO_BIN_URL    "https://raw.githubusercontent.com/brennyc86/BKOS-blanco/main/firmware/bkos_blanco_pico.uf2"
#else
  // ESP32-S3
  #define BLANCO_VERSIE_URL "https://raw.githubusercontent.com/brennyc86/BKOS-blanco/main/firmware/versie_esp32s3.txt"
  #define BLANCO_BIN_URL    "https://raw.githubusercontent.com/brennyc86/BKOS-blanco/main/firmware/bkos_blanco_esp32s3.bin"
#endif

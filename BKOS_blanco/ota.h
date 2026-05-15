#pragma once
#include "platform.h"

#define BLANCO_VERSIE "0.0.260515.1"

// ─── BKOS-NUI firmware URLs (platform-afhankelijk) ───────────────────────────
#if PLATFORM_WROOM
  #define NUI_VERSIE_URL "https://raw.githubusercontent.com/brennyc86/BKOS-NUI/main/firmware/versie_wroom.txt"
  #define NUI_BIN_URL    "https://raw.githubusercontent.com/brennyc86/BKOS-NUI/main/firmware/bkos_esp32wroom2432.bin"
#else
  #define NUI_VERSIE_URL "https://raw.githubusercontent.com/brennyc86/BKOS-NUI/main/firmware/versie_esp32s3.txt"
  #define NUI_BIN_URL    "https://raw.githubusercontent.com/brennyc86/BKOS-NUI/main/firmware/bkos_esp32s3_8048s070.bin"
#endif

// ─── BKOS-blanco self-update URLs (platform-afhankelijk) ─────────────────────
#if PLATFORM_WROOM
  #define BLANCO_VERSIE_URL "https://raw.githubusercontent.com/brennyc86/BKOS-blanco/main/firmware/versie_wroom.txt"
  #define BLANCO_BIN_URL    "https://raw.githubusercontent.com/brennyc86/BKOS-blanco/main/firmware/bkos_blanco_wroom.bin"
#else
  #define BLANCO_VERSIE_URL "https://raw.githubusercontent.com/brennyc86/BKOS-blanco/main/firmware/versie_esp32s3.txt"
  #define BLANCO_BIN_URL    "https://raw.githubusercontent.com/brennyc86/BKOS-blanco/main/firmware/bkos_blanco_esp32s3.bin"
#endif

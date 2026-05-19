#pragma once
// ─── Platform-detectie ────────────────────────────────────────────────────────
// Compiler flags: -DPLATFORM_WROOM=1 / -DPLATFORM_CYD28=1 /
//                 -DPLATFORM_CYD40H=1 / -DPLATFORM_CYD40V=1
// Pico wordt auto-gedetecteerd via ARDUINO_ARCH_RP2040.

#ifndef PLATFORM_WROOM
  #define PLATFORM_WROOM  0
#endif
#ifndef PLATFORM_CYD28
  #define PLATFORM_CYD28  0
#endif
#ifndef PLATFORM_CYD40H
  #define PLATFORM_CYD40H 0
#endif
#ifndef PLATFORM_CYD40V
  #define PLATFORM_CYD40V 0
#endif

#if defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_RP2350)
  #define PLATFORM_PICO   1
  #define PLATFORM_ESP32  0
  #undef  PLATFORM_WROOM
  #define PLATFORM_WROOM  0
  #undef  PLATFORM_CYD28
  #define PLATFORM_CYD28  0
  #undef  PLATFORM_CYD40H
  #define PLATFORM_CYD40H 0
  #undef  PLATFORM_CYD40V
  #define PLATFORM_CYD40V 0
#else
  #define PLATFORM_PICO   0
  #define PLATFORM_ESP32  1
#endif

#define PLATFORM_CYD     (PLATFORM_CYD28 || PLATFORM_CYD40H || PLATFORM_CYD40V)
#define PLATFORM_XPT2046 (PLATFORM_PICO  || PLATFORM_WROOM  || PLATFORM_CYD)
#define SCREEN_SMALL     (PLATFORM_PICO  || PLATFORM_WROOM  || PLATFORM_CYD28)

// ─── Scherm en pinnen per platform ───────────────────────────────────────────
#if PLATFORM_PICO
  #define TFT_W     240
  #define TFT_H     320
  #define TFT_BL    10
  #define TFT_CS    17
  #define TFT_DC    15
  #define TFT_RST   14
  #define TFT_SCK   18
  #define TFT_MOSI  19
  #define TFT_MISO  16
  #define PICO_TS_CS   13
  #define PICO_TS_IRQ  11

#elif PLATFORM_WROOM
  #define TFT_W     240
  #define TFT_H     320
  #define TFT_BL    19
  #define TFT_CS    15   // GPIO15 (zie BKOS3 hardware.h: cs_tft=15)
  #define TFT_DC    23   // GPIO23 (zie BKOS3/4: dc=23)
  #define TFT_RST   16   // GPIO16 (zie BKOS3 hardware.h: rst=16)
  #define TFT_SCK   14
  #define TFT_MOSI  13
  #define TFT_MISO  12
  #define WROOM_TS_CS   22   // GPIO22 (zie BKOS3: cs_ts=22)
  #define WROOM_TS_IRQ  21   // GPIO21 (zie BKOS3: irq=21)

#elif PLATFORM_CYD28
  #define TFT_W     240
  #define TFT_H     320
  #define TFT_BL    21
  #define TFT_CS    15
  #define TFT_DC     2
  #define TFT_RST   -1
  #define TFT_SCK   14
  #define TFT_MOSI  13
  #define TFT_MISO  12
  #define CYD28_TS_SCK   25
  #define CYD28_TS_MOSI  32
  #define CYD28_TS_MISO  39
  #define CYD28_TS_IRQ   36
  #define CYD28_TS_CS    33

#elif PLATFORM_CYD40H || PLATFORM_CYD40V
  #if PLATFORM_CYD40H
    #define TFT_W   480
    #define TFT_H   320
  #else
    #define TFT_W   320
    #define TFT_H   480
  #endif
  #define TFT_BL    27
  #define TFT_CS    15
  #define TFT_DC     2
  #define TFT_RST   -1
  #define TFT_SCK   14
  #define TFT_MOSI  13
  #define TFT_MISO  12
  // Touch deelt HSPI met display (SCK=14, MOSI=13, MISO=12), eigen CS=33
  #define CYD40_TS_SCK   14
  #define CYD40_TS_MOSI  13
  #define CYD40_TS_MISO  12
  #define CYD40_TS_CS    33
  #define CYD40_TS_IRQ   36

#else
  // ESP32-S3 800×480 RGB
  #define TFT_W    800
  #define TFT_H    480
  #define TFT_BL     2
#endif

// ─── Reboot ───────────────────────────────────────────────────────────────────
#if PLATFORM_PICO
  #define PLATFORM_REBOOT() rp2040.reboot()
#else
  #define PLATFORM_REBOOT() ESP.restart()
#endif

// ─── Vrij geheugen ────────────────────────────────────────────────────────────
#if PLATFORM_PICO
  #define PLATFORM_FREE_HEAP() ((unsigned)rp2040.getFreeHeap())
#else
  #define PLATFORM_FREE_HEAP() ((unsigned)ESP.getFreeHeap())
#endif

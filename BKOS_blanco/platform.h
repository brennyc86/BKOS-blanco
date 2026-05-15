#pragma once
// Platform-detectie: Pico via build-omgeving, WROOM via -DPLATFORM_WROOM=1 compiler flag.
// Standaard (geen flag) = ESP32-S3.

#if defined(ARDUINO_RASPBERRY_PI_PICO_W) || defined(ARDUINO_ARCH_RP2040)
  #define PLATFORM_PICO  1
  #define PLATFORM_WROOM 0
#else
  #define PLATFORM_PICO  0
  #ifndef PLATFORM_WROOM
    #define PLATFORM_WROOM 0
  #endif
#endif

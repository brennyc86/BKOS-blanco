#include "hw_scherm.h"
#include "hw_touch.h"
// Zelfde bestandssysteem-abstractie als BKOS-NUI's platform_fs.h: SPIFFS op
// ESP32, LittleFS op Pico (RP2040 LittleFS::begin() heeft geen format-argument).
#if PLATFORM_PICO
  #include <LittleFS.h>
  #define BLANCO_FS LittleFS
  #define BLANCO_FS_BEGIN() (BLANCO_FS.begin())
#else
  #include <SPIFFS.h>
  #define BLANCO_FS SPIFFS
  #define BLANCO_FS_BEGIN() (BLANCO_FS.begin(true))
#endif

Arduino_GFX *tft_p = nullptr;

int           tft_helderheid    = 75;
long          scherm_timer      = 60;
bool          tft_actief        = true;
long          scherm_touched    = 0;
bool          scherm_net_gewekt = false;
bool          tft_bijna_uit     = false;
unsigned long tft_dim_ms        = 0;
bool          tft_gedraaid      = false;

// Leest "draai=1" uit een eventueel achtergebleven BKOS-NUI config-bestand
// (/bkos_config.csv, zelfde partitie/formaat als BKOS-NUI's app_state.ino
// schrijft). Ontbreekt het bestand (nooit BKOS-NUI geïnstalleerd geweest,
// of een compleet lege flash) dan blijft het gewoon rechtop — geen fout.
static bool _gedraaid_uit_nui_config() {
    if (!BLANCO_FS_BEGIN()) return false;
    File f = BLANCO_FS.open("/bkos_config.csv", "r");
    if (!f) return false;
    bool gedraaid = false;
    while (f.available()) {
        String lijn = f.readStringUntil('\n');
        lijn.trim();
        if (lijn.startsWith("draai=")) {
            gedraaid = (lijn.substring(6).toInt() != 0);
            break;
        }
    }
    f.close();
    return gedraaid;
}

void tft_setup() {
    tft_gedraaid = _gedraaid_uit_nui_config();
#if PLATFORM_ESP32 && !PLATFORM_WROOM && !PLATFORM_CYD
    // ── ESP32-S3: 800×480 RGB panel ─────────────────────────────────────
    Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
        41, 40, 39, 42,
        14, 21, 47, 48, 45,
         9, 46,  3,  8, 16,  1,
        15,  7,  6,  5,  4,
        0, 210, 30, 16,
        0,  22, 13, 10,
        1, 16000000);
    tft_p = new Arduino_RGB_Display(800, 480, rgbpanel, 0, true);

#elif PLATFORM_WROOM
    // ── ILI9341 240×320 via shared HSPI (display + touch delen bus, eigen CS)
    Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO, &shared_hspi);
    tft_p = new Arduino_ILI9341(bus, TFT_RST, 0, false);

#elif PLATFORM_CYD28
    // ── ILI9341 240×320 via HSPI (touch heeft aparte VSPI)
    Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO, HSPI);
    tft_p = new Arduino_ILI9341(bus, TFT_RST, 0, false);

#elif PLATFORM_CYD40H || PLATFORM_CYD40V
    // ── ST7796 480×320 / 320×480 via HSPI ────────────────────────────────
    Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO, HSPI);
    tft_p = new Arduino_ST7796(bus, TFT_RST, 0, false);

#elif PLATFORM_PICO
    // ── Pico W: ILI9341 via SPI0 ─────────────────────────────────────────
    SPI.setRX(TFT_MISO);
    SPI.setTX(TFT_MOSI);
    SPI.setSCK(TFT_SCK);
    SPI.begin();
    Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS);
    tft_p = new Arduino_ILI9341(bus, TFT_RST, 0, false);
#endif

    tft.begin();
#if PLATFORM_CYD40H
    tft.setRotation(tft_gedraaid ? 3 : 1);   // 480×320 landscape, +180° indien gedraaid
#else
    tft.setRotation(tft_gedraaid ? 2 : 0);
#endif
    pinMode(TFT_BL, OUTPUT);
    tft_helderheid_zet(tft_helderheid);
}

void tft_helderheid_zet(int pct) {
    analogWrite(TFT_BL, map(constrain(pct, 0, 100), 0, 100, 0, 255));
}

void tft_loop() {
    if (tft_actief) {
        if (!actieve_touch && scherm_timer > 0 &&
            millis() > (unsigned long)(scherm_touched + (long)scherm_timer * 1000)) {
            tft_actief    = false;
            tft_bijna_uit = true;
            tft_dim_ms    = millis();
            tft_helderheid_zet(TFT_MIN_HELDER);
        }
    } else if (tft_bijna_uit) {
        if (actieve_touch) {
            tft_bijna_uit     = false;
            tft_actief        = true;
            scherm_net_gewekt = true;
            tft_helderheid_zet(tft_helderheid);
        } else if (millis() - tft_dim_ms > 5000UL) {
            tft_bijna_uit = false;
            tft_helderheid_zet(0);
        }
    } else {
        if (actieve_touch) {
            tft_actief        = true;
            scherm_net_gewekt = true;
            tft_helderheid_zet(tft_helderheid);
        }
    }
}

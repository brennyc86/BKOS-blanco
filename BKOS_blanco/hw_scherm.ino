#include "hw_scherm.h"
#include "hw_touch.h"

Arduino_GFX *tft_p = nullptr;

int           tft_helderheid    = 75;
long          scherm_timer      = 60;
bool          tft_actief        = true;
long          scherm_touched    = 0;
bool          scherm_net_gewekt = false;
bool          tft_bijna_uit     = false;
unsigned long tft_dim_ms        = 0;

void tft_setup() {
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
    tft.setRotation(1);   // 480×320 landscape
#elif PLATFORM_CYD28
    tft.setRotation(2);   // portret 180° (connector-oriëntatie CYD28 board)
#else
    tft.setRotation(0);
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

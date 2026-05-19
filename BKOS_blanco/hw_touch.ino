#include "hw_touch.h"
#include "hw_scherm.h"

bool actieve_touch = false;
int  ts_x = 0;
int  ts_y = 0;

// ─── Touch-objecten ───────────────────────────────────────────────────────────
#if PLATFORM_ESP32 && !PLATFORM_WROOM && !PLATFORM_CYD
    static TAMC_GT911 ts(TS_SDA, TS_SCK, -1, TS_RST, TFT_W, TFT_H);

#elif PLATFORM_PICO
    static XPT2046_Touchscreen ts(PICO_TS_CS, PICO_TS_IRQ);

#elif PLATFORM_WROOM
    static XPT2046_Touchscreen ts(WROOM_TS_CS, WROOM_TS_IRQ);

#elif PLATFORM_CYD28
    static XPT2046_Touchscreen ts(CYD28_TS_CS);   // geen IRQ op CYD28

#elif PLATFORM_CYD40H || PLATFORM_CYD40V
    static SPIClass              cyd_vspi(VSPI);
    static XPT2046_Touchscreen   ts(CYD40_TS_CS);  // aparte VSPI, geen IRQ
#endif

// ─── Kalibratie (XPT2046) ─────────────────────────────────────────────────────
#if PLATFORM_XPT2046
static int ts_cal_py_min = 200;
static int ts_cal_py_max = 3700;
static int ts_cal_px_hi  = 3700;
static int ts_cal_px_lo  = 200;
#endif

// ─── XPT2046 assen berekening ─────────────────────────────────────────────────
// De touch-paneel-assen zijn 90° gedraaid t.o.v. het display:
//   raw X → scherm Y (omgekeerd)
//   raw Y → scherm X
#if PLATFORM_XPT2046
static void ts_lees_xy() {
    TS_Point p = ts.getPoint();
    ts_x = map(p.y, ts_cal_py_min, ts_cal_py_max, 0, TFT_W);
    ts_y = map(p.x, ts_cal_px_hi,  ts_cal_px_lo,  0, TFT_H);
}
#endif

void ts_setup() {
#if PLATFORM_ESP32 && !PLATFORM_WROOM && !PLATFORM_CYD
    ts.begin();
    ts.setRotation(0);

#elif PLATFORM_PICO
    ts.begin(SPI);
    ts.setRotation(0);

#elif PLATFORM_WROOM || PLATFORM_CYD28
    // SPI geïnitialiseerd door tft_setup() met HSPI-pinnen; touch deelt deze bus
    ts.begin(SPI);
    ts.setRotation(0);

#elif PLATFORM_CYD40H || PLATFORM_CYD40V
    // Display gebruikt HSPI; touch heeft eigen VSPI (GPIO 25/32/39/33)
    cyd_vspi.begin(CYD40_TS_SCK, CYD40_TS_MISO, CYD40_TS_MOSI, CYD40_TS_CS);
    ts.begin(cyd_vspi);
    ts.setRotation(0);
#endif
}

bool ts_touched() {
#if PLATFORM_ESP32 && !PLATFORM_WROOM && !PLATFORM_CYD
    ts.read();
    if (ts.isTouched) {
        ts_x = map(ts.points[0].y, 5, 800, 0, TFT_W);
        ts_y = map(ts.points[0].x, 490, 5, 0, TFT_H);
        scherm_touched = millis();
        actieve_touch  = true;
        return true;
    }
    actieve_touch = false;
    return false;

#elif PLATFORM_PICO || PLATFORM_WROOM
    if (ts.tirqTouched() && ts.touched()) {
        ts_lees_xy();
        scherm_touched = millis();
        actieve_touch  = true;
        return true;
    }
    actieve_touch = false;
    return false;

#elif PLATFORM_CYD28 || PLATFORM_CYD40H || PLATFORM_CYD40V
    if (ts.touched()) {
        ts_lees_xy();
        scherm_touched = millis();
        actieve_touch  = true;
        return true;
    }
    actieve_touch = false;
    return false;

#else
    actieve_touch = false;
    return false;
#endif
}

int touch_x() { return ts_x; }
int touch_y() { return ts_y; }

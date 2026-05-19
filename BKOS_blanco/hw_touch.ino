#include "hw_touch.h"
#include "hw_scherm.h"

bool actieve_touch = false;
int  ts_x = 0;
int  ts_y = 0;

// ─── Touch-objecten ───────────────────────────────────────────────────────────
#if PLATFORM_ESP32 && !PLATFORM_WROOM && !PLATFORM_CYD
    // ESP32-S3: GT911 capacitief
    static TAMC_GT911 ts(TS_SDA, TS_SCK, -1, TS_RST, TFT_W, TFT_H);

#elif PLATFORM_PICO || PLATFORM_WROOM || PLATFORM_CYD28
    // Pico / WROOM / CYD28: XPT2046 op hoofd SPI-bus
    #if PLATFORM_PICO
      static XPT2046_Touchscreen ts(PICO_TS_CS, PICO_TS_IRQ);
    #elif PLATFORM_WROOM
      static XPT2046_Touchscreen ts(WROOM_TS_CS, WROOM_TS_IRQ);
    #else
      static XPT2046_Touchscreen ts(CYD28_TS_CS);
    #endif

#elif PLATFORM_CYD40H || PLATFORM_CYD40V
    // CYD40: XPT2046 op aparte VSPI
    static SPIClass           cyd_vspi(VSPI);
    static XPT2046_Touchscreen ts(CYD40_TS_CS);
#endif

// ─── Kalibratie (XPT2046) ─────────────────────────────────────────────────────
#if PLATFORM_XPT2046
static int ts_cal_py_min = 200;
static int ts_cal_py_max = 3700;
static int ts_cal_px_hi  = 3700;
static int ts_cal_px_lo  = 200;
#endif

void ts_setup() {
#if PLATFORM_ESP32 && !PLATFORM_WROOM && !PLATFORM_CYD
    ts.begin();
    ts.setRotation(0);

#elif PLATFORM_PICO || PLATFORM_WROOM || PLATFORM_CYD28
    ts.begin(SPI);
    ts.setRotation(0);

#elif PLATFORM_CYD40H || PLATFORM_CYD40V
    cyd_vspi.begin(CYD40_TS_SCK, CYD40_TS_MISO, CYD40_TS_MOSI);
    ts.begin(cyd_vspi);
    ts.setRotation(0);
#endif
}

bool ts_touched() {
#if PLATFORM_ESP32 && !PLATFORM_WROOM && !PLATFORM_CYD
    ts.read();
    if (ts.isTouched) {
        scherm_touched = millis();
        actieve_touch  = true;
        ts_x = touch_x();
        ts_y = touch_y();
        return true;
    }
    actieve_touch = false;
    return false;

#elif PLATFORM_PICO || PLATFORM_WROOM
    if (ts.tirqTouched() && ts.touched()) {
        scherm_touched = millis();
        actieve_touch  = true;
        ts_x = touch_x();
        ts_y = touch_y();
        return true;
    }
    actieve_touch = false;
    return false;

#elif PLATFORM_CYD28 || PLATFORM_CYD40H || PLATFORM_CYD40V
    if (ts.touched()) {
        scherm_touched = millis();
        actieve_touch  = true;
        ts_x = touch_x();
        ts_y = touch_y();
        return true;
    }
    actieve_touch = false;
    return false;
#else
    return false;
#endif
}

int touch_x() {
#if PLATFORM_ESP32 && !PLATFORM_WROOM && !PLATFORM_CYD
    return map(ts.points[0].y, 5, TFT_W, 0, TFT_W);
#elif PLATFORM_XPT2046
    TS_Point p = ts.getPoint();
    return map(p.x, ts_cal_px_lo, ts_cal_px_hi, 0, TFT_W);
#else
    return 0;
#endif
}

int touch_y() {
#if PLATFORM_ESP32 && !PLATFORM_WROOM && !PLATFORM_CYD
    return map(ts.points[0].x, TFT_H, 5, 0, TFT_H);
#elif PLATFORM_XPT2046
    TS_Point p = ts.getPoint();
    return map(p.y, ts_cal_py_min, ts_cal_py_max, 0, TFT_H);
#else
    return 0;
#endif
}

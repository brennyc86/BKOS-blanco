#include "ui_draw.h"

void ui_knop(int x, int y, int w, int h, const char* tekst, uint16_t bg, uint16_t fg, bool actief) {
    tft.fillRoundRect(x, y, w, h, KNOP_R, bg);
    if (actief) {
        tft.drawRoundRect(x,   y,   w,   h,   KNOP_R,   fg);
        tft.drawRoundRect(x+1, y+1, w-2, h-2, KNOP_R-1, fg);
    }
    tft.setTextSize(2);
    tft.setTextColor(fg);
    int tw = strlen(tekst) * 12;
    int th = 16;
    tft.setCursor(x + (w - tw) / 2, y + (h - th) / 2);
    tft.print(tekst);
}

void ui_tekst_midden(int x, int y, int w, const char* tekst, uint16_t kleur, uint8_t grootte) {
    tft.setTextSize(grootte);
    tft.setTextColor(kleur);
    int tw = strlen(tekst) * 6 * grootte;
    tft.setCursor(x + (w - tw) / 2, y);
    tft.print(tekst);
}

void ui_header(const char* titel) {
    tft.fillRect(0, 0, TFT_W, HEADER_H, C_SURFACE);
    tft.drawFastHLine(0, HEADER_H - 1, TFT_W, C_SURFACE2);
    tft.setTextSize(2);
    tft.setTextColor(C_CYAN);
    tft.setCursor(12, (HEADER_H - 16) / 2);
    tft.print(titel);
}

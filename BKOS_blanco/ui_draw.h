#pragma once
#include "hw_scherm.h"
#include "ui_colors.h"

#define HEADER_H  44
#define CONTENT_Y HEADER_H
#define CONTENT_H (TFT_H - HEADER_H)
#define KNOP_R    8

void ui_knop(int x, int y, int w, int h, const char* tekst, uint16_t bg, uint16_t fg, bool actief = false);
void ui_tekst_midden(int x, int y, int w, const char* tekst, uint16_t kleur, uint8_t grootte = 1);
void ui_header(const char* titel);

#pragma once

#include "OLED_Font_X68000_8x16.h"
#define fw (8)
#define fh (16)

static void OLED_DrawFont_X68000_8x16(const byte x, const byte y, byte a) {
	const u8* pData = &OLED_Font_X68000_8x16[(a - 0x20) * fh];
	for (byte dy = 0; dy < fh; dy++) {
		u8 Data = pgm_read_byte_far(pData++);
		for (byte dx = 0; dx < fw; dx++) {
			if (Data & 0x80) { OLED_PixelOn(x + dx, y + dy); }
			Data <<= 1;
		}
	}
}

static void OLED_DrawFont_X68000_8x16(byte x, const byte y, const String str, const bool SpaceHalf = false, const u8 padx = 0) {
	const int len = str.length();
	for (u8 idx = 0; idx < len; idx++) {
		byte a = str.charAt(idx);
		OLED_DrawFont_X68000_8x16(x, y, a);
		if (a == 0x20) {
			x += (SpaceHalf ? (fw / 2) : fw) + padx;
		} else {
			x += fw + padx;
		}
	}
}

static void OLED_DrawFont_X68000_8x16(byte x, const byte y, const char* pstr, const bool SpaceHalf = false, const u8 padx = 0) {
	OLED_DrawFont_X68000_8x16(x, y, String((char*)pstr), SpaceHalf, padx);
}

#undef fw
#undef fh

#include "OLED_Font_X68000_12x24.h"
#define fw (12)
#define fh (24)

static void OLED_DrawFont_X68000_12x24(const byte x, const byte y, byte a) {
	const u16* pData = &OLED_Font_X68000_12x24[(a - 0x20) * fh];
	for (byte dy = 0; dy < fh; dy++) {
		u16 Data = pgm_read_word_far(pData++);
		for (byte dx = 0; dx < fw; dx++) {
			if (Data & 0x800) { OLED_PixelOn(x + dx, y + dy); }
			Data <<= 1;
		}
	}
}

static void OLED_DrawFont_X68000_12x24(byte x, const byte y, const String str, const bool SpaceHalf = false, const u8 padx = 0) {
	const int len = str.length();
	for (u8 idx = 0; idx < len; idx++) {
		byte a = str.charAt(idx);
		OLED_DrawFont_X68000_12x24(x, y, a);
		if (a == 0x20) {
			x += (SpaceHalf ? (fw / 2) : fw) + padx;
		} else {
			x += fw + padx;
		}
	}
}

static void OLED_DrawFont_X68000_12x24(byte x, const byte y, const char* pstr, const bool SpaceHalf = false, const u8 padx = 0) {
	OLED_DrawFont_X68000_12x24(x, y, String((char*)pstr), SpaceHalf, padx);
}

#undef fw
#undef fh

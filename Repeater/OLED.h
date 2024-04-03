#pragma once

#include "U8x8lib.h"

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

static U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE, pinOLED_SCL, pinOLED_SDA); // HSPI Reset,SCL,SDA

#define OLED_Width (128)
#define OLED_Height (64)
#define OLED_HeightBlocks (OLED_Height/8)

static u8 OLED_Buf[OLED_Width * OLED_HeightBlocks];

static void OLED_Init() {
	u8x8.begin();
	u8x8.setFlipMode(0);
}

static void OLED_Clear(int StartBlockY = -1, int BlocksCount = -1) {
	if (StartBlockY == -1) {
		StartBlockY = 0;
		if (BlocksCount == -1) { BlocksCount = OLED_HeightBlocks; }
	}
	for (int y = StartBlockY; y < StartBlockY + BlocksCount; y++) {
		for (int x = 0; x < OLED_Width; x++) {
			OLED_Buf[OLED_Width * y + x] = 0;
		}
	}
}

static void OLED_Update(int StartBlockY = -1, int BlocksCount = -1) {
	if (StartBlockY == -1) {
		StartBlockY = 0;
		if (BlocksCount == -1) { BlocksCount = OLED_HeightBlocks; }
	}
	for (int y = StartBlockY; y < StartBlockY + BlocksCount; y++) {
		u8x8.drawTile(0, y, OLED_Width / 8, &OLED_Buf[OLED_Width * y]);
	}
}

static void OLED_DrawImage(const int x, const int yblk, const u8* pData, const int Width, const int BlockHeight) {
	if ((x < 0) || (OLED_Width <= x) || (yblk < 0) || (OLED_HeightBlocks <= yblk)) { Serial.println("OLED_DrawRaw: 範囲外 x=" + String(x) + ", yblk=" + String(yblk)); while (true); }
	if (((x + Width) < 0) || (OLED_Width < (x + Width)) || ((yblk + BlockHeight) < 0) || (OLED_HeightBlocks < (yblk + BlockHeight))) { Serial.println("OLED_DrawRaw: 範囲外 x+Width=" + String(x + Width) + ", yblk+BlockHeight=" + String(yblk + BlockHeight)); while (true); }
	if ((x & 3) != 0) { Serial.println("OLED_DrawRaw: xは4ドット単位です。 x=" + String(x)); while (true); }
	if ((Width & 3) != 0) { Serial.println("OLED_DrawRaw: widthは4ドット単位です。 Width=" + String(Width)); while (true); }

	const u32* pSrc = (u32*)pData;

	for (int dy = 0; dy < BlockHeight; dy++) {
		for (int dx = 0; dx < Width; dx += 4) {
			const u32 Data = pgm_read_dword_far(pSrc++);
			u32* pDst = (u32*)&OLED_Buf[(yblk + dy) * OLED_Width + (x + dx)];
			*pDst = Data;
		}
	}
}

static void OLED_PixelOn(int x, int y) {
	if ((x < 0) || (OLED_Width <= x) || (y < 0) || (OLED_Height <= y)) { return; Serial.println("OLED_PixelOn: x,y範囲外 x=" + String(x) + ", y=" + String(y)); while (true); }
	OLED_Buf[(y / 8) * OLED_Width + x] |= 1 << (y % 8);
}

static void OLED_PixelOff(int x, int y) {
	if ((x < 0) || (OLED_Width <= x) || (y < 0) || (OLED_Height <= y)) { return; Serial.println("OLED_PixelOff: x,y範囲外 x=" + String(x) + ", y=" + String(y)); while (true); }
	OLED_Buf[(y / 8) * OLED_Width + x] &= ~(1 << (y % 8));
}

static void OLED_SetPixel(int x, int y, bool f) {
	if (f) {
		OLED_PixelOn(x, y);
	} else {
		OLED_PixelOff(x, y);
	}
}

#include "OLED_Font.h"

static void OLED_DrawLine(int x1, int y1, int x2, int y2, bool f) {
	if ((x1 == x2) && (y1 == y2)) return;

	if (abs(y2 - y1) < abs(x2 - x1)) {
		if (x2 < x1) {
			int tmp;
			tmp = x2;
			x2 = x1;
			x1 = tmp;
			tmp = y2;
			y2 = y1;
			y1 = tmp;
		}
		int y = y1 << 8;
		const int yadd = ((y2 - y1) << 8) / (x2 - x1);
		for (int x = x1; x <= x2; x++) {
			OLED_SetPixel(x, y >> 8, f);
			y += yadd;
		}
	} else {
		if (y2 < y1) {
			int tmp;
			tmp = x2;
			x2 = x1;
			x1 = tmp;
			tmp = y2;
			y2 = y1;
			y1 = tmp;
		}
		int x = x1 << 8;
		const int xadd = ((x2 - x1) << 8) / (y2 - y1);
		for (int y = y1; y <= y2; y++) {
			OLED_SetPixel(x >> 8, y, f);
			x += xadd;
		}
	}
}

static void OLED_DrawBootScreen() {
	OLED_Clear();
	OLED_DrawFont_X68000_8x16(0, 16 * 0, "Boot...", true);
	OLED_Update();
}

static u32 OLED_Timeout = 0;

#include "Image_X68000Logo.h"

static void OLED_DrawInitedScreen() {
	OLED_Timeout = millis() + 30 * 1000;

	OLED_Clear();
	OLED_DrawFont_X68000_8x16(0, 16 * 0, "MDXOnline 004", true);
	OLED_DrawFont_X68000_8x16(0, 16 * 1, "WiFi Repeater", true);
	OLED_DrawImage(OLED_Width - Image_X68000Logo_Width, 0, Image_X68000Logo, Image_X68000Logo_Width, Image_X68000Logo_BlockHeight);
	OLED_DrawFont_X68000_8x16(0, 16 * 3, "Ver."__DATE__);
	OLED_Update();
}

static void OLED_DrawDialog(String Line0, String Line1 = "", String Line2 = "", String Line3 = "") {
	OLED_Timeout = millis() + 30 * 1000;

	OLED_Clear();
	OLED_DrawFont_X68000_8x16(0, 16 * 0, Line0);
	OLED_DrawFont_X68000_8x16(0, 16 * 1, Line1);
	OLED_DrawFont_X68000_8x16(0, 16 * 2, Line2);
	OLED_DrawFont_X68000_8x16(0, 16 * 3, Line3);
	OLED_Update();
}

static void OLED_DrawProgress(String Title = "", String Desc = "", float Value = 0, float Max = 0) {
	OLED_Timeout = millis() + 3 * 1000;
	OLED_Clear();
	OLED_DrawFont_X68000_8x16(0, 24 * 0, Title);
	if (!Desc.equals("")) {
		char buf[32];
		snprintf(buf, 32, "%d %s", (int)Max, Desc);
		OLED_DrawFont_X68000_8x16(0, 24 * 1, buf);
	}
	if (Max != 0) {
		int prgx = Value * OLED_Width / Max;
		if (prgx < 0) { prgx = 0; }
		if (OLED_Width < prgx) { prgx = OLED_Width; }
		for (int x = 0; x < prgx; x++) {
			for (int y = OLED_Height - 8; y < OLED_Height; y++) {
				OLED_PixelOn(x, y);
			}
		}
	}
	OLED_Update();
}

static void OLED_DrawScreenSaver() {
	OLED_Timeout = millis() + 30 * 1000;
	OLED_Clear();
	const u8 x = random(OLED_Width - Image_X68000Logo_Width) & ~3;
	const u8 yblk = random(OLED_HeightBlocks - Image_X68000Logo_BlockHeight);
	OLED_DrawImage(x, yblk, Image_X68000Logo, Image_X68000Logo_Width, Image_X68000Logo_BlockHeight);
	OLED_Update();
}

static bool OLED_isTimeout() {
	if (OLED_Timeout != 0) {
		if (OLED_Timeout < millis()) {
			return true;
		}
	}
	return false;
}
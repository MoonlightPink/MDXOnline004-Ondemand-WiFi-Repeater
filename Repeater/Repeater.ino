
#include "incs.h"

static void LittleFS_Init() {
	if (!LittleFS.begin()) {
		DebugSerial.println("LittleFS mount error.");
		LittleFS.format();
	}

	DebugSerial.println("LittleFS informations.");
	DebugSerial.println("totalBytes=\t" + String(LittleFS.totalBytes()));
	DebugSerial.println("usedBytes=\t" + String(LittleFS.usedBytes()));
	DebugSerial.println("FreeBytes=\t" + String(LittleFS.totalBytes() - LittleFS.usedBytes()));
}

static void LittleFS_CheckInterruptButton() {
	u32 wait = 15 * 1000; // 15秒待つ
	u32 timeout = millis() + wait;
	while (isPressInterruptButton()) {
		yield();
		if (timeout <= millis()) {
			LittleFS.format();
			OLED_Clear();
			OLED_DrawFont_X68000_8x16(0, 16 * 0, "The settings have", true);
			OLED_DrawFont_X68000_8x16(0, 16 * 1, "been initialized.", true);
			OLED_DrawFont_X68000_8x16(0, 16 * 2, "Please turn off", true);
			OLED_DrawFont_X68000_8x16(0, 16 * 3, " the power.", true);
			OLED_Update();
			while (true) { yield(); }
		}
		int prgx = (timeout - millis()) * OLED_Width / wait;
		if (prgx < 0) { prgx = 0; }
		if (OLED_Width < prgx) { prgx = OLED_Width; }
		OLED_Clear();
		OLED_DrawFont_X68000_8x16(0, 16 * 0, "Are you sure you", true);
		OLED_DrawFont_X68000_8x16(0, 16 * 1, " want to reset", true);
		OLED_DrawFont_X68000_8x16(0, 16 * 2, " the settings?", true);
		for (int x = 0; x < prgx; x++) {
			for (int y = OLED_Height - 8; y < OLED_Height; y++) {
				OLED_PixelOn(x, y);
			}
		}
		OLED_Update();
	}
}

void setup() {
	delay(1000);
	DebugSerial.begin(115200);
	DebugSerial.println("MDXOnline004 Ondemand WiFi Repeater " __DATE__ " " __TIME__);

	SetOnboardLED(true);

	OLED_Init();
	OLED_DrawBootScreen();

	pinMode(pinInterruptSW, INPUT_PULLUP);

	{
		u32 ChipID = 0;
		for (int i = 0; i < 17; i = i + 8) {
			ChipID |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
		}
		char tmp[64];
		sprintf(tmp, "%06x", ChipID);
		DebugSerial.println("Chip is " + String(ESP.getChipModel()) + " Revision v" + String(ESP.getChipRevision()) + ", Cores:" + String(ESP.getChipCores()) + ", ID:" + String(tmp));
	}

	LittleFS_Init();
	LittleFS_CheckInterruptButton();
	Settings.Init();
	LocalFiles_Init(false);
	FSCache.ClearAll();

	CWiFiSTA::Disconnect();

	FSMain.Disconnect();

	Physical.Init();

	OLED_DrawInitedScreen();

	DebugSerial.println("Inited.");
}

void loop() {
	if (PressedInterruptButton()) {
		Physical.SendInterrupt();
		return;
	}

	if (Physical.available()) {
		SetOnboardLED(false);
		if (Physical.ReadBuffer()) {
			RemoteExec();
			Physical.WriteBuffer();
		}
		SetOnboardLED(true);
		return;
	}

	delay(1);

	CWiFiSTA::Interval();

	if (OLED_isTimeout()) {
		OLED_DrawScreenSaver();
	}

	if (!OLED_PrgTitle.equals("")) {
		static u8 a = 0;
		if (OLED_PrgMax != 0) {
			if (a == 0) {
				OLED_DrawProgress(OLED_PrgTitle, OLED_PrgDesc, OLED_PrgValue, OLED_PrgMax);
			}
			a = (a + 1) % 16;
		} else {
			a = 0;
			OLED_DrawProgress(OLED_PrgTitle, OLED_PrgDesc, OLED_PrgValue, OLED_PrgMax);
		}
		OLED_PrgTitle = "";
	}
}

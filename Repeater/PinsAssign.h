#pragma once

// ESP32専用

static void FastDigitalWriteHigh(u8 pin) {
	if (pin < 32) {
		*((volatile u32*)GPIO_OUT_W1TS_REG) |= 1 << pin;
	} else {
		*((volatile u32*)GPIO_OUT1_W1TS_REG) |= 1 << pin;
	}
}

static void FastDigitalWriteLow(u8 pin) {
	if (pin < 32) {
		*((volatile u32*)GPIO_OUT_W1TC_REG) |= 1 << pin;
	} else {
		*((volatile u32*)GPIO_OUT1_W1TC_REG) |= 1 << pin;
	}
}

static void FastDigitalWrite(u8 pin, bool Level) {
	if (Level) {
		FastDigitalWriteHigh(pin);
	} else {
		FastDigitalWriteLow(pin);
	}
}

#define ESP32RegsLow32 (*((volatile u32*)GPIO_OUT_REG))

static void SetOnboardLED(bool f) {
	const u8 pin = 2;
	pinMode(pin, OUTPUT);
	digitalWrite(pin, f);
}

static const u8 pinData0 = 18;
static const u8 pinData1 = 19;
static const u8 pinData2 = 21;
static const u8 pinData3 = 22;
static const u8 pinData4 = 23;
static const u8 pinData5 = 25;
static const u8 pinData6 = 26;
static const u8 pinData7 = 27;
static const u8 pinNC1 = 2;
static const u8 pinNC2 = 4;
static const u8 pinSerialData = 16;
static const u8 pinSerialReady = 33;
static const u8 pinDataClock = 5;
static const u8 pinDataRecv = 32;

static const u8 pinInterruptSW = 15;

static const u8 pinOLED_SDA = 13;
static const u8 pinOLED_SCL = 14;

static void PinsInit_ins_Output(const u8 pin) {
	// pinMode(pin, OUTPUT_OPEN_DRAIN); // オープンドレイン（プルアップは68側で行う）（遅い）
	pinMode(pin, OUTPUT); // 3.3V出力
	FastDigitalWrite(pin, true);
}

static void PinsInit_ins_Input(const u8 pin) {
	pinMode(pin, INPUT); // ハイインピーダンス（プルアップは68側で行う）（遅い）
}

static bool isPressInterruptButton() {
	return !digitalRead(pinInterruptSW);
}

static bool PressedInterruptButton() {
	u32 ms = millis();
	while (isPressInterruptButton()) { yield(); }
	u32 time = millis() - ms;
	return 50 <= time; // チャタリング防止時間: 50ms
}

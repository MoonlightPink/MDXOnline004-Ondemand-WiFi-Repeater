#pragma once

static void SerialInit() {
	Serial2.begin(206611, SERIAL_8N1); // 68側はノーウェイト。キリが悪い数値でも、結構良い精度で上手いこと合わせてくれる。
}

static bool SerialAvailable() {
	return Serial2.available() != 0;
}

static void SerialClear() {
	while (SerialAvailable()) {
		Serial2.read();
	}
}

static int SerialRead() {
	if (!SerialAvailable()) {
		return -1;
	} else {
		return Serial2.read();
	}
}


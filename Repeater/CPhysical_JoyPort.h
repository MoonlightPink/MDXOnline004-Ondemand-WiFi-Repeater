#pragma once

#include "incs.h"

#include "CPhysical_JoyPort_Serial.h"

#define PHYSICALDEBUG DEBUG

static const u8 CRC8Table[0x100] = {
  0x00, 0x31, 0x62, 0x53, 0xc4, 0xf5, 0xa6, 0x97, 0xb9, 0x88, 0xdb, 0xea, 0x7d, 0x4c, 0x1f, 0x2e,
  0x43, 0x72, 0x21, 0x10, 0x87, 0xb6, 0xe5, 0xd4, 0xfa, 0xcb, 0x98, 0xa9, 0x3e, 0x0f, 0x5c, 0x6d,
  0x86, 0xb7, 0xe4, 0xd5, 0x42, 0x73, 0x20, 0x11, 0x3f, 0x0e, 0x5d, 0x6c, 0xfb, 0xca, 0x99, 0xa8,
  0xc5, 0xf4, 0xa7, 0x96, 0x01, 0x30, 0x63, 0x52, 0x7c, 0x4d, 0x1e, 0x2f, 0xb8, 0x89, 0xda, 0xeb,
  0x3d, 0x0c, 0x5f, 0x6e, 0xf9, 0xc8, 0x9b, 0xaa, 0x84, 0xb5, 0xe6, 0xd7, 0x40, 0x71, 0x22, 0x13,
  0x7e, 0x4f, 0x1c, 0x2d, 0xba, 0x8b, 0xd8, 0xe9, 0xc7, 0xf6, 0xa5, 0x94, 0x03, 0x32, 0x61, 0x50,
  0xbb, 0x8a, 0xd9, 0xe8, 0x7f, 0x4e, 0x1d, 0x2c, 0x02, 0x33, 0x60, 0x51, 0xc6, 0xf7, 0xa4, 0x95,
  0xf8, 0xc9, 0x9a, 0xab, 0x3c, 0x0d, 0x5e, 0x6f, 0x41, 0x70, 0x23, 0x12, 0x85, 0xb4, 0xe7, 0xd6,
  0x7a, 0x4b, 0x18, 0x29, 0xbe, 0x8f, 0xdc, 0xed, 0xc3, 0xf2, 0xa1, 0x90, 0x07, 0x36, 0x65, 0x54,
  0x39, 0x08, 0x5b, 0x6a, 0xfd, 0xcc, 0x9f, 0xae, 0x80, 0xb1, 0xe2, 0xd3, 0x44, 0x75, 0x26, 0x17,
  0xfc, 0xcd, 0x9e, 0xaf, 0x38, 0x09, 0x5a, 0x6b, 0x45, 0x74, 0x27, 0x16, 0x81, 0xb0, 0xe3, 0xd2,
  0xbf, 0x8e, 0xdd, 0xec, 0x7b, 0x4a, 0x19, 0x28, 0x06, 0x37, 0x64, 0x55, 0xc2, 0xf3, 0xa0, 0x91,
  0x47, 0x76, 0x25, 0x14, 0x83, 0xb2, 0xe1, 0xd0, 0xfe, 0xcf, 0x9c, 0xad, 0x3a, 0x0b, 0x58, 0x69,
  0x04, 0x35, 0x66, 0x57, 0xc0, 0xf1, 0xa2, 0x93, 0xbd, 0x8c, 0xdf, 0xee, 0x79, 0x48, 0x1b, 0x2a,
  0xc1, 0xf0, 0xa3, 0x92, 0x05, 0x34, 0x67, 0x56, 0x78, 0x49, 0x1a, 0x2b, 0xbc, 0x8d, 0xde, 0xef,
  0x82, 0xb3, 0xe0, 0xd1, 0x46, 0x77, 0x24, 0x15, 0x3b, 0x0a, 0x59, 0x68, 0xff, 0xce, 0x9d, 0xac,
};

// FastDigitalWrite 10回 で 2us位
// 信号の立ち上がりが7usくらい掛かる

static const u32 SendBufMaxSize = 2 + CONFIG_DATASIZE + 11;
static volatile u8 SendBuf[SendBufMaxSize];
static volatile u32 SendBufSize = 0;
static volatile u32 SendBufIndex = 0;

static void IRAM_ATTR INT_DataRecv_Falling() {
	if (digitalRead(pinDataRecv)) { return; } // もしスパイクノイズで割り込みが発生したなら何もしないで帰る

	// ここでDataClock信号をON/OFFする必要はないけど、オシロスコープで見やすいのでこのままにする。

	u32 regs = ESP32RegsLow32;

	regs &= ~((1 << pinData0) | (1 << pinData1) | (1 << pinData2) | (1 << pinData3) | (1 << pinData4) | (1 << pinData5) | (1 << pinData6) | (1 << pinData7));
	regs |= 1 << pinDataClock;
	const u32 Data = SendBuf[SendBufIndex++];
	regs |= (Data & ((1 << 0) | (1 << 1))) << (pinData0 - 0);
	regs |= (Data & ((1 << 2) | (1 << 3) | (1 << 4))) << (pinData2 - 2);
	regs |= (Data & ((1 << 5) | (1 << 6) | (1 << 7))) << (pinData5 - 5);
	ESP32RegsLow32 = regs;

	delayMicroseconds(1);
	regs &= ~(1 << pinDataClock);
	ESP32RegsLow32 = regs;
}

class CPhysical {
private:
	void PinsInit() {
		PinsInit_ins_Output(pinData0);
		PinsInit_ins_Output(pinData1);
		PinsInit_ins_Output(pinData2);
		PinsInit_ins_Output(pinData3);
		PinsInit_ins_Output(pinData4);
		PinsInit_ins_Output(pinData5);
		PinsInit_ins_Output(pinData6);
		PinsInit_ins_Output(pinData7);

		PinsInit_ins_Input(pinNC1);
		PinsInit_ins_Input(pinNC2);
		PinsInit_ins_Input(pinSerialData);
		PinsInit_ins_Output(pinDataClock);
		PinsInit_ins_Output(pinSerialReady);
		PinsInit_ins_Input(pinDataRecv);
	}

public:
	CBuffer* pReceiveBuf;
	CBuffer* pSendBuf;

	void Init() {
		DebugSerial.println("Init physical serial.");

		PinsInit();

		const int ReceiveRawSize = 1 * 1024;
		static byte ReceiveRaw[ReceiveRawSize];
		pReceiveBuf = new CBuffer(&ReceiveRaw[0], ReceiveRawSize);

		const int SendRawSize = CONFIG_DATASIZE + 1024;
		static byte SendRaw[SendRawSize];
		pSendBuf = new CBuffer(&SendRaw[0], SendRawSize);

		SerialInit();

		FastDigitalWrite(pinSerialReady, false);

		FastDigitalWrite(pinDataClock, true);
	}

	bool available() {
		return SerialAvailable();
	}

	const u32 TimeoutDef = 1000;
	u32 Timeout;
	void SerialReadStart() {
		Timeout = millis() + TimeoutDef;
	}
	int SerialReadBEU8() {
		while (true) {
			int ch = SerialRead();
			if (ch == -1) {
				if (Timeout < millis()) { DebugSerial.println("SerialReadBEU8: Timeout."); return -1; }
				continue;
			}
			Timeout = millis() + TimeoutDef;
			return ch;
		}
	}
	int SerialReadBEU16() {
		int ch1 = SerialReadBEU8();
		if (ch1 == -1) { return -1; }
		int ch2 = SerialReadBEU8();
		if (ch2 == -1) { return -1; }
		return (ch1 << 8) | ch2;
	}

	bool ReadBuffer() {
		SerialReadStart();

		const int Mode = SerialReadBEU8(); // Mode: 0x00=NOP, Other=DebugStatus, 0xff=Command.

		if (Mode == -1) {
			if (PHYSICALDEBUG) { DebugSerial.println("Recv timeout."); }
			return false;
		}
		if (Mode == 0x00) {
			if (PHYSICALDEBUG) { DebugSerial.println("Recv NOP."); }
			return false;
		}
		if (Mode != 0xff) {
			DebugSerial.println("Mode: 0x" + String(Mode, HEX));
			return false;
		}

		int ReceiveBufSize = SerialReadBEU16();
		if (ReceiveBufSize == -1) { return false; }
		if (pReceiveBuf->GetMaxSize() < ReceiveBufSize) { DebugSerial.println("受信バッファサイズを超えた。 ReceiveBufSize=" + String(ReceiveBufSize)); return false; }

		pReceiveBuf->SetSizeMax();
		pReceiveBuf->SetPos(0);
		for (int idx = 0; idx < ReceiveBufSize; idx++) {
			int ch = SerialReadBEU8();
			if (ch == -1) { return false; }
			pReceiveBuf->WriteBEU8(ch);
		}
		pReceiveBuf->AdjustSizePos();

		if (PHYSICALDEBUG) {
			DebugSerial.println("Data Received:");
			DebugSerial.println("ReceiveBufSize=" + String(ReceiveBufSize));
			pReceiveBuf->SetPos(0);
			for (int idx = 0; idx < ReceiveBufSize; idx++) {
				DebugSerial.print(String(pReceiveBuf->ReadU8(), HEX) + ",");
				if ((idx & 15) == 15) { DebugSerial.println(); }
			}
			DebugSerial.println();
		}

		pReceiveBuf->SetPos(0);

		return true;
	}
	void WriteBuffer() {
		pSendBuf->SetPos(0);

		if (PHYSICALDEBUG) { DebugSerial.println("SendBuf.Size=" + String(pSendBuf->GetSize()) + ", SendBuf[0]=0x" + String(pSendBuf->ReadU8(), HEX)); }

		if (pSendBuf->GetSize() == 0) {
			DebugSerial.println("出力バッファが空なのに書き出そうとしました。");
			return;
		}

		const u32 sendbufsize = pSendBuf->GetSize();

		SendBufSize = 3 + sendbufsize;

		SendBuf[0] = sendbufsize >> 8;
		SendBuf[1] = sendbufsize >> 0;
		SendBuf[2] = 0x00; // CRC8

		pSendBuf->SetPos(0);
		u8 CheckSum = 0;
		for (int idx = 0; idx < sendbufsize; idx++) {
			const u8 Data = pSendBuf->ReadU8();
			SendBuf[3 + idx] = Data;
			CheckSum = CRC8Table[CheckSum ^ Data];
		}
		SendBuf[2] = CheckSum;

		while (true) {
			SendBufIndex = 0;
			attachInterrupt(pinDataRecv, INT_DataRecv_Falling, FALLING);
			FastDigitalWrite(pinDataClock, false);
			bool isBreak = false;
			while (!SerialAvailable()) {
				if (isPressInterruptButton()) { isBreak = true; break; }
			}
			FastDigitalWrite(pinDataClock, true);
			detachInterrupt(pinDataRecv);

			if (isBreak) { DebugSerial.println("Interrupt break."); break; }

			const int Result = SerialReadBEU8(); // Result: 0x00=OK, Other=Retry.
			if (Result == -1) { DebugSerial.println("Timeout break."); break; }
			if (Result == 0x00) { break; }
			DebugSerial.println("Retry... Result: 0x" + String(Result, HEX));
		}

		while (SerialAvailable()) { SerialRead(); }

		if (PHYSICALDEBUG) { DebugSerial.println("-------------------------------------"); }
	}
	void SendInterrupt() {
		if (PHYSICALDEBUG) { DebugSerial.println("Physical: Send interrupt."); }

		const u32 sendbufsize = 4; // 0xffを4個送る

		SendBufSize = 3 + sendbufsize;

		SendBuf[0] = sendbufsize >> 8;
		SendBuf[1] = sendbufsize >> 0;
		SendBuf[2] = 0x00; // CRC8

		u8 CheckSum = 0;
		for (int idx = 0; idx < sendbufsize; idx++) {
			const u8 Data = 0xff;
			SendBuf[3 + idx] = Data;
			CheckSum = CRC8Table[CheckSum ^ Data];
		}
		SendBuf[2] = CheckSum;

		{
			SendBufIndex = 0;
			attachInterrupt(pinDataRecv, INT_DataRecv_Falling, FALLING);
			FastDigitalWrite(pinDataClock, false);
			delay(10);
			FastDigitalWrite(pinDataClock, true);
			detachInterrupt(pinDataRecv);
		}

		SerialClear();
	}
};

static CPhysical Physical;

#pragma once 

#include "incs.h"

class CBuffer {
private:
	byte* RawData;
	int RawDataMaxLen = 0;
	int RawDataLen = 0;
	int RawDataPos = 0;

public:
	CBuffer() {
		ResetBuffer(null, 0);
	}
	CBuffer(byte* _RawData, int _RawDataMaxLen) {
		ResetBuffer(_RawData, _RawDataMaxLen);
	}

	void ResetBuffer(byte* _RawData, int _RawDataMaxLen) {
		RawData = _RawData;
		RawDataMaxLen = _RawDataMaxLen;
		RawDataLen = RawDataMaxLen;
		RawDataPos = 0;
	}

	int GetMaxSize() { return RawDataMaxLen; }

	void AdjustSizePos() { RawDataLen = RawDataPos; }

	void SetSizeMax() { RawDataLen = RawDataMaxLen; }
	int GetSize() { return RawDataLen; }
	int GetRemain() { return RawDataLen - RawDataPos; }

	int GetPos() { return RawDataPos; }
	void SetPos(int pos) { RawDataPos = pos; }

	byte* GetRaw() { return RawData; }

	byte ReadU8() {
		if (RawDataLen <= RawDataPos) {
			DebugSerial.println("CBuffer.ReadU8: バッファを超えて読み込もうとした。");
			return 0xff;
		}
		return RawData[RawDataPos++];
	}
	UInt16 ReadU16() {
		UInt16 res = 0;
		res |= (UInt16)(ReadU8() << 0);
		res |= (UInt16)(ReadU8() << 8);
		return res;
	}
	UInt32 ReadU24() {
		UInt32 res = 0;
		res |= (UInt32)(ReadU8() << 0);
		res |= (UInt32)(ReadU8() << 8);
		res |= (UInt32)(ReadU8() << 16);
		return res;
	}
	UInt32 ReadU32() {
		UInt32 res = 0;
		res |= (UInt32)(ReadU8() << 0);
		res |= (UInt32)(ReadU8() << 8);
		res |= (UInt32)(ReadU8() << 16);
		res |= (UInt32)(ReadU8() << 24);
		return res;
	}

	void ReadBuf(byte* buf, int size) {
		for (int idx = 0; idx < size; idx++) {
			buf[idx]= RawData[RawDataPos+idx];
		}
		RawDataPos += size;
	}

	byte ReadBEU8() {
		return ReadU8();
	}
	UInt16 ReadBEU16() {
		UInt16 res = 0;
		res |= (UInt16)(ReadBEU8() << 8);
		res |= (UInt16)(ReadBEU8() << 0);
		return res;
	}
	UInt32 ReadBEU24() {
		UInt32 res = 0;
		res |= (UInt32)(ReadBEU8() << 16);
		res |= (UInt32)(ReadBEU8() << 8);
		res |= (UInt32)(ReadBEU8() << 0);
		return res;
	}
	UInt32 ReadBEU32() {
		UInt32 res = 0;
		res |= (UInt32)(ReadBEU8() << 24);
		res |= (UInt32)(ReadBEU8() << 16);
		res |= (UInt32)(ReadBEU8() << 8);
		res |= (UInt32)(ReadBEU8() << 0);
		return res;
	}

	void WriteU8(byte v) {
		if (RawDataLen <= RawDataPos) {
			DebugSerial.println("CBuffer.WriteBEU8: バッファを超えて書き込もうとした。");
		}
		RawData[RawDataPos++] = v;
	}

	void WriteBEU8(byte v) {
		WriteU8(v);
	}
	void WriteBEU16(UInt32 v) {
		WriteBEU8((byte)(v >> 8));
		WriteBEU8((byte)(v >> 0));
	}
	void WriteBEU24(UInt32 v) {
		WriteBEU8((byte)(v >> 16));
		WriteBEU8((byte)(v >> 8));
		WriteBEU8((byte)(v >> 0));
	}
	void WriteBEU32(UInt32 v) {
		WriteBEU8((byte)(v >> 24));
		WriteBEU8((byte)(v >> 16));
		WriteBEU8((byte)(v >> 8));
		WriteBEU8((byte)(v >> 0));
	}

	void ReadBytes(byte* buf, int buflen) {
		for (int idx = 0; idx < buflen; idx++) {
			buf[idx] = ReadU8();
		}
	}

	void WriteBytes(byte* buf, int buflen) {
		for (int idx = 0; idx < buflen; idx++) {
			WriteBEU8(buf[idx]);
		}
	}

	void WriteString(String str) {
		for (int idx = 0; idx < str.length(); idx++) {
			WriteBEU8(str[idx]);
		}
	}
};

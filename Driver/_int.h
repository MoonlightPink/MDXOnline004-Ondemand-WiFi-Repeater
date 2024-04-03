#pragma once

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

#define EnableDebugPrint (false)

static inline void DebugPrint(const char* msg) {
	if (!EnableDebugPrint) { return; }
	_dos_print(msg);
}

static inline void DebugPrintln(const char* msg) {
	if (!EnableDebugPrint) { return; }
	DebugPrint(msg);
	DebugPrint("\r\n");
}

static inline void DebugPrintlnInt(const char* msg, int Value) {
	if (!EnableDebugPrint) { return; }
	DebugPrint(msg);
	char buffer[sizeof(int) * 8 + 1];
	itoa(Value, buffer, 10);
	DebugPrintln(buffer);
}

static inline void DebugPrintlnU8(const char* msg, u8 Value) {
	if (!EnableDebugPrint) { return; }
	DebugPrint(msg);
	char buffer[2 + 1];
	itoa(Value, buffer, 16);
	DebugPrintln(buffer);
}


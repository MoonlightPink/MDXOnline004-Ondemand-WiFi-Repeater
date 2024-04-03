#pragma once

#include "incs.h"

#define null NULL

typedef u16 UInt16;
typedef u32 UInt32;

static bool CompSJISText_IgnoreCase(const byte* buf1, const int buf1len, const byte* buf2, const int buf2len) {
	if (buf1len != buf2len) { return false; }
	const int buflen = buf1len;
	for (int idx = 0; idx < buflen; idx++) {
		byte ch1 = buf1[idx];
		byte ch2 = buf2[idx];
		bool isDouble = 0x81 <= ch1 && ch1 <= 0x9f || 0xe0 <= ch1 && ch1 <= 0xef;
		if (isDouble) {
			if (ch1 != ch2) { return false; }
			idx++;
			if (buf1[idx] != buf2[idx]) { return false; }
		} else {
			if ('A' <= ch1 && ch1 <= 'Z') { ch1 |= 0x20; }
			if ('A' <= ch2 && ch2 <= 'Z') { ch2 |= 0x20; }
			if (ch1 != ch2) { return false; }
		}
	}
	return true;
}

static bool CompSJISText_IgnoreCase(const String buf1, const byte* buf2, const int buf2len) {
	if (buf1.length() != buf2len) { return false; }
	const int buflen = buf1.length();
	for (int idx = 0; idx < buflen; idx++) {
		byte ch1 = buf1[idx];
		byte ch2 = buf2[idx];
		bool isDouble = 0x81 <= ch1 && ch1 <= 0x9f || 0xe0 <= ch1 && ch1 <= 0xef;
		if (isDouble) {
			if (ch1 != ch2) { return false; }
			idx++;
			if (buf1[idx] != buf2[idx]) { return false; }
		} else {
			if ('A' <= ch1 && ch1 <= 'Z') { ch1 |= 0x20; }
			if ('A' <= ch2 && ch2 <= 'Z') { ch2 |= 0x20; }
			if (ch1 != ch2) { return false; }
		}
	}
	return true;
}

static bool CompSJISText_IgnoreCase(const String buf1, const String buf2) {
	if (buf1.length() != buf2.length()) { return false; }
	const int buflen = buf1.length();
	for (int idx = 0; idx < buflen; idx++) {
		byte ch1 = buf1[idx];
		byte ch2 = buf2[idx];
		bool isDouble = 0x81 <= ch1 && ch1 <= 0x9f || 0xe0 <= ch1 && ch1 <= 0xef;
		if (isDouble) {
			if (ch1 != ch2) { return false; }
			idx++;
			if (buf1[idx] != buf2[idx]) { return false; }
		} else {
			if ('A' <= ch1 && ch1 <= 'Z') { ch1 |= 0x20; }
			if ('A' <= ch2 && ch2 <= 'Z') { ch2 |= 0x20; }
			if (ch1 != ch2) { return false; }
		}
	}
	return true;
}

static void MemCopy(void* src, void* dst, const int len) {
	for (int idx = 0; idx < len; idx++) {
		((byte*)dst)[idx] = ((byte*)src)[idx];
	}
}

static String u8ToString(u8 v) {
	String res = "";

	const char hexs[0x10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	res += String(hexs[v >> 4]);
	res += String(hexs[v & 0x0f]);

	return res;
}

static String BufToString(byte* pbuf, int bufsize) {
	String res = "";
	for (int idx = 0; idx < bufsize; idx++) {
		res += u8ToString(pbuf[idx]);
	}
	return res;
}

static String u16ToString(u16 v) {
	String res = "";
	res += u8ToString(v >> 8);
	res += u8ToString(v >> 0);
	return res;
}
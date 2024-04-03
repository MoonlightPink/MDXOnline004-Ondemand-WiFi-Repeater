
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>
#include <x68k/dos.h>
#include <x68k/iocs.h>

#include "Common.h"

static const int TextMaxLen = 1024;
static char Text[TextMaxLen + 1];
static int TextLen = 0;

static u8 SendCommandReceiveText(const char* Command, const char* Param) {
	char fn[256] = { 0, };
	strcat(fn, "/CMD/");
	strcat(fn, Command);
	if (1 <= strlen(Param)) {
		int SlashLen = 0;
		for (size_t idx = 0; idx < strlen(Param); idx++) {
			if ((SlashLen % 18) == 0) { strcat(fn, "/"); }
			SlashLen++;
			char buf[2] = { Param[idx],0 };
			strcat(fn, buf);
		}
	}

	// printf("fopen: "); printf(fn); printf("\r\n");

	FILE* fp = fopen(fn, "r");
	if (fp == null) { return CMD_RES_Error_ClientFile; }

	TextLen = 0;

	while (true) {
		char data;
		if (fread(&data, 1, 1, fp) != 1) { break; }
		if (TextMaxLen <= TextLen) {
			printf("Error: Buffer overflow. バッファが溢れたので中断しました。");
			break;
		}
		Text[TextLen++] = data;
	}
	Text[TextLen] = 0x00;

	return CMD_RES_OK;
}

int main(int argc, char* argv[]) {
	printf("\r\n");
	printf("MDXOnline004 Ondemand WiFi Client / Status viewer 状態表示ツール / " __DATE__ " " __TIME__ "\r\n");
	printf("\r\n");

	u8 res = SendCommandReceiveText("GetWiFiStatusText", "");
	if (res != CMD_RES_OK) {
		ShowErrorMsg(res);
		return 1;
	}
	printf(Text);

	printf("\r\n");
	return 0;
}

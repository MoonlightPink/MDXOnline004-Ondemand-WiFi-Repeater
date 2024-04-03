
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

static u8 SendCommand(const char* Command, const char* Param) {
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

	u8 res;
	if (fread(&res, 1, 1, fp) != 1) { return CMD_RES_Error_ClientFile; }
	fclose(fp);

	return res;
}

int main(int argc, char* argv[]) {
	printf("\r\n");
	printf("MDXOnline004 Ondemand WiFi Client / Setup tool 環境設定ツール / " __DATE__ " " __TIME__ "\r\n");
	printf("\r\n");

	if (argc <= 1) {
		printf("ex> Setup.x /LENG /STestStation /P1234abcd /C\r\n");
		printf("\r\n");
		printf(" -LENG Display folder and file names in English.\r\n");
		printf(" -LJPN フォルダ名やファイル名を日本語で表示する。\r\n");
		printf("\r\n");
		printf(" -SSSIDName\tSSID name to WiFi connection.\r\n");
		printf(" -SSSID名\tWiFiの接続先SSID名\r\n");
		printf("\r\n");
		printf(" -PPassphase\tPassphase for WiFi connection.\r\n");
		printf("             \tCannot contain slashes and spaces.\r\n");
		printf(" -Pパスフェーズ\tWiFi接続用パスフェーズ\r\n");
		printf("             \tスラッシュとスペースを含めることはできません。\r\n");
		printf("\r\n");
		printf(" -C\tStart WiFi connection.\r\n");
		printf(" -C\tWiFi接続開始\r\n");
		printf("\r\n");
		printf(" -D\tBreak WiFi connection.\r\n");
		printf(" -D\tWiFi切断\r\n");
		printf("\r\n");
		printf("NOTICE: SSID name and passphase is saved in plain text on the microcontroller.\r\n");
		printf("If you turn on the power using Micro USB while pressing the reset button,\r\n");
		printf("all settings will be initialized.\r\n");
		printf("\r\n");
		printf("注意: SSID名とパスフェーズはプレーンテキストでマイコンに保存します。\r\n");
		printf("リセットボタンを押しながらMicroUSBで電源を入れると、全ての設定が初期化されます。\r\n");
		printf("\r\n");
		return 1;
	}

	for (int idx = 1; idx < argc; idx++) {
		char* arg = argv[idx];
		// printf("args %d / %d, argv=[%s]\r\n", idx, argc, arg);
		if (arg[0] == '-') {
			char Command = arg[1] | 0x20;
			const char* Param = &arg[2];
			switch (Command) {
			case 'l': {
				printf("Change language settings to %s.\r\n", Param);
				u8 res = SendCommand("SetLang", Param);
				if (res != CMD_RES_OK) { ShowErrorMsg(res); return 1; }
			} break;
			case 's': {
				printf("Change SSID name to WiFi connection to %s\r\n", Param);
				u8 res = SendCommand("SetSSID", Param);
				if (res != CMD_RES_OK) { ShowErrorMsg(res); return 1; }
			} break;
			case 'p': {
				printf("Change passphase for WiFi connection to %s\r\n", Param);
				u8 res = SendCommand("SetPass", Param);
				if (res != CMD_RES_OK) { ShowErrorMsg(res); return 1; }
			} break;
			case 'c': {
				printf("Connect WiFi.\r\n");
				u8 res = SendCommand("Connect", "");
				if (res != CMD_RES_OK) { ShowErrorMsg(res); return 1; }
			}break;
			case 'd': {
				printf("Disconnect WiFi.\r\n");
				u8 res = SendCommand("Disconnect", Param);
				if (res != CMD_RES_OK) { ShowErrorMsg(res); return 1; }
			} break;
			default: {
				printf("Unknown command.");
				return 1;
			}break;
			}
		}
	}

	printf("処理が完了しました。\r\n");
	printf("\r\n");
	return 0;
}

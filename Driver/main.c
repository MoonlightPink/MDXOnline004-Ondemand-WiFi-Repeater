/*
 * Copyright (c) 2023 Yuichi Nakamura (@yunkya2)
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>
#include <x68k/dos.h>
#include <x68k/iocs.h>

#include <config.h>
#include "x68kremote.h"
#include "remotedrv.h"

#include "_int.h"

 //****************************************************************************
// Global variables
//****************************************************************************

bool recovery = false;  //エラー回復モードフラグ
int timeout = 500;      //コマンド受信タイムアウト(5sec)
int resmode = 0;        //登録モード (0:常に登録 / 1:起動時にサーバと通信できたら登録)

#ifdef DEBUG
int debuglevel = 0;
#endif

//****************************************************************************
// for debugging
//****************************************************************************

#ifdef DEBUG
char heap[1024];                // temporary heap for debug print
void* _HSTA = heap;
void* _HEND = heap + 1024;
void* _PSP;

void DPRINTF(int level, char* fmt, ...) {
	char buf[256];
	va_list ap;

	if (debuglevel < level)
		return;

	va_start(ap, fmt);
	vsiprintf(buf, fmt, ap);
	va_end(ap);
	_iocs_b_print(buf);
}

void DNAMEPRINT(void* n, bool full, char* head) {
	struct dos_namestbuf* b = (struct dos_namestbuf*)n;

	DPRINTF1("%s%c:", head, b->drive + 'A');
	for (int i = 0; i < 65 && b->path[i]; i++) {
		DPRINTF1("%c", (uint8_t)b->path[i] == 9 ? '\\' : (uint8_t)b->path[i]);
	}
	if (full)
		DPRINTF1("%.8s%.10s.%.3s", b->name1, b->name2, b->ext);
}
#endif

//****************************************************************************
// Communication
//****************************************************************************

#include "main_Physical.h"

//****************************************************************************
// Utility routine
//****************************************************************************

static int my_atoi(char* p) {
	int res = 0;
	while (*p >= '0' && *p <= '9') {
		res = res * 10 + *p++ - '0';
	}
	return res;
}

//****************************************************************************
// Device driver interrupt rountine
//****************************************************************************

void com_timeout(struct dos_req_header* req) {
	if (resmode == 1) {     // 起動時にサーバが応答しなかった
		_dos_print("リモートドライブサービスが応答しないため組み込みません\r\n");
	}
	DPRINTF1("command timeout\r\n");
	req->errh = 0x10;
	req->errl = 0x02;
	req->status = -1;
	recovery = true;
}

int com_init(struct dos_req_header* req) {
	int units = 1;
#ifdef CONFIG_BOOTDRIVER
	_iocs_b_print
#else
	_dos_print
#endif
		("\r\nRemote drive driver with Joystick ports for MDXOnline004 Ondemand WiFi Repeater (" __DATE__ ")\r\n");

#ifdef CONFIG_BOOTDRIVER
#else
	char* p = (char*)req->status;
	p += strlen(p) + 1;
	while (*p != '\0') {
		if (*p == '/' || *p == '-') {
			p++;
			switch (*p | 0x20) {
#ifdef DEBUG
			case 'd':         // /D .. デバッグレベル増加
				debuglevel++;
				break;
#endif
			case 'r':         // /r<mode> .. 登録モード
				p++;
				resmode = my_atoi(p);
				break;
			case 't':         // /t<timeout> .. タイムアウト設定
				p++;
				timeout = my_atoi(p) * 100;
				if (timeout == 0)
					timeout = 500;
				break;
			case 'u':         // /u<units> .. ユニット数設定
				p++;
				units = my_atoi(p);
				if (units < 1 || units > 7)
					units = 1;
				break;
			}
		} else if (*p >= '0' && *p <= '9') {
		}
		p += strlen(p) + 1;
	}

#endif

#ifndef CONFIG_BOOTDRIVER
	if (resmode != 0) {     // サーバが応答するか確認する
		struct cmd_init cmd;
		struct res_init res;
		cmd.command = 0x00; /* init */
		com_cmdres(&cmd, sizeof(cmd), &res, sizeof(res));
		DPRINTF1("CHECK:\r\n");
	}
	resmode = 0;  // 応答を確認できたのでモードを戻す

	_dos_print("ドライブ");
	_dos_putchar('A' + *(char*)&req->fcb);
	if (units > 1) {
		_dos_print(":-");
		_dos_putchar('A' + *(char*)&req->fcb + units - 1);
	}
	_dos_print(":でジョイスティックポート#1/#2に接続したリモートドライブが利用可能です\r\n");
#endif
	DPRINTF1("Debug level: %d\r\n", debuglevel);

	Physical_Init();

	return units;
}

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

#ifndef _REMOTEDRV_H_
#define _REMOTEDRV_H_

#include <stdbool.h>
#include <stdint.h>
#include <setjmp.h>
#include "x68kremote.h"

#define DPRINTF1(...)  DPRINTF(1, __VA_ARGS__)
#define DPRINTF2(...)  DPRINTF(2, __VA_ARGS__)
#define DPRINTF3(...)  DPRINTF(3, __VA_ARGS__)

#ifdef DEBUG
void DPRINTF(int level, char *fmt, ...);
void DNAMEPRINT(void *n, bool full, char *head);
#else
#define DPRINTF(...)
#define DNAMEPRINT(n, full, head)
#endif

extern jmp_buf jenv;

void com_cmdres(void *wbuf, size_t wsize, void *rbuf, size_t rsize);
void com_timeout(struct dos_req_header *req);
int com_init(struct dos_req_header *req);

union Tb {
	struct cmd_init     cmd_init;
	struct res_init     res_init;
	struct cmd_dirop    cmd_dirop;
	struct res_dirop    res_dirop;
	struct cmd_rename   cmd_rename;
	struct res_rename   res_rename;
	struct cmd_chmod    cmd_chmod;
	struct res_chmod    res_chmod;
	struct cmd_files    cmd_files;
	struct res_files    res_files;
	struct cmd_nfiles   cmd_nfiles;
	struct res_nfiles   res_nfiles;
	struct cmd_create   cmd_create;
	struct res_create   res_create;
	struct cmd_open     cmd_open;
	struct res_open     res_open;
	struct cmd_close    cmd_close;
	struct res_close    res_close;
	struct cmd_read     cmd_read;
	struct res_read     res_read;
	struct cmd_write    cmd_write;
	struct res_write    res_write;
	struct cmd_filedate cmd_filedate;
	struct res_filedate res_filedate;
	struct cmd_dskfre   cmd_dskfre;
	struct res_dskfre   res_dskfre;
};

static union Tb b;

#endif /* _REMOTEDRV_H_ */

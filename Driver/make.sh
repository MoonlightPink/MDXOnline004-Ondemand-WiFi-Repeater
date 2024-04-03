#!/bin/bash

rm mdxo004.sys

m68k-xelf-gcc -m68000 -I. -c head.S
m68k-xelf-gcc -g -m68000 -I. -Os -finput-charset=utf-8 -fexec-charset=cp932 -c remotedrv.c
m68k-xelf-gcc -g -m68000 -I. -Os -finput-charset=utf-8 -fexec-charset=cp932 -c main.c
m68k-xelf-gcc -specs=c++small.specs -o mdxo004.sys head.o remotedrv.o main.o -nostartfiles -s

rm head.o
rm remotedrv.o
rm main.o
rm mdxo004.sys.elf


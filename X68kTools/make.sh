#!/bin/bash

rm Setup.x
rm Setup.lzx
m68k-xelf-gcc -g -m68000 -I. -Wall -Os -finput-charset=utf-8 -fexec-charset=cp932 -c Setup.cpp
m68k-xelf-gcc -specs=c++small.specs -o Setup.x Setup.o -s
rm Setup.o
rm Setup.x.elf
./run68.exe lzx.x -o Setup.lzx Setup.x | iconv -f utf-8 -t cp932
cp Setup.lzx ../Repeater/LocalFiles/Setup.x

rm Status.x
rm Status.lzx
m68k-xelf-gcc -g -m68000 -I. -Wall -Os -finput-charset=utf-8 -fexec-charset=cp932 -c Status.cpp
m68k-xelf-gcc -specs=c++small.specs -o Status.x Status.o -s
rm Status.o
rm Status.x.elf
./run68.exe lzx.x -o Status.lzx Status.x | iconv -f utf-8 -t cp932
cp Status.lzx ../Repeater/LocalFiles/Status.x

./run68.exe Setup.x -LENG | iconv -f cp932 -t utf-8

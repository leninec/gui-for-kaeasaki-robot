#pragma once

//#define BYTE unsigned char

// 1 byte
#define b00000001 1
#define b00000010 2
#define b00000100 4
#define b00001000 8
#define b00010000 16
#define b00100000 32
#define b01000000 64
#define b10000000 128

// 2 byte
#define b0000000000000001 1
#define b0000000000000010 2
#define b0000000000000100 4
#define b0000000000001000 8
#define b0000000000010000 16
#define b0000000000100000 32
#define b0000000001000000 64
#define b0000000010000000 128
#define b0000000100000000 256
#define b0000001000000000 512
#define b0000010000000000 1024
#define b0000100000000000 2048
#define b0001000000000000 4096
#define b0010000000000000 8192
#define b0100000000000000 16384
#define b1000000000000000 32768

#define SWITCH_ON(x,y) x = x | y
#define SWITCH_OFF(x,y) x = x & (~y)
#define COMPARE(x,y) (x & y)

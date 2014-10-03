#pragma once

#include <winsock2.h>
#include <windows.h>
#include <float.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

typedef unsigned char byte;

void PatchData(void *buf, unsigned long size, unsigned long addr);
void PatchByte(unsigned char byte, unsigned long size, unsigned long addr);

class memptr {
	/* Due to licensing, portions of this file have been omitted. */
};

class ASMHack {
	/* Due to licensing, portions of this file have been omitted. */
}
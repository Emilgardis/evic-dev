#include <stdint.h>

#ifndef MODE0_BITMAP_H
#define MODE0_BITMAP_H

uint8_t mode0_bitmap[] = {
	0x00, 0x00, 0xC0, 0xE0, 0xC3, 0x01, 0x3C, 0x00,
	0x3C, 0x00, 0x03, 0x08, 0x60, 0x00, 0xE1, 0xFC,
	0xC1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C,
	0x30, 0x84, 0x80, 0x04, 0x48, 0x80, 0x04, 0x48,
	0x40, 0x08, 0x03, 0x0F, 0x00, 0x00, 0x00, 0x00,
	0x0E, 0xBF, 0x1C, 0x48, 0x80, 0x04, 0x48, 0x80,
	0x04, 0x44, 0x40, 0x08, 0x03, 0x0F, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xE0, 0xF0, 0xCB, 0x85, 0x44,
	0x48, 0x84, 0x44, 0x48, 0x04, 0x04, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x7C, 0x30, 0x8C, 0x80, 0x04,
	0x48, 0x80, 0x04, 0xC4, 0x30, 0xF8, 0x00, 0x00
};

int mode0_bitmap_width  = 64;
int mode0_bitmap_height = 12;

#endif

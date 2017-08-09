#ifndef _TYPES_H
#define _TYPES_H
#include <sys/types.h>
#include <stdint.h>
struct Color {
	uint8_t r, g, b;
};

struct ImgDesc {
	void * head() { return &imgSep; }
	static size_t size() { return 10; }
	uint8_t _pad1;
	uint8_t imgSep;
	uint16_t left;
	uint16_t top;
	uint16_t width;
	uint16_t height;
	uint8_t localTblSz : 3;
	uint8_t reserved : 2;
	uint8_t sort : 1;
	uint8_t interlace : 1;
	uint8_t localTbl : 1;
	uint8_t _pad2 ;
};

struct LgcScrDesc{
    static size_t size() {return 7;}
    void * head() {return &width;}
    uint16_t width;
    uint16_t height;

    //packed field
    uint8_t g_color_tbl_sz : 3;
    uint8_t sort : 1;
    uint8_t color_res : 3;
    uint8_t g_color_tbl : 1;

    uint8_t bgColorIdx;
    uint8_t bgPixelRatio;
    uint8_t _pad;
};

#endif// _TYPES_H

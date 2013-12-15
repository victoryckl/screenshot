/********************************************************************
	created:	2012/04/07
	filename: 	readbmp.c
	author:		
	
	purpose:	
*********************************************************************/

//-------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "mybmp.h"

//行数据翻转
void rgb565_line_reversal(void * p565, int w, int h)
{
	int i;
	unsigned char * pline = NULL;
	unsigned char * pline1 = NULL;
	unsigned char * linebuffer = NULL;
	int linesize;
	if (p565 && w > 0 && h > 1) {//至少两行才需要翻转
		linesize = ((w * 2) + 3) & ~3;//4的整数倍
		linebuffer = (unsigned char *)malloc(linesize);
		if (linebuffer) {
			pline = (unsigned char *)p565;
			pline1 = (unsigned char *)p565 + linesize * (h - 1);
			for (i = 0; i < (h>>1); i++) {
				memcpy(linebuffer, pline, linesize);
				memcpy(pline, pline1, linesize);
				memcpy(pline1, linebuffer, linesize);
				pline += linesize;
				pline1 -= linesize;
			}
			free(linebuffer);
		}
	}
}

void rgb888_line_reversal(void * p888, int w, int h)
{
	int i;
	unsigned char * pline = NULL;
	unsigned char * pline1 = NULL;
	unsigned char * linebuffer = NULL;
	int linesize;
	if (p888 && w > 0 && h > 1) {//至少两行才需要翻转
		linesize = w * 3;
		linebuffer = (unsigned char *)malloc(linesize);
		if (linebuffer) {
			pline = (unsigned char *)p888;
			pline1 = (unsigned char *)p888 + w * (h - 1) * 3;
			for (i = 0; i < (h>>1); i++) {
				memcpy(linebuffer, pline, linesize);
				memcpy(pline, pline1, linesize);
				memcpy(pline1, linebuffer, linesize);
				pline += linesize;
				pline1 -= linesize;
			}
			free(linebuffer);
		}
	}
}

static int rgb565_to_rgb888(const void * p565, int w, int h, void * p888)
{
	unsigned char * p888end = NULL;
	unsigned char * p888line = NULL;
	const unsigned short * p565end = NULL;
	const unsigned short * p565line = NULL;
	if (p565 && p888 && w > 0 && h > 0) {
		p565line = (unsigned short *)p565;
		p565end = (unsigned short *)p565 + w * h;
		p888line = (unsigned char *)p888;
		
		while (p565line < p565end) {
			//565 b|g|r -> 888 r|g|b
			*p888line++ = (unsigned char)(((*p565line) >> 0 ) << 3);
			*p888line++ = (unsigned char)(((*p565line) >> 5 ) << 2);
			*p888line++ = (unsigned char)(((*p565line) >> 11) << 3);
			p565line++;
		}
		return 0;
	} else {
		printf("rgb888_to_rgb565 : parameter error\n");
	}
	return -1;
}

static int rgb888_to_rgb565(const void * p888, int w, int h, void * p565)
{
	const unsigned char * p888end = NULL;
	const unsigned char * p888line = NULL;
	unsigned short * p565end = NULL;
	unsigned short * p565line = NULL;
	if (p565 && p888 && w > 0 && h > 0) {
		p888line = (unsigned char *)p888;
		p888end = (unsigned char *)p888 + w * 3 * h;
		p565line = (unsigned short *)p565;

		while (p888line < p888end) {
			//888 r|g|b -> 565 b|g|r
			*p565line =  (((p888line[0] >> 3) & 0x1F) << 0)//r
						|(((p888line[1] >> 2) & 0x3F) << 5)//g
						|(((p888line[2] >> 3) & 0x1F) << 11);//b
			p565line++;
			p888line+=3;
		}
		return 0;
	} else {
		printf("rgb888_to_rgb565 : parameter error\n");
	}
	return -1;
}


void * rgb888_to_rgb565_buffer(const void * p888, int w, int h)
{
	int size = w * h * 2;
	void * p565 = NULL;
	if (p888 && w > 0 && h > 0) {
		p565 = malloc(size);
		if (p565)
			rgb888_to_rgb565(p888, w, h, p565);
	}
	return p565;
}

void * rgb565_to_rgb888_buffer(const void * p565, int w, int h)
{
	int size = w * h * 3;
	void * p888 = NULL;
	if (p565 && w > 0 && h > 0) {
		p888 = malloc(size);
		if (p888)
			rgb565_to_rgb888(p565, w, h, p888);
	}
	return p888;
}

//-------------------------------------------------------------------

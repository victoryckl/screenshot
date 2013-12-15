/********************************************************************
	created:	2012/04/07
	filename: 	bmp_to_screen.c
	author:		
	
	purpose:	
*********************************************************************/

//-------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./myfb.h"
#include "./mybmp.h"

static int g_action_type[][eRGB_MAX] = 
{
	{eACTION_MIN, eACTION_MIN,               eACTION_MIN},
	{eACTION_MIN, eBMP_RGB565_TO_FB0_RGB565, eBMP_RGB565_TO_FB0_RGB888},
	{eACTION_MIN, eBMP_RGB888_TO_FB0_RGB565, eBMP_RGB888_TO_FB0_RGB888},
};

typedef void * (* RGB_CONVERT)(const void * p888, int w, int h);
typedef void   (* RGB_REVERSAL)(void * pdata, int w, int h);
typedef void   (* RGB_COPY)(void * psrc, void * pdst, int sw, int sh, int dw, int dh);

typedef struct tagOperate
{
	RGB_CONVERT rgb_convert;
	RGB_REVERSAL rgb_reversal;
	RGB_COPY rgb_copy;
}OPERATE;

static void rbg565_copy_to_fb0(void * psrc, void * pdst, int sw, int sh, int dw, int dh);
static void rbg888_copy_to_fb0(void * psrc, void * pdst, int sw, int sh, int dw, int dh);

static const OPERATE g_operate[] =
{
	{NULL,                    rgb565_line_reversal, rbg565_copy_to_fb0},
	{rgb565_to_rgb888_buffer, rgb888_line_reversal, rbg888_copy_to_fb0},
	{rgb888_to_rgb565_buffer, rgb565_line_reversal, rbg565_copy_to_fb0},
	{NULL,                    rgb888_line_reversal, rbg888_copy_to_fb0},
};

static void rbg565_copy_to_fb0(void * psrc, void * pdst, int sw, int sh, int dw, int dh)
{
	int srclinesize = ((sw * 2) + 3) & ~3;//4的整数倍
	int dstlinesize = dw * 2;
	int copylinesize = (sw < dw ? sw : dw) * 2;
	int copylines = sh < dh ? sh : dh;



	unsigned char * psrcline = (unsigned char *)psrc;
	unsigned char * pend = psrcline + copylines * sw * 2;
	unsigned char * pdstline = (unsigned char *)pdst;

	while (psrcline < pend) {
		memcpy(pdstline, psrcline, copylinesize);
		psrcline += srclinesize;
		pdstline += dstlinesize;
	}
}

static void rbg888_copy_to_fb0(void * psrc, void * pdst, int sw, int sh, int dw, int dh)
{
	int srclinesize = sw * 3;
	int dstlinesize = dw * 3;
	int copylinesize = (sw < dw ? sw : dw) * 3;
	int copylines = sh < dh ? sh : dh;
	
	unsigned char * psrcline = (unsigned char *)psrc;
	unsigned char * pend = psrcline + copylines * sw * 3;
	unsigned char * pdstline = (unsigned char *)pdst;
	
	while (psrcline < pend) {
		memcpy(pdstline, psrcline, copylinesize);
		psrcline += srclinesize;
		pdstline += dstlinesize;
	}
}

static int get_index(int bpp)
{
	int index = eRGB_MIN;
	if (16 == bpp) {
		index = eRGB_565;
	} else if (24 == bpp) {
		index = eRGB_888;
	}
	return index;
}


static int get_action_type(const char * path)
{
	ImageInfo imageinfo;
#ifdef WIN32
	bmp_info(path, &imageinfo);
	return eBMP_TO_BMP;
#else
	int action_type = eACTION_MIN;
	FBinfo fbinfo;
	do 
	{
		if (bmp_info(path, &imageinfo) < 0) {
			break;
		}
		if (fb_info(&fbinfo) < 0) {
			break;
		}

		action_type = g_action_type[get_index(imageinfo.bpp)][get_index(fbinfo.bpp)];
		if (eACTION_MIN == action_type) {
			printf("get_action_type : not support type!\n");
			break;
		}
	} while (0);

	return action_type;
#endif
}

#ifndef WIN32
static int to_screen(BMP * bmp, int action_type)
{
	int success = -1;
	int w, h;
	void * pdata = NULL;
	void * psrc = NULL;
	const OPERATE * operate = NULL;
	struct FB * fb = NULL;
	do 
	{
		fb = fb_create(0);
		if (!fb) {
			break;
		}
		w = bmp_width(bmp);
		h = bmp_height(bmp);
		psrc = bmp_data(bmp);
		operate = &g_operate[action_type];

		if (operate->rgb_convert) {
			pdata = operate->rgb_convert(psrc, w, h);
			if (NULL == pdata)
				break;
			psrc = pdata;
		}

		if (!bmp_forward(bmp)) {
			if (operate->rgb_reversal)
				operate->rgb_reversal(psrc, w, h);
		}

		if (operate->rgb_copy)
			operate->rgb_copy(psrc, fb_bits(fb), bmp_width(bmp), bmp_height(bmp), fb_width(fb), fb_height(fb));

		success = 0;
	} while (0);

	if (pdata) {
		free(pdata);
	}
	if (fb) {
		fb_destory(fb);
	}
	return success;
}
#endif

static int to_file(BMP * bmp)
{
	int bitCount = bmp_bpp(bmp);
	int w = bmp_width(bmp);
	int h = bmp_height(bmp);
	void * pdata = bmp_data(bmp);
	if (!bmp_forward(bmp)) {//不是正向的图片数据，需要翻转
		if (16 == bitCount) {
			rgb565_line_reversal(pdata, w, h);
		} else if (24 == bitCount) {
			rgb888_line_reversal(pdata, w, h);
		}
	}
	return save_bmp(ROOT_PATH"/out.bmp", w, h, pdata, bitCount);
}

int bmp_to_screen(const char * path)
{
	int action_type = eACTION_MIN;
	BMP * bmp = NULL;
	//printf("bmp_to_screen enter!\n");
	do 
	{
		if (!path) {
			printf("bmp_to_screen : path is null!\n");
			break;
		}
		action_type = get_action_type(path);
		if (eACTION_MIN == action_type) {
			break;
		}
		bmp = bmp_open(path);
		if (!bmp) {
			break;
		}

#ifdef WIN32
		to_file(bmp);
#else
		to_screen(bmp, action_type);
#endif

	} while (0);

	if (bmp) {
		bmp_close(bmp);
	}
	//printf("bmp_to_screen exit!\n");
	return 0;
}

int tofb(int argc, char * argv[])
{
	int ret = -1;
	char path[256] = ROOT_PATH"/bmp/in.565.negative.bmp";

	if (argc >= 2) {
		strcpy(path, argv[1]);
	}
	bmp_to_screen(path);
	return 0;
}


//--输出图片到内存------------------------------------------------

static int get_action_type_buffer(const char * path, int buffer_bits)
{
	ImageInfo imageinfo;
	int action_type = eACTION_MIN;
	do 
	{
		if (bmp_info(path, &imageinfo) < 0) {
			break;
		}
		action_type = g_action_type[get_index(imageinfo.bpp)][get_index(buffer_bits)];
		if (eACTION_MIN == action_type) {
			printf("get_action_type : not support type!\n");
			break;
		}
	} while (0);
	
	return action_type;
}

static int to_buffer(BMP * bmp, void * buffer, int buffer_w, int buffer_h, int action_type)
{
	int success = -1;
	int w, h;
	void * pdata = NULL;
	void * psrc = NULL;
	const OPERATE * operate = NULL;
	struct FB * fb = NULL;
	do 
	{
		w = bmp_width(bmp);
		h = bmp_height(bmp);
		psrc = bmp_data(bmp);
		operate = &g_operate[action_type];
		
		if (operate->rgb_convert) {
			pdata = operate->rgb_convert(psrc, w, h);
			if (NULL == pdata)
				break;
			psrc = pdata;
		}
		
		if (!bmp_forward(bmp)) {
			if (operate->rgb_reversal)
				operate->rgb_reversal(psrc, w, h);
		}
		
		if (operate->rgb_copy)
			operate->rgb_copy(psrc, buffer, w, h, buffer_w, buffer_h);
		
		success = 0;
	} while (0);
	
	if (pdata) {
		free(pdata);
	}
	return success;
}

static int _to_buffer(const char * path, void * buffer, int buffer_w, int buffer_h, int buffer_bits)
{
	int action_type = eACTION_MIN;
	BMP * bmp = NULL;
	do 
	{
		if (!path) {
			printf("bmp_to_screen : path is null!\n");
			break;
		}
		action_type = get_action_type_buffer(path, buffer_bits);
		if (eACTION_MIN == action_type) {
			break;
		}
		bmp = bmp_open(path);
		if (!bmp) {
			break;
		}
		
		to_buffer(bmp, buffer, buffer_w, buffer_h, action_type);
	} while (0);
	
	if (bmp) {
		bmp_close(bmp);
	}
	return 0;
}

//将图片数据输出到buffer中，返回buffer
//path - 图片路径
//buffer_w / buffer_h  buffer中图片数据的宽度/高度
//buffer_bits buffer中图片数据的位数，可填16或24
void * bmp_to_buffer(const char * path, int buffer_w, int buffer_h, int buffer_bits)
{
	void * buffer = NULL;
	int bytespp = 0;
	int linesize = 0;
	int buffer_size = 0;
	do 
	{
		if (!path) {
			printf("bmp_to_buffer : path is null\n");
			break;
		}
		if (buffer_w <= 0 || buffer_h <= 0) {
			printf("bmp_to_buffer : buffer's width <= 0 or height <= 0\n");
			break;
		}
		if (buffer_bits != 16 && buffer_bits != 24) {
			printf("bmp_to_buffer : buffer bits (%d) only support 16 bits or 24 bits\n");
			break;
		}

		bytespp = buffer_bits / 8;
		linesize = (buffer_w * bytespp + 3) & ~3;
		buffer_size = linesize * buffer_h;

		buffer = malloc(buffer_size);
		if (!buffer) {
			printf("bmp_to_buffer : buffer = malloc(%) failed!\n", buffer_size);
			break;
		}

		_to_buffer(path, buffer, buffer_w, buffer_h, buffer_bits);

	} while (0);

	return buffer;
}

#define IMAGE_COUNT 30
#define BUFFER_W	800
#define BUFFER_H	480

//将图片数据输出到buffer中，返回buffer
//要求图片放在/mnt/sdcard/bmp/路径下，
//共30张，名称为 1.bmp, 2.bmp, ... 30.bmp
void * bmp_to_buffer_800x480_24bits(int index)
{
	void * buffer = NULL;
	char path[256] = ROOT_PATH;
	index = index > 0 ? index : -index;
	index = (index % IMAGE_COUNT);
	sprintf(path, "%s/%d.bmp", ROOT_PATH, index + 1);

	buffer = bmp_to_buffer(path, BUFFER_W, BUFFER_H, 24);
	return buffer;
}

void * bmp_to_buffer_800x480_16bits(int index)
{
	void * buffer = NULL;
	char path[256] = ROOT_PATH;
	index = index > 0 ? index : -index;
	index = (index % IMAGE_COUNT);
	sprintf(path, "%s/bmp/%d.bmp", ROOT_PATH, index + 1);
	
	buffer = bmp_to_buffer(path, BUFFER_W, BUFFER_H, 16);
	return buffer;
}

void test_bmp_to_buffer_800x480_16bits(void)
{
	void * buffer = NULL;
	int i;
	char outpath[256] = {0};
	for (i = 0; i < IMAGE_COUNT; i++) {
		buffer = bmp_to_buffer_800x480_16bits(i);
		if (buffer) {
			sprintf(outpath, "%s/%d.bmp", ROOT_PATH, i + 1);
			save_bmp(outpath, BUFFER_W, BUFFER_H, buffer, 16);
			free(buffer);
		}
	}
}

void test_bmp_to_buffer_1(const char * path, const char * outpath, int w, int h, int bpp)
{
	void * buffer = NULL;
	
	buffer = bmp_to_buffer(path, w, h, bpp);
	if (buffer) {
		save_bmp(outpath, w, h, buffer, bpp);
		free(buffer);
	}
}
void test_bmp_to_buffer(void)
{
	test_bmp_to_buffer_1(ROOT_PATH"/bmp/1.bmp", ROOT_PATH"/out.bmp", 800, 480, 24);//16 -> 24
	test_bmp_to_buffer_1(ROOT_PATH"/out.bmp", ROOT_PATH"/out1.bmp", 800, 480, 24);//24 -> 24
	test_bmp_to_buffer_1(ROOT_PATH"/out.bmp", ROOT_PATH"/out2.bmp", 800, 480, 16);//24 -> 16
	test_bmp_to_buffer_1(ROOT_PATH"/bmp/1.bmp", ROOT_PATH"/out3.bmp", 800, 480, 16);//16 -> 16
	
	test_bmp_to_buffer_1(ROOT_PATH"/bmp/1.bmp", ROOT_PATH"/out4.bmp", 400, 400, 16);//buffer 小于 图片
	test_bmp_to_buffer_1(ROOT_PATH"/bmp/1.bmp", ROOT_PATH"/out4.bmp", 900, 500, 16);//buffer 大于 图片

	test_bmp_to_buffer_800x480_16bits();
}



//-------------------------------------------------------------------

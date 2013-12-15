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

struct tagBMP
{
	BMPHEADER header;
	BITMAPINFO info;

	int tpye;//类型
	int forward;//图像行数据是否是正向的
	FILE * hfile;
	int dataSize;//图像数据的大小
	unsigned char * pdata;//图像数据
};

static long filesize(FILE *stream)
{
	long curpos, length;
	curpos = ftell(stream);
	fseek(stream, 0L, SEEK_END);
	length = ftell(stream);
	fseek(stream, curpos, SEEK_SET);
	return length;
}

static int read_head(BMP * bmp)
{
	fseek(bmp->hfile, 0, SEEK_SET);
	fread(bmp->header.bfType, 1, 14, bmp->hfile);
	return 0;
}

static int read_info(BMP * bmp)
{
	fseek(bmp->hfile, 14, SEEK_SET);
	fread(&(bmp->info), 1, sizeof(bmp->info), bmp->hfile);
	return 0;
}

static int check_bmp(BMP * bmp)
{
	int ok = -1;
	int bitCount, w, h;
	do 
	{
		if (bmp->header.bfType[0] != 'B' || bmp->header.bfType[1] != 'M') {
			printf("check_bmp : not bmp file! %c%c\n",
				bmp->header.bfType[0], bmp->header.bfType[1]);
			break;
		}

		w = bmp->info.bmiHeader.biWidth;
		h = bmp->info.bmiHeader.biHeight;
		if (w <= 0 || h == 0) {
			printf("check_bmp : bmp width(%d) or height(%d) is error\n", w, h);
			break;
		}

		bitCount = bmp->info.bmiHeader.biBitCount;
		if (24 == bitCount) {
			if (bmp->info.bmiHeader.biCompression != BI_RGB) {
				printf("sorry! not support 24bits compress (%d) bmp!!\n",
					bmp->info.bmiHeader.biCompression);
				break;
			}
			bmp->tpye = eRGB_888;
		} else if (16 == bitCount) {
			if (    0xF800 != bmp->info.rgb[0]
				||  0x07E0 != bmp->info.rgb[1]
				||  0x001F != bmp->info.rgb[2]) 
			{
				printf("sorry! 16bits only support rgb565 bmp!!\n");
				break;
			}
			bmp->tpye = eRGB_565;
		} else {
			printf("sorry! not support %d bits bmp!!\n", bitCount);
			break;
		}

		if (h < 0) {
			bmp->forward = 1;
			h = -h;
			bmp->info.bmiHeader.biHeight = h;
		}
		bmp->dataSize = filesize(bmp->hfile) - bmp->header.bfOffBits;
		ok = 0;
	} while (0);

	return ok;
}

static int read_data(BMP * bmp)
{
	unsigned char * pdata = NULL;
	pdata = (unsigned char *)malloc(bmp->dataSize);
	if (pdata) {
		fseek(bmp->hfile, bmp->header.bfOffBits, SEEK_SET);
		fread(pdata, 1, bmp->dataSize, bmp->hfile);
		bmp->pdata = pdata;
		return 0;
	} else {
		printf("read_data : malloc(%d)\n", bmp->dataSize);
		return -1;
	}
}

static BMP * _bmp_open(const char * path)
{
	int success = -1;
	BMP * bmp = NULL;
	FILE * hfile = NULL;
	do 
	{
		if (!path) {
			printf("bmp_open : path is null!\n");
			break;
		}
		hfile = fopen(path, "rb");
		if (!hfile) {
			printf("bmp_open : fopen(%s) failed!\n", path);
			break;
		}
		bmp = (BMP *)malloc(sizeof(BMP));
		if (!bmp) {
			printf("bmp_open : malloc(sizeof(BMP)) failed! %d\n", sizeof(BMP));
			break;
		}
		memset(bmp, 0, sizeof(BMP));
		bmp->hfile = hfile;
		
		read_head(bmp);
		read_info(bmp);
		if (check_bmp(bmp) < 0) {
			break;
		}
		success = 0;
	} while (0);
	
	if (success < 0) {//如果失败，则清理现场
		if (bmp) {
			bmp_close(bmp);
			bmp = NULL;
		}
	}
	
	return bmp;
}

int bmp_info(const char * path, ImageInfo * imageinfo)
{
	int success = -1;
	BMP * bmp = NULL;
	FILE * hfile = NULL;
	do 
	{
		if (NULL == imageinfo) {
			printf("bmp_info : imageinfo is null!\n");
			break;
		}
		memset(imageinfo, 0, sizeof(*imageinfo));
		bmp = _bmp_open(path);
		if (NULL == bmp) {
			break;
		}
		imageinfo->bpp = bmp->info.bmiHeader.biBitCount;
		imageinfo->width = bmp->info.bmiHeader.biWidth;
		imageinfo->height = bmp->info.bmiHeader.biHeight;

		printf("%s,%s,%dbpp,%d*%d,forward:%d\n",
			path,
			bmp->tpye == eRGB_888 ? "rgb888" : "rgb565",
			imageinfo->bpp,
			imageinfo->width,
			imageinfo->height,
			bmp->forward);

		success = 0;
	} while (0);
	
	bmp_close(bmp);
	return success;
}

BMP * bmp_open(const char * path)
{
	int success = -1;
	BMP * bmp = NULL;
	FILE * hfile = NULL;
	do 
	{
		bmp = _bmp_open(path);
		if (NULL == bmp) {
			break;
		}
		if (read_data(bmp) < 0) {
			break;
		}
		success = 0;
	} while (0);

	if (bmp) {//关闭文件
		if (bmp->hfile) {
			fclose(bmp->hfile);
			bmp->hfile = NULL;
		}
	}
	
	if (success < 0) {//如果失败，则清理现场
		if (bmp) {
			bmp_close(bmp);
			bmp = NULL;
		}
	}
	
	return bmp;
}

void bmp_close(BMP * bmp)
{
	if (bmp) {
		if (bmp->hfile) {
			fclose(bmp->hfile);
		}
		if (bmp->pdata) {
			free(bmp->pdata);
		}
		free(bmp);
	}
}

int bmp_width(BMP * bmp)
{
	if (bmp)
		return bmp->info.bmiHeader.biWidth;
	return 0;
}
int bmp_height(BMP * bmp)
{
	if (bmp)
		return bmp->info.bmiHeader.biHeight;
	return 0;
}
int bmp_bpp(BMP * bmp)
{
	if (bmp)
		return bmp->info.bmiHeader.biBitCount;
	return 0;
}
void * bmp_data(BMP * bmp)
{
	if (bmp)
		return bmp->pdata;
	return NULL;
}
int bmp_forward(BMP * bmp)
{
	if (bmp)
		return bmp->forward;
	return 0;
}


//-------------------------------------------------------------------

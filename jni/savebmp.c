/********************************************************************
	created:	2012/02/07
	filename: 	savebmp.c
	author:		
	
	purpose:	
*********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "mybmp.h"

static int get_rgb888_header(int w, int h, BMPHEADER * head, BMPINFO * info)
{
	int size = 0;
	if (head && info) {
		size = w * h * 3;
		memset(head, 0, sizeof(* head));
		memset(info, 0, sizeof(* info));
		head->bfType[0] = 'B';
		head->bfType[1] = 'M';
		head->bfOffBits = 14 + sizeof(* info);
		head->bfSize = head->bfOffBits + size;
		head->bfSize = (head->bfSize + 3) & ~3;//windows要求文件大小必须是4的倍数
		size = head->bfSize - head->bfOffBits;
		
		info->biSize = sizeof(BMPINFO);
		info->biWidth = w;
		info->biHeight = -h;
		info->biPlanes = 1;
		info->biBitCount = 24;
		info->biCompression = BI_RGB;
		info->biSizeImage = size;

		printf("rgb888:%dbpp,%d*%d,%d\n", info->biBitCount, w, h, head->bfSize);
	}
	return size;
}

static int get_rgb565_header(int w, int h, BMPHEADER * head, BITMAPINFO * info)
{
	int size = 0;
	if (head && info) {
		size = w * h * 2;
		memset(head, 0, sizeof(* head));
		memset(info, 0, sizeof(* info));
		head->bfType[0] = 'B';
		head->bfType[1] = 'M';
		head->bfOffBits = 14 + sizeof(* info);
		head->bfSize = head->bfOffBits + size;
		head->bfSize = (head->bfSize + 3) & ~3;
		size = head->bfSize - head->bfOffBits;
		
		info->bmiHeader.biSize = sizeof(info->bmiHeader);
		info->bmiHeader.biWidth = w;
		info->bmiHeader.biHeight = -h;
		info->bmiHeader.biPlanes = 1;
		info->bmiHeader.biBitCount = 16;
		info->bmiHeader.biCompression = BI_BITFIELDS;
		info->bmiHeader.biSizeImage = size;

		info->rgb[0] = 0xF800;
		info->rgb[1] = 0x07E0;
		info->rgb[2] = 0x001F;

		printf("rgb565:%dbpp,%d*%d,%d\n", info->bmiHeader.biBitCount, w, h, head->bfSize);
	}
	return size;
}

static int save_bmp_rgb565(FILE * hfile, int w, int h, void * pdata)
{
	int success = -1;
	int size = 0;
	BMPHEADER head;
	BITMAPINFO info;
	
	size = get_rgb565_header(w, h, &head, &info);
	if (size > 0) {
		fwrite(head.bfType, 1, 14, hfile);
		fwrite(&info, 1, sizeof(info), hfile);
		fwrite(pdata, 1, size, hfile);
		success = 0;
	}

	return success;
}

static int save_bmp_rgb888(FILE * hfile, int w, int h, void * pdata)
{
	int success = -1;
	int size = 0;
	BMPHEADER head;
	BMPINFO info;
	
	size = get_rgb888_header(w, h, &head, &info);
	if (size > 0) {
		fwrite(head.bfType, 1, 14, hfile);
		fwrite(&info, 1, sizeof(info), hfile);
		fwrite(pdata, 1, size, hfile);
		success = 0;
	}
	
	return success;
}

int save_bmp(const char * path, int w, int h, void * pdata, int bpp)
{
	int success = -1;
	FILE * hfile = NULL;

	do 
	{
		if (path == NULL || w <= 0 || h <= 0 || pdata == NULL) {
			printf("if (path == NULL || w <= 0 || h <= 0 || pdata == NULL)\n");
			break;
		}

		remove(path);
		hfile = fopen(path, "wb");
		if (hfile == NULL) {
			printf("open(%s) failed!\n", path);
			break;
		}

		printf("%s,", path);
		switch (bpp)
		{
		case 16:
			success = save_bmp_rgb565(hfile, w, h, pdata);
			break;
		case 24:
			success = save_bmp_rgb888(hfile, w, h, pdata);
			break;
		default:
			printf("error: not support format!\n");
			break;
		}
	} while (0);

	if (hfile != NULL)
		fclose(hfile);
	
	return success;
}

//-------------------------------------------------------------------

/********************************************************************
	created:	2012/02/07
	filename: 	screenshot.c
	author:		
	
	purpose:	
*********************************************************************/

#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "./myfb.h"
#include "./mybmp.h"

#ifdef WIN32
//-------------------------------------------------------------------

int screen_shot(const char * path)
{
	int w = 80;
	int h = 20;
	int size = w*h*2;
	char * buf = (char *)malloc(size + 10);
	
	if (buf) {
		memset(buf, 0xff, (size + 10));
		memset(buf, 0xff, size/2);
		memset(buf + size/2, 0x00, size/2);
		save_bmp("./jni/w.bmp", w, h, buf, 16);
		free(buf);
	}
	
	return 0;
}

//-------------------------------------------------------------------
#else

int screen_shot(const char * path)
{
	struct FB * fb = NULL;
	fb = fb_create(1);
	if (fb) {
		save_bmp(path, fb_width(fb), fb_height(fb), fb_bits(fb), fb_bpp(fb));
		fb_destory(fb);
	}
	return 0;
}

#endif //#ifdef WIN32

int shot(int argc, char * argv[])
{
	char path[256] = ROOT_PATH"/shot.bmp";
	if (argc >= 2) {
		strcpy(path, argv[1]);
	}
	return screen_shot(path);
}


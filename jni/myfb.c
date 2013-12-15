/********************************************************************
	created:	2012/02/07
	filename: 	myfb.c
	author:		
	
	purpose:	
*********************************************************************/
#ifndef WIN32
//-------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <linux/fb.h>
#include <linux/kd.h>
#include "./myfb.h"

struct FB {
    unsigned short *bits;
    int fd;
    struct fb_fix_screeninfo fi;
    struct fb_var_screeninfo vi;
};

int fb_bpp(struct FB *fb)
{
	if (fb) {
		return fb->vi.bits_per_pixel;
	}
	return 0;
}

int fb_width(struct FB *fb)
{
	if (fb) {
		return fb->vi.xres;
	}
	return 0;
}

int fb_height(struct FB *fb)
{
	if (fb) {
		return fb->vi.yres;
	}
	return 0;
}

int fb_size(struct FB *fb)
{
	if (fb) {
		unsigned bytespp = fb->vi.bits_per_pixel / 8;
		return (fb->vi.xres * fb->vi.yres * bytespp);
	}
	return 0;
}

int fb_virtual_size(struct FB *fb)
{
	if (fb) {
		unsigned bytespp = fb->vi.bits_per_pixel / 8;
		return (fb->vi.xres_virtual * fb->vi.yres_virtual * bytespp);
	}
	return 0;
}

void * fb_bits(struct FB *fb)
{
	unsigned short * bits = NULL;
	if (fb) {
		int offset, bytespp;
		bytespp = fb->vi.bits_per_pixel / 8;

		/* HACK: for several of our 3d cores a specific alignment
		* is required so the start of the fb may not be an integer number of lines
		* from the base.  As a result we are storing the additional offset in
		* xoffset. This is not the correct usage for xoffset, it should be added
		* to each line, not just once at the beginning */
		offset = fb->vi.xoffset * bytespp;
		offset += fb->vi.xres * fb->vi.yoffset * bytespp;
		bits = fb->bits + offset / sizeof(*fb->bits);
	}
	return bits;
}

void fb_update(struct FB *fb)
{
	if (fb) {
		fb->vi.yoffset = 1;
		ioctl(fb->fd, FBIOPUT_VSCREENINFO, &fb->vi);
		fb->vi.yoffset = 0;
		ioctl(fb->fd, FBIOPUT_VSCREENINFO, &fb->vi);
	}
}

//-------------------------------------------------------------------
static int _fb_open(struct FB *fb, int readonly)
{
	if (NULL == fb) {
		printf("_fb_open : fb is null\n");
		return -1;
	}
	memset(fb, 0, sizeof(struct FB));
	fb->bits = MAP_FAILED;
	fb->fd = open("/dev/graphics/fb0", readonly ? O_RDONLY : O_RDWR);
    if (fb->fd < 0) {
		printf("open(\"/dev/graphics/fb0\") failed!\n");
        return -1;
	}
	return 0;
}

static int _fb_info(struct FB *fb)
{
	int ret = -1;
	do 
	{	
		if (ioctl(fb->fd, FBIOGET_FSCREENINFO, &fb->fi) < 0) {
			printf("FBIOGET_FSCREENINFO failed!\n");
			break;
		}
		if (ioctl(fb->fd, FBIOGET_VSCREENINFO, &fb->vi) < 0) {
			printf("FBIOGET_VSCREENINFO failed!\n");
			break;
		}
		ret = 0;
	} while (0);
	return ret;
}

static int _fb_mmap(struct FB * fb, int readonly)
{
	fb->bits = mmap(0, fb_virtual_size(fb), PROT_READ | (readonly ? 0 : PROT_WRITE), MAP_SHARED, fb->fd, 0);
    if (fb->bits == MAP_FAILED) {
		printf("mmap() failed!\n");
        return -1;
	}
	return 0;
}

static void fb_close(struct FB *fb)
{
	if (fb) {
		if (fb->bits != MAP_FAILED) {
			munmap(fb->bits, fb_virtual_size(fb));
			fb->bits = MAP_FAILED;
		}
		if (fb->fd >= 0) {
			close(fb->fd);
			fb->fd = -1;
		}
	}
}

static int fb_open(struct FB *fb, int readonly)
{
	int ret = -1;
	do 
	{
		if (_fb_open(fb, readonly) < 0) {
			break;
		}
		if (_fb_info(fb) < 0) {
			break;
		}
		if (_fb_mmap(fb, readonly) < 0) {
			break;
		}
		ret = 0;
	} while (0);

	if (ret < 0) {
		fb_close(fb);
	}
    return ret;
}

//-------------------------------------------------------------------

int fb_info(FBinfo * fbinfo)
{
	int ret = -1;
	struct FB sfb;
	struct FB * fb = &sfb;

	//printf("sizeof(struct FB) = %d\n", sizeof(struct FB));
	do 
	{
		if (NULL == fbinfo) {
			printf("fb_info : fbinfo is null!\n");
			break;
		}
		memset(fbinfo, 0, sizeof(*fbinfo));
		if (_fb_open(fb, 1) < 0) {
			break;
		}
		if (_fb_info(fb) < 0) {
			break;
		}
		fbinfo->bpp = fb_bpp(fb);
		fbinfo->width = fb_width(fb);
		fbinfo->height = fb_height(fb);

		printf("fb0,%s,%dbpp,%d*%d\n",
			fbinfo->bpp == 24 ? "rgb888" : "rgb565",
			fbinfo->bpp,
			fbinfo->width,
			fbinfo->height);

		ret = 0;
	} while (0);
	
	fb_close(fb);
    return ret;
}

static struct FB g_fb;
struct FB * fb_create(int readonly)
{
	if (fb_open(&g_fb, readonly) < 0) {
		return NULL;
	}
	return &g_fb;
}

void fb_destory(struct FB *fb)
{
	fb_close(fb);
}

//-------------------------------------------------------------------
#endif//#ifndef WIN32
#include "platform_video.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <gccore.h>

uint8_t *pixelBuffer;
int pixelBufferPitch = 0;

static void* xfb = NULL;
static GXRModeObj* rmode = NULL;

bool initVideo()
{
    VIDEO_Init();

    rmode = VIDEO_GetPreferredMode(NULL);
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);
	VIDEO_SetBlack(false);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (rmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
    
    pixelBufferPitch = GAME_WIDTH * BYTES_PER_PIXEL;
	pixelBuffer = new uint8_t[pixelBufferPitch * GAME_HEIGHT];
	memset(pixelBuffer, 0, pixelBufferPitch * GAME_HEIGHT);

    return true;
}

void shutdownVideo()
{

}

void blitScreen()
{
    VIDEO_WaitVSync();
}
#include "platform_video.h"

#include <libdragon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t *pixelBuffer;
int pixelBufferPitch = 0;

surface_t gameSurface;

bool initVideo(int argc, char **argv)
{
    display_init(RESOLUTION_640x480, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_DISABLED);
    rdpq_init();

    pixelBufferPitch = GAME_WIDTH * BYTES_PER_PIXEL;
	pixelBuffer = new uint8_t[pixelBufferPitch * GAME_HEIGHT];
	memset(pixelBuffer, 0, pixelBufferPitch * GAME_HEIGHT);
    gameSurface = surface_alloc(FMT_RGBA32, GAME_WIDTH, GAME_HEIGHT);

    return true;
}

void shutdownVideo()
{
    rdpq_close();
    display_close();
}

void blitScreen()
{
    surface_t* disp = display_get();
    rdpq_attach_clear(disp, NULL);
    rdpq_set_mode_standard();
    memcpy(pixelBuffer, gameSurface.buffer, pixelBufferPitch * GAME_HEIGHT);
    rdpq_tex_blit(&gameSurface, 0, 0, NULL);
    rdpq_detach_show();
}
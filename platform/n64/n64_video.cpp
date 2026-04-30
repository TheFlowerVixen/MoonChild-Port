#include "platform_video.h"

#include <libdragon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "moonchild/globals.hpp"

uint8_t *pixelBuffer;
int pixelBufferPitch = 0;

bool initVideo(int argc, char **argv)
{
    display_init(RESOLUTION_640x480, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_DISABLED);
    rdpq_init();

    return true;
}

void shutdownVideo()
{
    rdpq_close();
    display_close();
}

void blitScreen()
{

}
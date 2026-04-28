#ifndef _PLATFORM_VIDEO_H
#define _PLATFORM_VIDEO_H

#include <cstdint>

#define GAME_WIDTH 640
#define GAME_HEIGHT 480
#define BYTES_PER_PIXEL 4

extern uint8_t *pixelBuffer;
extern int pixelBufferPitch;

bool initVideo(int argc, char **argv);
void shutdownVideo();

void blitScreen();

#endif
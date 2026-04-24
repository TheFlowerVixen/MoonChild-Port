#include "platform_video.h"

#include <cstdio>

#include <SDL.h>

uint8_t *pixelBuffer;
int pixelBufferPitch = 0;

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
SDL_Texture *frameTexture = nullptr;

bool initVideo()
{
    window = SDL_CreateWindow("moonchild shell", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, GAME_WIDTH, GAME_HEIGHT, 0);
    if (!window)
    {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return false;
    }

    frameTexture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STREAMING,
                        GAME_WIDTH, GAME_HEIGHT);
    if (!frameTexture)
    {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        return false;
    }
    SDL_SetTextureBlendMode(frameTexture, SDL_BLENDMODE_NONE);

    pixelBufferPitch = GAME_WIDTH * BYTES_PER_PIXEL;
    pixelBuffer = new uint8_t[pixelBufferPitch * GAME_HEIGHT];
    memset(pixelBuffer, 0, pixelBufferPitch * GAME_HEIGHT);

    return true;
}

void shutdownVideo()
{
    delete[] pixelBuffer;
    pixelBuffer = nullptr;

    if (frameTexture)
    {
        SDL_DestroyTexture(frameTexture);
        frameTexture = nullptr;
    }
    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}

void blitScreen()
{
    SDL_UpdateTexture(frameTexture, nullptr, pixelBuffer, pixelBufferPitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, frameTexture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}
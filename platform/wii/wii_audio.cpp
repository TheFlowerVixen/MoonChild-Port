#include "platform_audio.h"

bool initAudio()
{
    return true;
}

void shutdownAudio()
{
}

int loadWaveSample(char *path)
{
    return 0;
}

void freeWaveSample(int assetHandle)
{
}

int playWaveOneshot(int assetHandle)
{
    return 0;
}

int playWaveLooping(int assetHandle)
{
    return 0;
}

void stopWaveSample(int assetHandle)
{
}

void volumeWaveSample(int assetHandle, int volume)
{
}

bool panWaveSample(int assetHandle, int left, int right)
{
    return true;
}

bool loadMusicFile(char *path)
{
    return true;
}

void playMusicLooping()
{
}

void stopMusic()
{
}

#include "platform_audio.h"

bool initAudio()
{
    return true;
}

void shutdownAudio()
{
}

TrackInfo getTrackInfo(TrackID id)
{
    TrackInfo info = { nullptr, 0.0f };

    switch (id)
    {
        case TRACK_2_TITLE:
            info.track = "title.mp3";
            info.volume = 0.8f;
            break;
        
        case TRACK_3_WORLD_1:
            info.track = "world1.mp3";
            info.volume = 0.5f;
            break;
        
        case TRACK_4_WORLD_2:
            info.track = "world2.mp3";
            info.volume = 0.5f;
            break;
        
        case TRACK_5_WORLD_3:
            info.track = "world3.mp3";
            info.volume = 0.5f;
            break;
        
        case TRACK_6_WORLD_4:
            info.track = "world4.mp3";
            info.volume = 0.5f;
            break;
        
        case TRACK_7_GAME_OVER:
            info.track = "gameover.mp3";
            info.volume = 1.0f;
            break;
        
        default:
            printf("audio track %d requested", id);
            break;
    }
    
    return info;
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

#include "platform_audio.h"

#include <asndlib.h>
#include <gccore.h>
#include <cstdio>
#include <mp3player.h>

#define log(x) fprintf(stderr, "debug: %s\r\n", x);

// Music (devkitPro mp3 player)

FILE* musicFile;

s32 readMusicFile(void* cb_data, void* readStart, s32 readSize)
{
    return (s32)fread(readStart, readSize, 1, (FILE*)cb_data);
}

// SFX (ASND library)

static lwpq_t audioQueue = LWP_TQUEUE_NULL;
static lwp_t audioThread = LWP_THREAD_NULL;
static bool audioThreadRunning = false;

static void* audioThreadRoutine()
{
    while (audioThreadRunning)
    {
        
    }
}

bool initAudio()
{
    ASND_Init();
    MP3Player_Init();
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
    musicFile = fopen(path, "rb");
    return musicFile;
}

void playMusicLooping(float volume)
{
    MP3Player_Volume(volume * MAX_VOLUME);
    MP3Player_PlayFile(musicFile, readMusicFile, NULL);
}

void stopMusic()
{
    if (MP3Player_IsPlaying() && musicFile)
    {
        MP3Player_Stop();
        fclose(musicFile);
    }
}

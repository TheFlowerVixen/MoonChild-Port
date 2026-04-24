#include "platform_audio.h"

#include <libdragon.h>

// Mixer channel allocation
#define CHANNEL_MUSIC 0
#define CHANNEL_SFX_START 1
#define MAX_CHANNELS 16
#define MAX_SAMPLES 1000

wav64_t* music;
wav64_t* samples[MAX_SAMPLES];

int findFreeChannel()
{
    for (int i = CHANNEL_SFX_START; i < MAX_CHANNELS; i++)
    {
        if (!mixer_ch_playing(i))
            return i;
    }
    return -1;
}

void shutdownAudio()
{
    for (int i = 0; i < MAX_SAMPLES; i++)
    {
        if (samples[i] != NULL)
            wav64_close(samples[i]);
    }
    for (int i = 0; i < MAX_CHANNELS; i++)
    {
        if (mixer_ch_playing(i))
            mixer_ch_stop(i);
    }

    mixer_close();
    audio_close();
}

bool initAudio()
{
    audio_init(48000, 4);
	mixer_init(16);

	wav64_init_compression(3);

	mixer_ch_set_limits(CHANNEL_MUSIC, 0, 128000, 0);

    return true;
}

TrackInfo getTrackInfo(TrackID id)
{
    TrackInfo info = { NULL, 0.0f };

    switch (id)
    {
        case TRACK_2_TITLE:
            info.track = "title.wav64";
            info.volume = 0.8f;
            break;
        
        case TRACK_3_WORLD_1:
            info.track = "world1.wav64";
            info.volume = 0.5f;
            break;
        
        case TRACK_4_WORLD_2:
            info.track = "world2.wav64";
            info.volume = 0.5f;
            break;
        
        case TRACK_5_WORLD_3:
            info.track = "world3.wav64";
            info.volume = 0.5f;
            break;
        
        case TRACK_6_WORLD_4:
            info.track = "world4.wav64";
            info.volume = 0.5f;
            break;
        
        case TRACK_7_GAME_OVER:
            info.track = "gameover.wav64";
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
    for (int i = 0; i < MAX_SAMPLES; i++)
    {
        if (samples[i] == NULL)
        {
            samples[i] = wav64_load(path, NULL);
            return i;
        }
    }
    return -1;
}

void freeWaveSample(int assetHandle)
{
    if (samples[assetHandle] != NULL)
    {
        wav64_close(samples[assetHandle]);
        samples[assetHandle] = NULL;
    }
}

int playWaveOneshot(int assetHandle)
{
    if (samples[assetHandle] != NULL)
    {
        int channel = findFreeChannel();
        if (channel > -1)
        {
            wav64_play(samples[assetHandle], channel);
            return channel;
        }
    }
    return -1;
}

int playWaveLooping(int assetHandle)
{
    if (samples[assetHandle] != NULL)
    {
        int channel = findFreeChannel();
        if (channel > -1)
        {
            wav64_set_loop(samples[assetHandle], true);
            wav64_play(samples[assetHandle], channel);
            return channel;
        }
    }
    return -1;
}

void stopWaveSample(int assetHandle)
{
    if (assetHandle < MAX_CHANNELS && mixer_ch_playing(assetHandle))
        mixer_ch_stop(assetHandle);
}

void volumeWaveSample(int assetHandle, int volume)
{
    if (assetHandle < MAX_CHANNELS)
        mixer_ch_set_vol(assetHandle, volume / MAXVOLUME, volume / MAXVOLUME);
}

bool panWaveSample(int assetHandle, int left, int right)
{
    if (assetHandle < MAX_CHANNELS)
        mixer_ch_set_vol(assetHandle, left / MAXVOLUME, right / MAXVOLUME);
}

bool loadMusicFile(char *path)
{
    if (music == NULL)
    {
        music = wav64_load(path, NULL);
    }
    else
    {
        wav64_close(music);
        music = wav64_load(path, NULL);
    }
    wav64_set_loop(music, true);
    return true;
}

void playMusicLooping()
{
    wav64_play(music, CHANNEL_MUSIC);
}

void stopMusic()
{
    mixer_ch_stop(CHANNEL_MUSIC);
}
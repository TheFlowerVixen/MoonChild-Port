#ifndef _PLATFORM_AUDIO_H
#define _PLATFORM_AUDIO_H

enum TrackID
{
    TRACK_0_UNUSED,
    TRACK_1_UNUSED,
    TRACK_2_TITLE,
    TRACK_3_WORLD_1,
    TRACK_4_WORLD_2,
    TRACK_5_WORLD_3,
    TRACK_6_WORLD_4,
    TRACK_7_GAME_OVER
};

typedef struct TrackInfo_s
{
    const char* track;
    float volume;
} TrackInfo;

bool initAudio(int argc, char **argv);
void shutdownAudio();

TrackInfo getTrackInfo(TrackID id);

// Wave asset handle from loadWaveSample (>= 0). Distinct from channel handles returned by play*.
int loadWaveSample(char *path);
void freeWaveSample(int assetHandle);

// Playback channel handle from SDL_mixer (>= 0), or -1 on failure. Valid until the sample stops
// (oneshot) or until you stop it (looping).
int playWaveOneshot(int assetHandle);
int playWaveLooping(int assetHandle);

void stopWaveSample(int assetHandle);
void volumeWaveSample(int assetHandle, int volume);

// Stereo pan via Mix_SetPanning: each side 0-255 (0 = silent). Center is often left=255, right=255.
bool panWaveSample(int assetHandle, int left, int right);

bool loadMusicFile(char *path);
void playMusicLooping(float volume);
void stopMusic();

void playTestWavSound();
void toggleTestMusic();

#define MAXPANVOLUME (255)
#define MAXVOLUME (128)    //(MIX_MAX_VOLUME)

#endif
#ifndef _PLATFORM_AUDIO_H
#define _PLATFORM_AUDIO_H

bool initAudio();
void shutdownAudio();

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
void playMusicLooping();
void stopMusic();

void playTestWavSound();
void toggleTestMusic();

#define MAXPANVOLUME (255)
#define MAXVOLUME (128)    //(MIX_MAX_VOLUME)

#endif
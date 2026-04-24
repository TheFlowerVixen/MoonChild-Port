#include "platform_audio.h"

// This Audio layer for SDL is hacked togehter and not very well thought out.
// SDL uses channels to set volumes and panning, while thr origibal directx you could set the volume and
// pan on the objects. This is quite the mismatch. So in this h@ck I just 'remember' the channel the sound was playing on
// in order to still be able to set the panning. (Panning is used for some looping sounds like the waterfall)

#include <SDL_mixer.h>

#include <cstdio>
#include <vector>

#define MAXSOUNDOBJECTS (128)
#define MAXSOUNDCHANNELS (32)
namespace {
int soundObjectToChannel[MAXSOUNDOBJECTS]; // becuase SDL only handles panning per channel and not per sound object we store the channel number here

Mix_Music *loadedMusic = nullptr;

std::vector<Mix_Chunk *> waveAssets;

char testWavPath[] = "assets/test.wav";
char testMp3Path[] = "assets/test.mp3";

int testWavAsset = -1;
bool testMusicLoaded = false;

int allocatedChannels = 0;

bool assetHandleOk(int assetHandle) {
  if (assetHandle < 0) {
    return false;
  }
  size_t idx = (size_t)assetHandle;
  if (idx >= waveAssets.size()) {
    return false;
  }
  return waveAssets[idx] != nullptr;
}

void haltChannelsUsingChunk(Mix_Chunk *chunk) {
  if (!chunk || allocatedChannels <= 0) {
    return;
  }
  for (int ch = 0; ch < allocatedChannels; ch++) {
    if (Mix_Playing(ch) && Mix_GetChunk(ch) == chunk) {
      Mix_HaltChannel(ch);
    }
  }
}

}  // namespace

void shutdownAudio() {
  if (loadedMusic) {
    Mix_HaltMusic();
    Mix_FreeMusic(loadedMusic);
    loadedMusic = nullptr;
  }
  for (size_t i = 0; i < waveAssets.size(); i++) {
    if (waveAssets[i]) {
      Mix_FreeChunk(waveAssets[i]);
      waveAssets[i] = nullptr;
    }
  }
  waveAssets.clear();
  testWavAsset = -1;
  Mix_CloseAudio();
  Mix_Quit();
}

bool initAudio() {
  int initFlags = Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG);
  if (initFlags == 0) {
    fprintf(stderr, "Mix_Init: %s\n", Mix_GetError());
  }
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
    fprintf(stderr, "Mix_OpenAudio failed: %s\n", Mix_GetError());
    return false;
  }
  Mix_AllocateChannels(MAXSOUNDCHANNELS);
  allocatedChannels = MAXSOUNDCHANNELS;

  for (int i = 0; i < MAXSOUNDOBJECTS; i++) {
    soundObjectToChannel[i] = -1;
  }
  return true;
}

TrackInfo getTrackInfo(TrackID id)
{
    TrackInfo info = { NULL, 0.0f };

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

int loadWaveSample(char *path) {
  Mix_Chunk *chunk = Mix_LoadWAV(path);
  if (!chunk) {
    fprintf(stderr, "Mix_LoadWAV failed for %s: %s\n", path, Mix_GetError());
    return -1;
  }
  for (size_t i = 0; i < waveAssets.size(); i++) {
    if (waveAssets[i] == nullptr) {
      waveAssets[i] = chunk;
      return (int)i;
    }
  }
  waveAssets.push_back(chunk);
  return (int)(waveAssets.size() - 1);
}

void freeWaveSample(int assetHandle) {
  if (!assetHandleOk(assetHandle)) {
    return;
  }
  Mix_Chunk *chunk = waveAssets[(size_t)assetHandle];
  haltChannelsUsingChunk(chunk);
  Mix_FreeChunk(chunk);
  waveAssets[(size_t)assetHandle] = nullptr;
  if (assetHandle == testWavAsset) {
    testWavAsset = -1;
  }
}

int playWaveOneshot(int assetHandle) {
  if (!assetHandleOk(assetHandle)) {
    return -1;
  }
  Mix_Chunk *chunk = waveAssets[(size_t)assetHandle];
  int channel = Mix_PlayChannel(-1, chunk, 0);
  if (assetHandle >= 0 && assetHandle < MAXSOUNDOBJECTS) {
    soundObjectToChannel[assetHandle] = channel;
  }
  return channel;
}

int playWaveLooping(int assetHandle) {
  if (!assetHandleOk(assetHandle)) {
    return -1;
  }
  Mix_Chunk *chunk = waveAssets[(size_t)assetHandle];
  int channel = Mix_PlayChannel(-1, chunk, -1);
  if (assetHandle >= 0 && assetHandle < MAXSOUNDOBJECTS) {
    soundObjectToChannel[assetHandle] = channel;
  }
  return channel;
}

void stopWaveSample(int assetHandle) {
  if (assetHandle < 0 || assetHandle >= MAXSOUNDOBJECTS) {
    return;
  }
  if (!assetHandleOk(assetHandle)) {
    return;
  }
  int channel = soundObjectToChannel[assetHandle];
  if (channel < 0) {
    return;
  }

  Mix_HaltChannel(channel);
}

void volumeWaveSample(int assetHandle, int volume) {
  if (assetHandle < 0 || assetHandle >= MAXSOUNDOBJECTS) {
    return;
  }
  if (!assetHandleOk(assetHandle)) {
    return;
  }
  if (volume < 0) {
    volume = 0;
  }
  if (volume > MIX_MAX_VOLUME) {
    volume = MIX_MAX_VOLUME;
  }

  int channel = soundObjectToChannel[assetHandle];
  if (channel < 0) {
    return;
  }

  Mix_Volume(channel, volume);
}

bool panWaveSample(int assetHandle, int left, int right) {
  if (assetHandle < 0 || assetHandle >= MAXSOUNDOBJECTS) {
    return false;
  }
  if (!assetHandleOk(assetHandle)) {
    return -1;
  }
  if (left < 0) {
    left = 0;
  }
  if (left > 255) {
    left = 255;
  }
  if (right < 0) {
    right = 0;
  }
  if (right > 255) {
    right = 255;
  }

  int channel = soundObjectToChannel[assetHandle];
  if (channel < 0) {
    return false;
  }

  int result =
      Mix_SetPanning((int)channel, (Uint8)left, (Uint8)right);

  return result != 0;
}

bool loadMusicFile(char *path) {
  if (loadedMusic) {
    Mix_HaltMusic();
    Mix_FreeMusic(loadedMusic);
    loadedMusic = nullptr;
  }
  loadedMusic = Mix_LoadMUS(path);
  if (!loadedMusic) {
    fprintf(stderr, "Mix_LoadMUS failed for %s: %s\n", path, Mix_GetError());
    return false;
  }
  return true;
}

void playMusicLooping() {
  if (!loadedMusic) {
    return;
  }
  Mix_PlayMusic(loadedMusic, -1);
}

void stopMusic() {
  Mix_HaltMusic();
}

void playTestWavSound() {
  if (testWavAsset < 0) {
    testWavAsset = loadWaveSample(testWavPath);
    if (testWavAsset < 0) {
      printf("test wav: still could not load %s\n", testWavPath);
      return;
    }
  }
  int channel = playWaveOneshot(testWavAsset);
  if (channel < 0) {
    printf("test wav: could not play %s (no free channel?)\n", testWavPath);
    return;
  }
  printf("test wav: played %s (channel %d)\n", testWavPath, channel);
}

void toggleTestMusic() {
  if (!testMusicLoaded) {
    if (!loadMusicFile(testMp3Path)) {
      printf("test mp3: still could not load %s\n", testMp3Path);
      return;
    }
    testMusicLoaded = true;
    playMusicLooping();
    printf("test mp3: started %s\n", testMp3Path);
    return;
  }
  if (Mix_PlayingMusic()) {
    stopMusic();
    printf("test mp3: stopped\n");
  } else {
    playMusicLooping();
    printf("test mp3: started\n");
  }
}

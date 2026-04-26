#include "platform_audio.h"

#include <asndlib.h>
#include <gccore.h>
#include <cstdlib>
#include <cstdio>

#include "wii/detail/wave_data.hpp"

static constexpr u32 MAX_SAMPLE_COUNT = 64;

struct SampleData {
public:
    enum {
        PLAYBACK_STOPPED,
        PLAYBACK_RUNNING,
        PLAYBACK_PAUSED
    };

    bool inUse;
    u8 channelCount;
    u32 sampleCount;
    u16 sampleRate;
    s16 *sampleData;

    s32 playbackVoice;
    u8 playbackVolume;
    u8 playbackPanL;
    u8 playbackPanR;
    s32 playbackState;

    void startPlayback(bool loopEnable);
    void stopPlayback(void);

    void setPlaybackPause(bool pause);

    void retuneVoice(void);

    void update(void);

private:
    u32 calcSampleDataSize(void) const {
        return ((sampleCount * sizeof(s16)) + 31) & ~31;
    }

    u8 calcVolumeL(void) const {
        return static_cast<u32>(playbackVolume) * static_cast<u32>(playbackPanL) / 255;
    }
    u8 calcVolumeR(void) const {
        return static_cast<u32>(playbackVolume) * static_cast<u32>(playbackPanL) / 255;
    }
};

class CAudio {
    friend struct SampleData;

public:
    CAudio() {}

    void doInit(void);
    void doShutdown(void);

    void doCalc(void);

    s32 createSample(void *wavData);
    void freeSample(s32 sampleIndex);

    void startSamplePlayback(s32 sampleIndex, u8 volume, bool loop);
    void stopSamplePlayback(s32 sampleIndex);

    void setSamplePlaybackPause(s32 sampleIndex, bool pause);
    void setSamplePlaybackVolume(s32 sampleIndex, u8 volume);
    void setSamplePlaybackPan(s32 sampleIndex, u8 panL, u8 panR);

    void assignBGM(void *wavData);

    void startBGMPlayback(u8 volume);
    void stopBGMPlayback(void);

private:
    SampleData *acquireSampleData(void) {
        for (u32 i = 0; i < MAX_SAMPLE_COUNT; i++) {
            if (!mSampleData[i].inUse) {
                return &mSampleData[i];
            }
        }
        return NULL;
    }

    s32 allocVoice(void) {
        for (u32 i = 0; i < MAX_SND_VOICES; i++) {
            if (!mVoiceInUse[i]) {
                mVoiceInUse[i] = true;
                return i;
            }
        }
        return -1;
    }
    void releaseVoice(s32 voiceIndex) {
        mVoiceInUse[voiceIndex] = false;
    }

    SampleData *findSampleByVoice(s32 voice) {
        for (u32 i = 0; i < MAX_SAMPLE_COUNT; i++) {
            if (mSampleData[i].inUse && (mSampleData[i].playbackVoice == voice)) {
                return &mSampleData[i];
            }
        }
        return NULL;
    }

private:
    SampleData mSampleData[MAX_SAMPLE_COUNT];
    bool mVoiceInUse[MAX_SND_VOICES];

    SampleData mBGMData;
};
static CAudio sAudio;

/*
 * Implementation
 */

void SampleData::startPlayback(bool loopEnable) {
    if (playbackVoice < 0) {
        playbackVoice = sAudio.allocVoice();
        if (playbackVoice < 0) {
            printf("[SampleData::startPlayback] Could not alloc voice\n");
            playbackState = PLAYBACK_STOPPED;
            return;
        }
    }

    s32 asndFormat = (channelCount == 1) ? VOICE_MONO_16BIT : VOICE_STEREO_16BIT;
    u32 sampleDataSize = calcSampleDataSize();

    u8 volumeL = calcVolumeL();
    u8 volumeR = calcVolumeR();

    s32 result;
    if (loopEnable) {
        result = ASND_SetInfiniteVoice(
            playbackVoice, asndFormat, sampleRate, 0,
            sampleData, sampleDataSize, volumeL, volumeR
        );
    }
    else {
        result = ASND_SetVoice(
            playbackVoice, asndFormat, sampleRate, 0,
            sampleData, sampleDataSize, volumeL, volumeR,
            NULL
        );
    }

    if (result != SND_OK) {
        printf("[SampleData::startPlayback] ASND voice start failure (#%d)\n", playbackVoice);
        playbackState = PLAYBACK_STOPPED;
        return;
    }

    playbackState = PLAYBACK_RUNNING;

#if 0
    printf("[SampleData::startPlayback] Playback of voice #%d has started\n", playbackVoice);
    printf("[SampleData::startPlayback] sampleRate=%d\n", sampleRate);
    printf("[SampleData::startPlayback] asndFormat=%d\n", asndFormat);
    printf("[SampleData::startPlayback] sampleData=%p\n", sampleData);
    printf("[SampleData::startPlayback] sampleDataSize=%d\n", sampleDataSize);
    printf("[SampleData::startPlayback] volumeL=%d\n", volumeL);
    printf("[SampleData::startPlayback] volumeR=%d\n", volumeR);
#endif
}

void SampleData::stopPlayback(void) {
    if (playbackState == PLAYBACK_STOPPED) {
        return;
    }

    // printf("ASND_StopVoice(%d)\n", playbackVoice);
    ASND_StopVoice(playbackVoice);
    sAudio.releaseVoice(playbackVoice);
    playbackVoice = -1;

    playbackState = PLAYBACK_STOPPED;
}

void SampleData::setPlaybackPause(bool pause) {
    if ((playbackState == PLAYBACK_RUNNING) || (playbackState == PLAYBACK_PAUSED)) {
        ASND_PauseVoice(playbackVoice, pause ? TRUE : FALSE);
        playbackState = pause ? PLAYBACK_PAUSED : PLAYBACK_RUNNING;
    }
    else {
        printf("[SampleData::setPlaybackPause] Cannot set pause when playback is stopped\n");
    }
}

void SampleData::retuneVoice(void) {
    if (playbackVoice >= 0) {
        ASND_ChangeVolumeVoice(playbackVoice, calcVolumeL(), calcVolumeR());
    }
}

void SampleData::update(void) {
    if (playbackVoice >= 0) {
        if (ASND_StatusVoice(playbackVoice) == SND_UNUSED) {
            // printf("ASND_StatusVoice(%d) is SND_UNUSED\n", playbackVoice);
            sAudio.releaseVoice(playbackVoice);
            playbackVoice = -1;

            playbackState = PLAYBACK_STOPPED;
        }
    }
}

static void asndCallback(void) {
    sAudio.doCalc();
}

void CAudio::doInit(void) {
    ASND_Init();
    ASND_Pause(FALSE);

    for (u32 i = 0; i < MAX_SAMPLE_COUNT; i++) {
        mSampleData[i].inUse = false;
    }
    mBGMData.inUse = false;

    ASND_SetCallback(asndCallback);
}

void CAudio::doShutdown(void) {
    ASND_End();
}

void CAudio::doCalc(void) {
    for (u32 i = 0; i < MAX_SAMPLE_COUNT; i++) {
        if (mSampleData[i].inUse) {
            mSampleData[i].update();
        }
    }
}

s32 CAudio::createSample(void *wavData) {
    SampleData *sampleData = acquireSampleData();
    if (sampleData == NULL) {
        printf("[CAudio::createSample] Sample Data buffer is full!\n");
        return -1;
    }

    sampleData->inUse = true;
    sampleData->channelCount = WavGetChannelCount(wavData);
    sampleData->sampleCount = WavGetSampleCount(wavData);
    sampleData->sampleRate = WavGetSampleRate(wavData);
    sampleData->sampleData = WavGetPCM16(wavData); // Dynamically allocated

    sampleData->playbackVoice = -1;
    sampleData->playbackVolume = 255;
    sampleData->playbackPanL = 255;
    sampleData->playbackPanR = 255;
    sampleData->playbackState = SampleData::PLAYBACK_STOPPED;

    return (sampleData - mSampleData);
}

void CAudio::freeSample(s32 sampleIndex) {
    if ((sampleIndex < 0) || (sampleIndex >= (s32)MAX_SAMPLE_COUNT)) {
        return;
    }
    SampleData *sampleData = &mSampleData[sampleIndex];

    if (sampleData->playbackState != SampleData::PLAYBACK_STOPPED) {
        sampleData->stopPlayback();
    }

    sampleData->inUse = false;
    free(sampleData->sampleData);
}

void CAudio::startSamplePlayback(s32 sampleIndex, u8 volume, bool loop) {
    if ((sampleIndex < 0) || (sampleIndex >= (s32)MAX_SAMPLE_COUNT)) {
        return;
    }
    SampleData *sampleData = &mSampleData[sampleIndex];

    sampleData->playbackVolume = volume;
    sampleData->playbackPanL = 255;
    sampleData->playbackPanR = 255;
    sampleData->startPlayback(loop);
}

void CAudio::stopSamplePlayback(s32 sampleIndex) {
    if ((sampleIndex < 0) || (sampleIndex >= (s32)MAX_SAMPLE_COUNT)) {
        return;
    }
    SampleData *sampleData = &mSampleData[sampleIndex];

    sampleData->stopPlayback();
}

void CAudio::setSamplePlaybackPause(s32 sampleIndex, bool pause) {
    if ((sampleIndex < 0) || (sampleIndex >= (s32)MAX_SAMPLE_COUNT)) {
        return;
    }
    SampleData *sampleData = &mSampleData[sampleIndex];

    sampleData->setPlaybackPause(pause);
}

void CAudio::setSamplePlaybackVolume(s32 sampleIndex, u8 volume) {
    if ((sampleIndex < 0) || (sampleIndex >= (s32)MAX_SAMPLE_COUNT)) {
        return;
    }
    SampleData *sampleData = &mSampleData[sampleIndex];

    sampleData->playbackVolume = volume;
    sampleData->retuneVoice();
}

void CAudio::setSamplePlaybackPan(s32 sampleIndex, u8 panL, u8 panR) {
    if ((sampleIndex < 0) || (sampleIndex >= (s32)MAX_SAMPLE_COUNT)) {
        return;
    }
    SampleData *sampleData = &mSampleData[sampleIndex];

    sampleData->playbackPanL = panL;
    sampleData->playbackPanR = panR;
    sampleData->retuneVoice();
}

void CAudio::assignBGM(void *wavData) {
    if (mBGMData.inUse) {
        stopBGMPlayback();

        free(mBGMData.sampleData);
        // mBGMData.inUse = false;
    }

    if (wavData == NULL) {
        mBGMData.inUse = false;
        return;
    }

    mBGMData.inUse = true;
    mBGMData.channelCount = WavGetChannelCount(wavData);
    mBGMData.sampleCount = WavGetSampleCount(wavData);
    mBGMData.sampleRate = WavGetSampleRate(wavData);
    mBGMData.sampleData = WavGetPCM16(wavData); // Dynamically allocated

    mBGMData.playbackVoice = -1;
    mBGMData.playbackVolume = 255;
    mBGMData.playbackPanL = 255;
    mBGMData.playbackPanR = 255;
    mBGMData.playbackState = SampleData::PLAYBACK_STOPPED;
}

void CAudio::startBGMPlayback(u8 volume) {
    if (!mBGMData.inUse) {
        return;
    }

    if (mBGMData.playbackState != SampleData::PLAYBACK_STOPPED) {
        stopBGMPlayback();
    }

    mBGMData.playbackVoice = allocVoice();
    mBGMData.playbackVolume = volume;
    mBGMData.playbackPanL = 255;
    mBGMData.playbackPanR = 255;
    mBGMData.startPlayback(true);
}

void CAudio::stopBGMPlayback(void) {
    if (!mBGMData.inUse) {
        return;
    }

    mBGMData.stopPlayback();
}

/*
 * Public interface
 */

bool initAudio() {
    sAudio.doInit();
    return true;
}

void shutdownAudio() {
    sAudio.doShutdown();
}

TrackInfo getTrackInfo(TrackID id) {
    TrackInfo info = { NULL, 0.0f };

    switch (id) {
    case TRACK_2_TITLE:
        info.track = "title.wav";
        info.volume = 0.8f;
        break;
    
    case TRACK_3_WORLD_1:
        info.track = "world1.wav";
        info.volume = 0.5f;
        break;
    
    case TRACK_4_WORLD_2:
        info.track = "world2.wav";
        info.volume = 0.5f;
        break;
    
    case TRACK_5_WORLD_3:
        info.track = "world3.wav";
        info.volume = 0.5f;
        break;
    
    case TRACK_6_WORLD_4:
        info.track = "world4.wav";
        info.volume = 0.5f;
        break;
    
    case TRACK_7_GAME_OVER:
        info.track = "gameover.wav";
        info.volume = 1.0f;
        break;
    
    default:
        printf("[getTrackInfo] Audio track %d is invalid\n", id);
        break;
    }
    
    return info;
}

static void *loadWaveFile(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    u32 fileSize = ftell(file);
    rewind(file);

    void *wavBuffer = new u8[fileSize];
    if (wavBuffer == NULL) {
        fclose(file);
        return NULL;
    }
    
    fread(wavBuffer, fileSize, 1, file);    
    fclose(file);

    WavPreprocess(wavBuffer);

    return wavBuffer;
}

int loadWaveSample(char *path) {
    void *wavBuffer = loadWaveFile(path);
    if (wavBuffer == NULL) {
        return -1;
    }

    s32 sampleIndex = sAudio.createSample(wavBuffer);

    delete[] static_cast<u8 *>(wavBuffer);

    return sampleIndex;
}

void freeWaveSample(int assetHandle) {
    sAudio.freeSample(assetHandle);
}

int playWaveOneshot(int assetHandle) {
    sAudio.startSamplePlayback(assetHandle, 255, false);
    return assetHandle;
}

int playWaveLooping(int assetHandle) {
    sAudio.startSamplePlayback(assetHandle, 255, true);
    return assetHandle;
}

void stopWaveSample(int assetHandle) {
    sAudio.stopSamplePlayback(assetHandle);
}

void volumeWaveSample(int assetHandle, int volume) {
    sAudio.setSamplePlaybackVolume(assetHandle, (u8)(volume * (255.0f / (float)MAXVOLUME)));
}

bool panWaveSample(int assetHandle, int left, int right) {
    sAudio.setSamplePlaybackPan(assetHandle, left, right);
    return true;
}

bool loadMusicFile(char *path) {
    if (path == NULL) {
        //printf("loadMusicFile(NULL)\n");
        return false;
    }

    sAudio.assignBGM(NULL);

    void *wavBuffer = loadWaveFile(path);
    if (wavBuffer == NULL) {
        printf("loadMusicFile(\"%s\"): failed to load WAV asset\n", path);
        return false;
    }

    sAudio.assignBGM(wavBuffer);

    delete[] static_cast<u8 *>(wavBuffer);

    return true;
}
void playMusicLooping(float volume) {
    sAudio.startBGMPlayback(volume * 255.0f);
}
void stopMusic() {
    sAudio.stopBGMPlayback();
}

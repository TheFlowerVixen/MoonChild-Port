#include "platform_movie_player.h"

#include <libdragon.h>

const char* getMoviePath(MovieID id)
{
    switch (id)
    {
        case MOVIE_INTRO:
            return "intro";
        
        case MOVIE_BUMPER_1_2:
            return "bumper12";
        
        case MOVIE_BUMPER_2_3:
            return "bumper23";
        
        case MOVIE_BUMPER_3_4:
            return "bumper34";
        
        case MOVIE_EXTRO:
            return "extro";
        
        case MOVIE_GAME_OVER:
            return "gameover";
        
        default:
            return nullptr;
    }
}

struct MoviePlayer::Impl
{
    video_t* video;
    wav64_t* audio;
    yuv_blitter_t* yuvBlitter;
    video_info_t* videoInfo;
};

MoviePlayer::MoviePlayer()
{
    impl = new Impl();
    playing = false;
    streamEnded = false;
    movieDurationSec = 0.0;
    movieStartTicksMs = 0;
    lastPresentedPtsSec = -1.0;
}

MoviePlayer::~MoviePlayer()
{
    clearState();
    delete impl;
    impl = nullptr;
}

void MoviePlayer::clearState()
{
    if (!impl)
        return;
    
    if (impl->yuvBlitter)
        yuv_blitter_free(impl->yuvBlitter);
    
    if (impl->videoInfo)
        free(impl->videoInfo);
    
    if (impl->audio)
        wav64_close(impl->audio);
    
    if (impl->video)
        video_close(impl->video);
}

bool MoviePlayer::playFile(char *filePath, MovieDoneCallback callback, void *userData)
{
    clearState();

    doneCallback = callback;
    doneUserData = userData;
    
    if (!filePath)
        return false;
    
    // Skip video for now; not finished
    invokeDoneCallback(true);

    /*
    static char videoFilePath[4096];
    static char audioFilePath[4096];

    snprintf(videoFilePath, sizeof(videoFilePath), "%s.h264", filePath);
    snprintf(audioFilePath, sizeof(audioFilePath), "%s.wav64", filePath);

    impl->video = video_open(videoFilePath, NULL);
    if (!impl->video)
        return false;
    
    impl->videoInfo = &video_get_info(impl->video);
    
    yuv_init();
    impl->yuvBlitter = &yuv_blitter_new(impl->videoInfo->width, impl->videoInfo->height, 0, 0, NULL, &impl->videoInfo->colorspace);

    //display_set_fps_limit(impl->videoInfo->framerate);

    impl->audio = wav64_load(audioFilePath, NULL);
    if (!impl->audio)
        return false;
    */

    return true;
}

void MoviePlayer::invokeDoneCallback(bool naturalEnd)
{
    MovieDoneCallback callback = doneCallback;
    void *userData = doneUserData;
    doneCallback = nullptr;
    doneUserData = nullptr;
    if (callback)
        callback(naturalEnd, userData);
}

void MoviePlayer::stop(bool naturalEnd)
{
    if (!playing)
    {
        clearState();
        return;
    }

    clearState();
    invokeDoneCallback(naturalEnd);
}

bool MoviePlayer::isPlaying()
{
    return playing;
}

bool MoviePlayer::decodeNextFrame()
{
    if (!playing)
        return false;
    
    return true;
}

void MoviePlayer::presentCurrentFrame(uint8_t *pixels, int width, int height, int pitch)
{
}

void MoviePlayer::update(uint8_t *pixels, int width, int height, int pitch)
{
    if (!playing)
        return;
}

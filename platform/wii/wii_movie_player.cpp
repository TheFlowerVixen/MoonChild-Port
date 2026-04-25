#include "platform_movie_player.h"

const char* getMoviePath(MovieID id)
{
    switch (id)
    {
        case MOVIE_INTRO:
            return "intro.mp4";
        
        case MOVIE_BUMPER_1_2:
            return "bumper12.mp4";
        
        case MOVIE_BUMPER_2_3:
            return "bumper23.mp4";
        
        case MOVIE_BUMPER_3_4:
            return "bumper34.mp4";
        
        case MOVIE_EXTRO:
            return "extro.mp4";
        
        case MOVIE_GAME_OVER:
            return "gameover.mp4";
        
        default:
            return nullptr;
    }
}

struct MoviePlayer::Impl
{

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
}

bool MoviePlayer::playFile(char *filePath, MovieDoneCallback callback, void *userData)
{
    clearState();

    doneCallback = callback;
    doneUserData = userData;
    
    if (!filePath)
        return false;
    
    // Skip movie for now; not implemented
    invokeDoneCallback(true);

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

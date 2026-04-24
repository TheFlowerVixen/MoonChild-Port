#include "platform_movie_player.h"

MoviePlayer::MoviePlayer()
{
    playing = false;
    streamEnded = false;
    movieDurationSec = 0.0;
    movieStartTicksMs = 0;
    lastPresentedPtsSec = -1.0;
}

MoviePlayer::~MoviePlayer()
{
    clearState();
}

void MoviePlayer::clearState()
{
}

bool MoviePlayer::playFile(char *filePath, MovieDoneCallback callback, void *userData)
{
    return true;
}

void MoviePlayer::invokeDoneCallback(bool naturalEnd)
{
}

void MoviePlayer::stop(bool naturalEnd)
{
}

bool MoviePlayer::isPlaying()
{
    return false;
}

bool MoviePlayer::decodeNextFrame()
{
    return false;
}

void MoviePlayer::presentCurrentFrame(uint8_t *pixels, int width, int height, int pitch)
{
}

void MoviePlayer::update(uint8_t *pixels, int width, int height, int pitch)
{
}

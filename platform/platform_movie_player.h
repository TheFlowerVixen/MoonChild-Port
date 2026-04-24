#ifndef _PLATFORM_MOVIE_PLAYER_H
#define _PLATFORM_MOVIE_PLAYER_H

#include <cstdint>

enum MovieID
{
  MOVIE_INTRO,
  MOVIE_BUMPER_1_2,
  MOVIE_BUMPER_2_3,
  MOVIE_BUMPER_3_4,
  MOVIE_EXTRO,
  MOVIE_GAME_OVER
};

const char* getMoviePath(MovieID id);

typedef void (*MovieDoneCallback)(bool naturalEnd, void *userData);

class MoviePlayer {
 public:
  MoviePlayer();
  ~MoviePlayer();

  bool playFile(char *filePath, MovieDoneCallback doneCallback, void *userData);
  void stop(bool naturalEnd);
  bool isPlaying();
  void update(uint8_t *pixels, int width, int height, int pitch);

 private:
  void clearState();
  bool decodeNextFrame();
  void presentCurrentFrame(uint8_t *pixels, int width, int height, int pitch);
  void invokeDoneCallback(bool naturalEnd);

  struct Impl;
  Impl *impl;
  MovieDoneCallback doneCallback;
  void *doneUserData;
  bool playing;
  bool streamEnded;
  double movieDurationSec;
  uint64_t movieStartTicksMs;
  double lastPresentedPtsSec;
};

#endif
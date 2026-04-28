#include "platform_system.h"
#include "platform_movie_player.h"
#include <SDL.h>
#include <cstdio>
#include <cstdint>
#include <framewrk/frm_int.hpp>
#include "moonchild/mc.hpp"
#include "moonchild/globals.hpp"
#include "moonchild/prefs.hpp"

namespace {

char prefOrg[] = "moonchild";
char prefApp[] = "moonchild_shell";

}  // namespace

MoviePlayer* moviePlayer;

uint64_t performanceFrequency = 0;
uint64_t tickIntervalTicks = 0;
uint64_t nextTickTime = 0;
int ticksPerSecond = 60;

void movieDoneCallback(bool naturalEnd, void *userData)
{
	(void)userData;
	movieFinishedNaturally = naturalEnd;
	movieDoneSignal = true;
}

bool initSystem()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0)
	{
		fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
		return false;
	}
	performanceFrequency = SDL_GetPerformanceFrequency();
	tickIntervalTicks = performanceFrequency / (uint64_t)ticksPerSecond;
	nextTickTime = SDL_GetPerformanceCounter();

	moviePlayer = new MoviePlayer();
	
	return true;
}

void shutdownSystem()
{
	SDL_Quit();
}

void keyDown(int);
void keyUp(int);

bool pollEvents()
{
    SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT)
			return true;
		if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
		{
			if (moviePlayer && moviePlayer->isPlaying())
			{
				moviePlayer->stop(false);
				continue;
			}
			switch (e.key.keysym.scancode)
			{
				case SDL_SCANCODE_UP:
					keyDown(SDL_SCANCODE_UP);
					break;
				case SDL_SCANCODE_DOWN:
					keyDown(SDL_SCANCODE_DOWN);
					break;
				case SDL_SCANCODE_LEFT:
					keyDown(SDL_SCANCODE_LEFT);
					break;
				case SDL_SCANCODE_RIGHT:
					keyDown(SDL_SCANCODE_RIGHT);
					break;
				case SDL_SCANCODE_SPACE:
					keyDown(SDL_SCANCODE_SPACE);
					break;
				case SDL_SCANCODE_ESCAPE:
					keyDown(SDL_SCANCODE_ESCAPE);
					break;
				case SDL_SCANCODE_E:
					keyDown(SDL_SCANCODE_E);
					break;
				case SDL_SCANCODE_P:
					keyDown(SDL_SCANCODE_P);
					break;
				case SDL_SCANCODE_M:
				{
					char introMoviePath[] = "assets/movies/intro.mp4";
					bool movieStarted = moviePlayer->playFile(introMoviePath, movieDoneCallback, nullptr);
					if (!movieStarted)
						printf("movie: failed to play %s\n", introMoviePath);
					else
						printf("movie: playing %s\n", introMoviePath);
					break;
				}
				default:
					break;
			}
		}
		if (moviePlayer && moviePlayer->isPlaying())
		{
			if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_FINGERDOWN)
			{
				moviePlayer->stop(false);
				continue;
			}
		}
		if (e.type == SDL_KEYUP && e.key.repeat == 0)
		{
			switch (e.key.keysym.scancode)
			{
				case SDL_SCANCODE_UP:
					keyUp(SDL_SCANCODE_UP);
					break;
				case SDL_SCANCODE_DOWN:
					keyUp(SDL_SCANCODE_DOWN);
					break;
				case SDL_SCANCODE_LEFT:
					keyUp(SDL_SCANCODE_LEFT);
					break;
				case SDL_SCANCODE_RIGHT:
					keyUp(SDL_SCANCODE_RIGHT);
					break;
				case SDL_SCANCODE_SPACE:
					keyUp(SDL_SCANCODE_SPACE);
					break;
				case SDL_SCANCODE_ESCAPE:
					keyUp(SDL_SCANCODE_ESCAPE);
					break;
				case SDL_SCANCODE_E:
					keyUp(SDL_SCANCODE_E);
					break;
				case SDL_SCANCODE_P:
					keyUp(SDL_SCANCODE_P);
					break;
				default:
					break;
			}
		}
	}

	return false;
}

void syncMouse()
{
	static int prevLeft = 0;
	int mx = 0;
	int my = 0;
	uint32_t buttons = SDL_GetMouseState(&mx, &my);
	g_MouseXCurrent = mx;
	g_MouseYCurrent = my;
	int left = (buttons & SDL_BUTTON_LMASK) ? 1 : 0;
	g_MouseFlg = left;
	g_MouseActualFlg = left;
	if (left != 0 && prevLeft == 0)
	{
		g_MouseXDown = mx;
		g_MouseYDown = my;
	}
	prevLeft = left;
}

void preSync()
{
	for (;;)
	{
		uint64_t now = SDL_GetPerformanceCounter();
		if (now >= nextTickTime)
			break;

		uint64_t remaining = nextTickTime - now;
		uint64_t remainingNs = (remaining * 1000000000ULL) / performanceFrequency;
		if (remainingNs > 2000000ULL)
			SDL_Delay(1);
	}
}

void postSync()
{
	uint64_t now = SDL_GetPerformanceCounter();
	nextTickTime += tickIntervalTicks;
	if (now > nextTickTime + tickIntervalTicks)
		nextTickTime = now + tickIntervalTicks;
}

// Called by the game to get the full path to a file
char *FullPath(char *filename) {
  static char buffer[4096];
  if (!filename) {
    return nullptr;
  }
  char *base = SDL_GetBasePath();
  if (base) {
    snprintf(buffer, sizeof(buffer), "%sassets/moonchild/%s", base, filename);
    SDL_free(base);
  } else {
    snprintf(buffer, sizeof(buffer), "assets/moonchild/%s", filename);
  }
  return buffer;
}

// Called by the game to get the full path to an audio file
char *FullAudioPath(char *filename) {
  static char buffer[4096];
  if (!filename) {
    return nullptr;
  }
  char *base = SDL_GetBasePath();
  if (base) {
    snprintf(buffer, sizeof(buffer), "%sassets/audio/%s", base, filename);
    SDL_free(base);
  } else {
    snprintf(buffer, sizeof(buffer), "assets/audio/%s", filename);
  }
  return buffer;
}

char *FullMoviePath(char *filename) {
  static char buffer[4096];
  if (!filename) {
    return nullptr;
  }
  char *base = SDL_GetBasePath();
  if (base) {
    snprintf(buffer, sizeof(buffer), "%sassets/movies/%s", base, filename);
    SDL_free(base);
  } else {
    snprintf(buffer, sizeof(buffer), "assets/movies/%s", filename);
  }
  return buffer;
}

// Called by the game to get the full path to a writable file (Only hiscore file)
char *FullWritablePath(char *filename) {
  static char buffer[4096];
  if (!filename) {
    return nullptr;
  }
  char *pref = SDL_GetPrefPath(prefOrg, prefApp);
  if (pref) {
    snprintf(buffer, sizeof(buffer), "%smoonchild/%s", pref, filename);
    SDL_free(pref);
  } else {
    snprintf(buffer, sizeof(buffer), "./moonchild/%s", filename);
  }
  return buffer;
}

// Internal method (only used here) to load a TGA file
typedef unsigned char BYTE;
unsigned short*LoadTGA(char *FileName)
{
	char logbuf[100];
	snprintf(logbuf, sizeof(logbuf), "loading: %s\n", FileName);
//	LOG(logbuf);
    
	// load targa file
	BYTE* tgabuff = new BYTE[20];
	bool OK = true;
  FILE *tga = fopen( FullPath(FileName), "rb" );
  if (!tga) return 0;
  fread(tgabuff, 20, 1, tga);
  fclose(tga);
	// gzFile tga = gzopen( FullPath(FileName), "rb" );
	// if (!tga) return 0; 
	// gzread(tga, tgabuff, 20);
	// gzclose( tga );
    
//	LOG("open succeeded\n");
    
	int TgaIDLen;
	int TgaCMapType;
	int TgaImgType;
	int TgaCMapOrig;
	int TgaCMapLen;
	int TgaCMapSize;
	int TgaXPos;
	int TgaYPos;
	int TgaWidth;
	int TgaHeight;
	int TgaPixSize;
	TgaIDLen		= *tgabuff;
	TgaCMapType	= *(tgabuff + 1);
	TgaImgType	= *(tgabuff + 2);
	TgaCMapOrig	= *(tgabuff + 3) + 256 * *(tgabuff + 4);
	TgaCMapLen	= *(tgabuff + 5) + 256 * *(tgabuff + 6);
	TgaCMapSize	= *(tgabuff + 7);
	TgaXPos		= *(tgabuff + 8) + 256 * *(tgabuff + 9);
	TgaYPos		= *(tgabuff + 10) + 256 * *(tgabuff + 11);
	TgaWidth	= *(tgabuff + 12) + 256 * *(tgabuff + 13);
	TgaHeight	= *(tgabuff + 14) + 256 * *(tgabuff + 15);
	TgaPixSize	= *(tgabuff + 16);
	delete [] tgabuff;
    
	int w,h;
	w = TgaWidth;
	h = TgaHeight;
	unsigned short *dest;
	int size = w * 4 * h + 20;
	tgabuff = new BYTE[size];
	dest = new unsigned short[w*h];  // hier komt uitgepakte plaatje
  
  // replace the gzip loading by normal loading
	// tga = gzopen( FullPath(FileName), "rb" );
	// if (!tga)
	// {
	// 	delete [] tgabuff;
	// 	delete [] dest;
	// 	return 0;
	// }
	// int read = gzread( tga, tgabuff, size );
	// gzclose( tga );
  tga = fopen( FullPath(FileName), "rb" );
	if (!tga)
	{
	 	delete [] tgabuff;
	 	delete [] dest;
	 	return 0;
	}
  fread(tgabuff, size, 1, tga);
  fclose(tga);
	
	if (TgaImgType == 1)
	{
		// Palettized image
		unsigned short* pal = new unsigned short[256];
		for ( int i = 0; i < 256; i++ )
		{
			int b = *(tgabuff + 18 + i * 3);
			int g = *(tgabuff + 18 + i * 3 + 1);
			int r = *(tgabuff + 18 + i * 3 + 2);
			pal[i] = (unsigned short)(((r >> 3) << 11) + ((g >> 2) << 5) + (b >> 3));
		}
		unsigned char* src = tgabuff + 18 + 768 + (h - 1) * w;
		unsigned short* dst = (unsigned short*)dest;
		for ( int y = 0; y < h; y++ )
		{
			for ( int x = 0; x < w; x++ )
			{
				int idx = *(src + x);
				*(dst + x) = pal[idx];
			}
			dst += w;
			src -= w;
		}
	}
	else
	{
		// Store the data at the specified target address
		unsigned char* src = (tgabuff + 18) + (((h - 1) * w)*4);
		unsigned short* dst = (unsigned short*)dest;
		for ( int i = 0; i < h; i++ )
		{
			for ( int x = 0; x < w; x++ )
			{
				int r,g,b,a;
				unsigned short rgba;
				b= *src++;
				g= *src++;
				r= *src++;
				a= *src++;
				rgba = ((r>>3)<<11)+((g>>2)<<5)+(b>>3);
				*(dst + x) = rgba; //*(src + x);
			}
			dst += w;
			src -= (w*8);
		}
	}
    
	delete [] tgabuff;
    
//	LOG("tga success\n");
	return dest;
}

// Called by the game to show a picture (tga)
void ShowPicture(char *FileName)
{
  unsigned short *TempPic;
	TempPic   = LoadTGA(FileName);
    
	video->DrawTempPic();
    
	delete [] TempPic;
}

void keyDown(int key) {
  switch (key) {
    case SDL_SCANCODE_UP:  // move up
      framework_EventHandle(FW_KEYDOWN,(int) prefs->upkey);
      break;  // move up
    case SDL_SCANCODE_DOWN:  // move down
      framework_EventHandle(FW_KEYDOWN,(int) prefs->downkey);
      break;
    case SDL_SCANCODE_LEFT:  // move left
      framework_EventHandle(FW_KEYDOWN,(int) prefs->leftkey);
      break;
    case SDL_SCANCODE_RIGHT:  // move right
      framework_EventHandle(FW_KEYDOWN,(int) prefs->rightkey);
      break;
	case SDL_SCANCODE_LCTRL:
	  framework_EventHandle(FW_KEYDOWN,(int) prefs->usekey);
	  break;
    case SDL_SCANCODE_SPACE:   // fire or switch
      framework_EventHandle(FW_KEYDOWN,(int) prefs->shootkey);
	  framework_EventHandle(FW_KEYDOWN,(int) prefs->jumpkey);
      break;
    case SDL_SCANCODE_ESCAPE:  // break out of level
      framework_EventHandle(FW_KEYDOWN,(int) 'Q');
      break;
    case SDL_SCANCODE_E:  // If editor is compiled (define in mc.cpp) then this is the key to show it
      framework_EventHandle(FW_KEYDOWN,(int) 'E');
      break;
    case SDL_SCANCODE_P:  // if editor is compiled (define in mc.cpp) then this is the key to show patterns(tiles)
      framework_EventHandle(FW_KEYDOWN,(int) 'P');
      break;
    default:
      break;
  }
}

void keyUp(int key) {
  switch (key) {
    case SDL_SCANCODE_UP:  // move up
      framework_EventHandle(FW_KEYUP,(int) prefs->upkey);
      break;  // move up
    case SDL_SCANCODE_DOWN:  // move down
      framework_EventHandle(FW_KEYUP,(int) prefs->downkey);
      break;
    case SDL_SCANCODE_LEFT:  // move left
      framework_EventHandle(FW_KEYUP,(int) prefs->leftkey);
      break;
    case SDL_SCANCODE_RIGHT:  // move right
      framework_EventHandle(FW_KEYUP,(int) prefs->rightkey);
      break;
	case SDL_SCANCODE_LCTRL:
	  framework_EventHandle(FW_KEYUP,(int) prefs->usekey);
	  break;
    case SDL_SCANCODE_SPACE:   // fire or switch
      framework_EventHandle(FW_KEYUP,(int) prefs->shootkey);
	  framework_EventHandle(FW_KEYUP,(int) prefs->jumpkey);
      break;
    case SDL_SCANCODE_ESCAPE:  // break out of level
      framework_EventHandle(FW_KEYUP,(int) 'Q');
      break;
    case SDL_SCANCODE_E:  // If editor is compiled (define in mc.cpp) then this is the key to show it
      framework_EventHandle(FW_KEYUP,(int) 'E');
      break;
    case SDL_SCANCODE_P:  // if editor is compiled (define in mc.cpp) then this is the key to show patterns(tiles)
      framework_EventHandle(FW_KEYUP,(int) 'P');
      break;
    default:
      break;
  }
}


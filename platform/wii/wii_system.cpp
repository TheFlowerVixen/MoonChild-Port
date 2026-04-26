#include "platform_system.h"

#include <ogcsys.h>
#include <fat.h>
#include <wiiuse/wpad.h>
#include <cstdio>
#include <unistd.h>
#include <dirent.h>

#include "framewrk/frm_int.hpp"
#include "moonchild/mc.hpp"
#include "moonchild/globals.hpp"
#include "moonchild/prefs.hpp"

MoviePlayer* moviePlayer;

class CSystem {
public:
	CSystem() {}

	void doInit(void);
	void doShutdown(void);

	bool doCalc(void);

	bool getShutdownFlag(void) const {
		return mShutdownFlag;
	}
	bool getResetFlag(void) const {
		return mResetFlag;
	}

private:
	void setupMoonChild(void);
    void updateMoonChild(void);

	void moonChildSubmitKey(u32 button, s32 key) {
		if ((mButtonsDown & button) == button) {
			framework_EventHandle(FW_KEYDOWN, key);
		}
		else if ((mButtonsUp & button) == button) {
			framework_EventHandle(FW_KEYUP, key);
		}
	}

	static void sysPowerCallback(void);
	static void sysResetCallback(u32 irq, void *ctx);
	static void wpadPowerCallback(s32 chan);

private:
	u32 mButtonsDown;
	u32 mButtonsUp;
	u32 mButtonsHeld;

	bool mShutdownFlag;
	bool mResetFlag;
};
static CSystem sSystem;

/*
 * Implementation
 */

void CSystem::sysPowerCallback(void) {
	sSystem.mShutdownFlag = true;
}

void CSystem::sysResetCallback(u32 irq, void *ctx) {
	sSystem.mResetFlag = true;
}

void CSystem::wpadPowerCallback(s32 chan) {
	sSystem.mResetFlag = true;
}

void CSystem::doInit(void) {
	L2Enhance();

	SYS_STDIO_Report(true);

	fatInitDefault();

	WPAD_Init();
    WPAD_SetDataFormat(0, WPAD_FMT_BTNS_ACC_IR);

	mShutdownFlag = false;
	mResetFlag = false;

	SYS_SetResetCallback(sysResetCallback);
    SYS_SetPowerCallback(sysPowerCallback);
	WPAD_SetPowerButtonCallback(wpadPowerCallback);

	setupMoonChild();
}

void CSystem::doShutdown(void) {
	WPAD_Shutdown();

	if (mShutdownFlag) {
		mShutdownFlag = false;
		SYS_ResetSystem(SYS_POWEROFF, 0, 0);

		for (;;);
	}
}

bool CSystem::doCalc(void) {
	WPAD_ScanPads();

	mButtonsDown = WPAD_ButtonsDown(0);
    mButtonsUp = WPAD_ButtonsUp(0);
    mButtonsHeld = WPAD_ButtonsHeld(0);

	if ((mButtonsDown & WPAD_BUTTON_HOME) != 0) {
		return true;
	}

	updateMoonChild();

	return (mShutdownFlag || mResetFlag);
}

void CSystem::setupMoonChild(void) {
	moviePlayer = new MoviePlayer();
}

void CSystem::updateMoonChild(void) {
	moonChildSubmitKey(WPAD_BUTTON_LEFT, prefs->downkey);
	moonChildSubmitKey(WPAD_BUTTON_RIGHT, prefs->upkey);

	moonChildSubmitKey(WPAD_BUTTON_DOWN, prefs->rightkey);
	moonChildSubmitKey(WPAD_BUTTON_UP, prefs->leftkey);

	moonChildSubmitKey(WPAD_BUTTON_2, prefs->shootkey);

	moonChildSubmitKey(WPAD_BUTTON_PLUS, 'Q');
}

/*
 * Public interface
 */

bool initSystem() {
	sSystem.doInit();
	return true;
}
void shutdownSystem() {
	sSystem.doShutdown();
}

bool pollEvents() {
	return sSystem.doCalc();
}

// (stub)
void syncMouse() {}

// (stub)
void preSync() {}

// (stub)
void postSync() {}

// Called by the game to get the full path to a file
char *FullPath(char *filename) {
	if (filename == NULL) {
		return NULL;
	}

	static char buffer[128];
	snprintf(buffer, sizeof(buffer), "/moonchild_assets/moonchild/%s", filename);
	return buffer;
}

// Called by the game to get the full path to an audio file
char *FullAudioPath(char *filename) {
	if (filename == NULL) {
		return NULL;
	}

	static char buffer[128];
	snprintf(buffer, sizeof(buffer), "/moonchild_assets/audio/%s", filename);
	return buffer;
}

char *FullMoviePath(char *filename) {
	if (filename == NULL) {
		return NULL;
	}

	static char buffer[128];
	snprintf(buffer, sizeof(buffer), "/moonchild_assets/movies/%s", filename);
	return buffer;
}

// Called by the game to get the full path to a writable file (Only hiscore file)
char *FullWritablePath(char *filename) {
	if (filename == NULL) {
		return NULL;
	}

	static char buffer[128];
	snprintf(buffer, sizeof(buffer), "/moonchild_save/%s", filename);
	return buffer;
}

#if 0

// Internal method (only used here) to load a TGA file
typedef unsigned char BYTE;
unsigned short* LoadTGA(char *FileName) {
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

#endif

// Called by the game to show a picture (tga)
void ShowPicture(char *FileName)
{
#if 0
  unsigned short *TempPic;
	TempPic   = LoadTGA(FileName);
    
	//video->DrawTempPic();
    
	delete [] TempPic;
#endif
}


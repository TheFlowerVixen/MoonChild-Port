#include "platform_system.h"

#include <libdragon.h>
#include <cstdio>

#include "framewrk/frm_int.hpp"
#include "moonchild/mc.hpp"
#include "moonchild/globals.hpp"
#include "moonchild/prefs.hpp"

MoviePlayer* moviePlayer;

joypad_buttons_t butPressed;
joypad_buttons_t butReleased;

bool initSystem(int argc, char **argv)
{
    debug_init_isviewer();
    debug_init_usblog();

    if (dfs_init(DFS_DEFAULT_LOCATION) != DFS_ESUCCESS)
    {
        printf("Filesystem failed to start!\n");
        return 1;
    }
    else
    {
        printf("Filesystem started!\n");
    }

	joypad_init();

	return true;
}

void shutdownSystem()
{
	joypad_close();
}

bool pollEvents()
{
    joypad_poll();
    butPressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    butReleased = joypad_get_buttons_released(JOYPAD_PORT_1);

    if (butPressed.d_up)
        framework_EventHandle(FW_KEYDOWN, (int)prefs->upkey);
    if (butReleased.d_up)
        framework_EventHandle(FW_KEYUP, (int)prefs->upkey);
    
    if (butPressed.d_down)
        framework_EventHandle(FW_KEYDOWN, (int)prefs->downkey);
    if (butReleased.d_down)
        framework_EventHandle(FW_KEYUP, (int)prefs->downkey);
    
    if (butPressed.d_left)
        framework_EventHandle(FW_KEYDOWN, (int)prefs->leftkey);
    if (butReleased.d_left)
        framework_EventHandle(FW_KEYUP, (int)prefs->leftkey);
    
    if (butPressed.d_right)
        framework_EventHandle(FW_KEYDOWN, (int)prefs->rightkey);
    if (butReleased.d_right)
        framework_EventHandle(FW_KEYUP, (int)prefs->rightkey);

    if (butPressed.a)
        framework_EventHandle(FW_KEYDOWN, (int)prefs->shootkey);
    if (butReleased.a)
        framework_EventHandle(FW_KEYUP, (int)prefs->shootkey);
	
	return false;
}

void syncMouse()
{

}

void preSync()
{

}

void postSync()
{
	mixer_try_play();
}

// Called by the game to get the full path to a file
char *FullPath(char *filename)
{
	static char buffer[4096];
	if (!filename)
		return nullptr;
	snprintf(buffer, sizeof(buffer), "rom:/moonchild/%s", filename);
	return buffer;
}

// Called by the game to get the full path to an audio file
char *FullAudioPath(char *filename)
{
	static char buffer[4096];
	if (!filename)
		return nullptr;
	snprintf(buffer, sizeof(buffer), "rom:/audio/%s64", filename);
	return buffer;
}

// Called by the game to get the full path to a movie file
char *FullMoviePath(char *filename)
{
	static char buffer[4096];
	if (!filename)
		return nullptr;
	snprintf(buffer, sizeof(buffer), "rom:/movies/%s", filename);
	return buffer;
}

// Called by the game to get the full path to a writable file (Only hiscore file)
char *FullWritablePath(char *filename)
{	
	// TODO: writable path
	return nullptr;
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


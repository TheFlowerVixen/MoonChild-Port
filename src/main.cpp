#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _IN_MAIN
#include "framewrk/frm_int.hpp"
#include "framewrk/frm_wrk.hpp"
#include "moonchild/mc.hpp"
#include "moonchild/globals.hpp"
#include "moonchild/prefs.hpp"

int g_MouseFlg = 0;
int g_MouseActualFlg = 0;
int g_MouseXDown = 0;
int g_MouseYDown = 0;
int g_MouseXCurrent = 0;
int g_MouseYCurrent = 0;

#include "macro.h"

bool initMoonChild(unsigned char *pixelBuffer, int width, int height)
{
	video = new Cvideo();
	audio = new Caudio();  // create audio AFTER window is created!
	timer = new Ctimer();  // Create timer facilities
	movie = new Cmovie(audio, moviePlayer);  // Initiate movie playback features
	
	g_SettingsFlg = 0;	//we starten met het settings window off 
	gbGameLoop = 1;
	
	heartbeat = NULL;
    
    if (!video->on(pixelBuffer, width, height, 256) )
    {
        //EXIT!
        return false;
    }
    
    heartbeat = framework_InitGame(video, audio, timer, movie);
    
    if (heartbeat == NULL)
    {
        //EXIT!
        return false;
    }

	return true;
}
   
// if someone wants to reset progress (aka start over). This is how to do it
void resetProgress()
{
	//reset code
	maxlevel = 0;

	for(int i=0; i<13; i++)
	{
		blacksperlevel[i] = 0;
		scoreblacksperlevel[i] = 0;
	}
}
 
// if someone wants to enable the cheat. This is how to do it
void enableCheat()
{
    //cheat code
    maxlevel = 12;
    
    for(int i=0; i<13; i++)
    {
        blacksperlevel[i] = 0;
        scoreblacksperlevel[i] = 0;
    }
}

bool gameTick()
{    
    audio->checkVolume();
    
    if (g_SettingsFlg) // settings screen op het beeld?
        video->DrawSettings();
    else
    {
        if (heartbeat != NULL)
        {
            heartbeat = (HEARTBEAT_FN) heartbeat();
            if (heartbeat == NULL) // No heartbeat anymore, Let's close
				return false;
        }
    }

	return true;
}

int main(int argc, char **argv)
{
	if (!initSystem(argc, argv))
	{
		CMN_DEBUG_LOG("[main] failed to initialize system subsystem");
		shutdownSystem();
		return 1;
	}

	if (!initVideo(argc, argv))
	{
		CMN_DEBUG_LOG("[main] failed to initialize video subsystem");
		shutdownVideo();
		shutdownSystem();
		return 1;
	}

	if (!initAudio(argc, argv))
	{
		CMN_DEBUG_LOG("[main] failed to initialize audio subsystem");
		shutdownAudio();
		shutdownVideo();
		shutdownSystem();
		return 1;
	}

	initMoonChild(pixelBuffer, GAME_WIDTH, GAME_HEIGHT);
	while (1)
	{
		if (pollEvents())
			break;
		
		syncMouse();

    	preSync();

		if (moviePlayer && moviePlayer->isPlaying())
			moviePlayer->update(pixelBuffer, GAME_WIDTH, GAME_HEIGHT, pixelBufferPitch);
		else if (!gameTick())
			break;

		blitScreen();

		postSync();
	}

	shutdownAudio();
	shutdownVideo();
	shutdownSystem();

	return 0;
}

#ifndef _PREFS_H
#define _PREFS_H

#include "../framewrk/frm_wrk.hpp"

/* Stuff which has to do with preferences */

void prefs_calcvals(void);

#define PREFS_LORES 0
#define PREFS_HIRES 1

struct PREFS
{
  UINT16 screentopx;
  UINT16 screentopy;
  UINT16 screenwidth;
  UINT16 screenheight;
  UINT16 reso;
#if !(defined(PLATFORM_WII) || defined(PLATFORM_N64))
  UINT16 leftkey;
  UINT16 rightkey;
  UINT16 upkey;
  UINT16 downkey;
  UINT16 usekey;
  UINT16 shootkey;
#endif
};



#endif

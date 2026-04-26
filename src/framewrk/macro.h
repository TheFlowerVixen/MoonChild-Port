#ifndef MACRO_H
#define MACRO_H

#include <stdio.h>

#if defined(PLATFORM_N64) || defined(PLATFORM_WII)

#define BE_BSWAP_16(x) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))
#define BE_BSWAP_32(x)                                            \
     ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) |   \
      (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#else

#define BE_BSWAP_16(x) (x)
#define BE_BSWAP_32(x) (x)

#endif // defined(PLATFORM_N64) || defined(PLATFORM_WII)

#define FRMWORK_ASSERT(condition) do { \
    if (!(condition)) { printf("[FRMWORK_ASSERT] Assertion failed: " #condition "\n"); for (;;); } \
} while (0)

#endif // MACRO_H

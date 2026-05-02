#ifndef MACRO_H
#define MACRO_H

#include <stdio.h>

#if defined(PLATFORM_N64) || defined(PLATFORM_WII)

#define BIGENDIAN

#define BE_BSWAP_16(x) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))
#define BE_BSWAP_32(x)                                            \
     ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) |   \
      (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#else

#define BE_BSWAP_16(x) (x)
#define BE_BSWAP_32(x) (x)

#endif // defined(PLATFORM_N64) || defined(PLATFORM_WII)

#define CMN_ASSERT(condition) do {                                                              \
    if (!(condition)) { printf("[CMN_ASSERT] Assertion failed: " #condition "\n"); for (;;); }  \
} while (0)

#ifdef LOG_FILE

// This is probably the stupidest way of writing to a log file but it's the only way I tried that works
#define CMN_DEBUG_LOG(...) do { \
    FILE* logFile = fopen("/mc_log.txt", "a"); \
    if (logFile) { fprintf(logFile, __VA_ARGS__); fclose(logFile); } \
    printf(__VA_ARGS__); \
} while (0)

#else

#define CMN_DEBUG_LOG(...) printf(__VA_ARGS__);

#endif

#endif // MACRO_H

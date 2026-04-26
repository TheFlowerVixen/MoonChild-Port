#include "wii/detail/wave_data.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

typedef unsigned long u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef short s16;

#define IDENTIFIER_TO_U32(char1, char2, char3, char4) ( \
    ((u32)(char1) << 24) | ((u32)(char2) << 16) |       \
    ((u32)(char3) <<  8) | ((u32)(char4) <<  0)         \
)

#define BE_BSWAP_16(x) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))
#define BE_BSWAP_32(x)                                            \
     ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) |   \
      (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#define Panic(...) do { printf(__VA_ARGS__); for (;;); } while (0)

enum {
    /* Master */
    WAV_CHUNK_RIFF = IDENTIFIER_TO_U32('R','I','F','F'),
    /* Format */
    WAV_CHUNK_FMT  = IDENTIFIER_TO_U32('f','m','t',' '),
    /* Data */
    WAV_CHUNK_DATA = IDENTIFIER_TO_U32('d','a','t','a'),
};

#define WAVE_MAGIC IDENTIFIER_TO_U32('W','A','V','E')

enum {
    WAV_FORMAT_PCM      = 1,
    WAV_FORMAT_FLOAT    = 3,
    WAV_FORMAT_A_LAW    = 6,
    WAV_FORMAT_MU_LAW   = 7
};

typedef struct __attribute__((packed)) {
    /* Master (RIFF) Chunk */
    u32 riffChunkType; /* Compare to WAVE_CHUNK_RIFF. */
    u32 riffChunkSize; /* The file size can be calculated from this value by adding
                        * the size of riffChunkType and riffChunkSize (effectively 8).
                        */
    u32 waveMagic; /* Compare to WAVE_MAGIC. */
} WavFileHeader;

static inline u32 _WavCalcFileSize(void *wavData) {
    const WavFileHeader *fileHeader = (const WavFileHeader *)wavData;
    return BE_BSWAP_32(fileHeader->riffChunkSize) + 8;
}

/* Chunk type is WAVE_CHUNK_FMT. */
typedef struct __attribute__((packed)) {
    u16 format; /* See WAVE_FORMAT enums. */

    u16 channelCount;
    u32 sampleRate;

    u32 dataRate; // Bytes per second (sampleRate * sampleSize * channelCount)

    u16 blockSize; // sampleSize * channelCount

    u16 bitsPerSample; // 8 * sampleSize
} WavFmtChunk;

/* Chunk type is WAVE_CHUNK_DATA. */
typedef struct __attribute__((packed)) {
    u8 data[1];
} WavDataChunk;

typedef struct __attribute((packed)) {
    u32 type; /* See WAVE_CHUNK enums. */
    u32 size; /* Size of chunk body. */
} WavChunkHeader;

/* NOTE: This function will panic if the specified chunk is not found */
template <typename T>
const T *_WavFindChunk(void *wavData, u32 type) {
    const WavChunkHeader *chunksStart = (const WavChunkHeader *)((u8 *)wavData + sizeof(WavFileHeader));
    const WavChunkHeader *chunksEnd = (const WavChunkHeader *)((u8 *)wavData + _WavCalcFileSize(wavData));
    
    const WavChunkHeader *currentChunk = (const WavChunkHeader *)(chunksStart);
    while ((currentChunk + 1) <= chunksEnd) {
        if (currentChunk->type == type)
            return (const T *)(currentChunk + 1);

        currentChunk = (const WavChunkHeader *)(
            (const u8 *)(currentChunk + 1) + ((BE_BSWAP_32(currentChunk->size) + 1) & ~1)
        );
    }

    const char *typeChars = (const char *)&type;
    Panic(
        "WAV '%c%c%c%c' chunk not found",
        typeChars[0], typeChars[1], typeChars[2], typeChars[3]
    );
}

u32 _WavGetChunkSize(const void *chunkData) {
    const WavChunkHeader *chunkHeader = (const WavChunkHeader *)(
        (const u8 *)chunkData - sizeof(WavChunkHeader)
    );
    return BE_BSWAP_32(chunkHeader->size);
}

void WavPreprocess(void *wavData) {
    const WavFileHeader *fileHeader = (const WavFileHeader *)wavData;

    if (fileHeader->riffChunkType != WAV_CHUNK_RIFF) {
        Panic("WavPreprocess: WAV RIFF chunk type is nonmatching");
    }

    if (fileHeader->waveMagic != WAVE_MAGIC) {
        Panic("WavPreprocess: WAV WAVE magic is nonmatching");
    }

    const WavFmtChunk *fmtChunk = _WavFindChunk<WavFmtChunk>(wavData, WAV_CHUNK_FMT);

    u16 sampleFormat = BE_BSWAP_16(fmtChunk->format);
    u16 bitsPerSample = BE_BSWAP_16(fmtChunk->bitsPerSample);

    switch (sampleFormat) {
    case WAV_FORMAT_PCM:
        if ((bitsPerSample != 16) && (bitsPerSample != 24)) {
            Panic(
                "WavPreprocess: %u-bit PCM isn't supported (expected 32-bit FLOAT, "
                "16-bit PCM, or 24-bit PCM)",
                bitsPerSample
            );
        }
        break;
    case WAV_FORMAT_FLOAT:
        if (bitsPerSample != 32) {
            Panic(
                "WavPreprocess: %u-bit FLOAT isn't supported (expected 32-bit FLOAT, "
                "16-bit PCM, or 24-bit PCM)",
                bitsPerSample
            );
        }
        break;
    case WAV_FORMAT_A_LAW:
        Panic("WavPreprocess: A-LAW format is unsupported");
    case WAV_FORMAT_MU_LAW:
        Panic("WavPreprocess: MU-LAW format is unsupported");
    default:
        Panic("WavPreprocess: WAV format %u is unknown", sampleFormat);
    }
}

u32 WavGetSampleRate(void *wavData) {
    const WavFmtChunk *fmtChunk = _WavFindChunk<WavFmtChunk>(wavData, WAV_CHUNK_FMT);
    return BE_BSWAP_32(fmtChunk->sampleRate);
}

u16 WavGetChannelCount(void *wavData) {
    const WavFmtChunk *fmtChunk = _WavFindChunk<WavFmtChunk>(wavData, WAV_CHUNK_FMT);
    return BE_BSWAP_16(fmtChunk->channelCount);
}

bool WavGetSamplesAreFloat(void *wavData) {
    const WavFmtChunk *fmtChunk = _WavFindChunk<WavFmtChunk>(wavData, WAV_CHUNK_FMT);
    return fmtChunk->format == BE_BSWAP_16(WAV_FORMAT_FLOAT);
}

u16 WavGetSampleSize(void *wavData) {
    const WavFmtChunk *fmtChunk = _WavFindChunk<WavFmtChunk>(wavData, WAV_CHUNK_FMT);
    return BE_BSWAP_16(fmtChunk->bitsPerSample) / 8;
}

void *WavGetData(void *wavData) {
    const WavDataChunk *dataChunk = _WavFindChunk<WavDataChunk>(wavData, WAV_CHUNK_DATA);
    return (void *)dataChunk->data;
}

u32 WavGetDataSize(void *wavData) {
    const WavDataChunk *dataChunk = _WavFindChunk<WavDataChunk>(wavData, WAV_CHUNK_DATA);
    return _WavGetChunkSize(dataChunk);
}

u32 WavGetSampleCount(void *wavData) {
    return WavGetDataSize(wavData) / WavGetSampleSize(wavData);
}

s16 *WavGetPCM16(void *wavData) {
    const WavFmtChunk *fmtChunk = _WavFindChunk<WavFmtChunk>(wavData, WAV_CHUNK_FMT);
    const WavDataChunk *dataChunk = _WavFindChunk<WavDataChunk>(wavData, WAV_CHUNK_DATA);

    u32 sampleCount = WavGetSampleCount(wavData);

    u32 dstSize = ((sizeof(s16) * sampleCount) + 31) & ~31;
    s16 *dst = (s16 *)aligned_alloc(32, dstSize);
    if (dst == NULL) {
        Panic("WavGetPCM16: failed to allocate sample buffer");
    }

    memset(dst, 0x00, dstSize);

    u16 sampleFormat = BE_BSWAP_16(fmtChunk->format);
    u16 bitsPerSample = BE_BSWAP_16(fmtChunk->bitsPerSample);

    /* 32-bit FLOAT sample format */
    if (sampleFormat == WAV_FORMAT_FLOAT) {
        const u32 *srcI = (const u32 *)dataChunk->data;
        for (u32 i = 0; i < sampleCount; i++) {
            u32 sampleI = BE_BSWAP_32(srcI[i]);
            float sample = *(float *)&sampleI * 32768.f;

            if (sample > 32767.f) {
                sample = 32767.f;
            }
            else if (sample < -32768.f) {
                sample = -32768.f;
            }
            dst[i] = (s16)roundf(sample);
        }
    }
    /* 16-bit PCM sample format */
    else if ((sampleFormat == WAV_FORMAT_PCM) && (bitsPerSample == 16)) {
        for (u32 i = 0; i < sampleCount; i++) {
            const u16 *src = (const u16 *)dataChunk->data;
            dst[i] = BE_BSWAP_16(src[i]);
        }
    }
    /* 24-bit PCM sample format */
    else if ((sampleFormat == WAV_FORMAT_PCM) && (bitsPerSample == 24)) {
        const u8 *src = (const u8 *)dataChunk->data;
        for (u32 i = 0; i < sampleCount; i++) {
            int sample = ((int)(src[i * 3 + 2]) << 8)  | 
                         ((int)(src[i * 3 + 1]) << 16) | 
                         ((int)(src[i * 3 + 0]) << 24);
            sample >>= 8; /* Normalize */

            if (sample > 32767) {
                sample = 32767;
            }
            else if (sample < -32768) {
                sample = -32768;
            }
            dst[i] = (s16)sample;
        }
    }
    else {
        Panic("WavGetPCM16: no convert condition met");
    }
    
    return dst;
}

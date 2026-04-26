#ifndef _WII_DETAIL_WAVE_DATA_H
#define _WII_DETAIL_WAVE_DATA_H

void WavPreprocess(void *wavData);

unsigned long WavGetSampleRate(void *wavData);
unsigned short WavGetChannelCount(void *wavData);
bool WavGetSamplesAreFloat(void *wavData);
unsigned short WavGetSampleSize(void *wavData);
void *WavGetData(void *wavData);
unsigned long WavGetDataSize(void *wavData);
unsigned long WavGetSampleCount(void *wavData);
short *WavGetPCM16(void *wavData);

#endif // _WII_DETAIL_WAVE_DATA_H

//delay.h
#ifndef DELAY_H
#define DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

//#define _POSIX_C_SOURCE 199309L
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../../typedef.h"
#include <stdio.h>

typedef struct ESDelay ESDelay;

struct ESDelay
{
	MySampleType* delay_buffer;
	MySampleType* temp_buffer;
	unsigned int sample_count;
	unsigned length;
	unsigned index;
};

MySampleType* ESDelayCreateArray(unsigned int arraySize);

MySampleType* ESDelayCreateCopyOfArray(ESDelay* delay, unsigned int sample_count);

bool ESDelayInitialise(ESDelay **delay, unsigned int sample_count, unsigned length);

bool ESDelayCreate(ESDelay **delay, unsigned int sample_count, unsigned length);

bool ESDelayDestroy(ESDelay *delay);

MySampleType* createCopyOfArray(ESDelay* delay, unsigned int sample_count);	

MySampleType* ESDelayDelayChannels(ESDelay* delay, MySampleType* samples, unsigned int sample_count);


#ifdef __cplusplus
}
#endif

#endif /* DELAY_H */
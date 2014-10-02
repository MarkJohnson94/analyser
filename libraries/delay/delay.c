//#define _POSIX_C_SOURCE 199309L
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "delay.h"

typedef struct ESDelay ESDelay;

MySampleType* ESDelayCreateArray(unsigned int array_size)
{
	MySampleType *arr;
	arr = (MySampleType*)calloc(array_size, sizeof(MySampleType)); //allocate memory
	int i;
	for (i = 0; i < array_size; i++) //instantiate array
	{
		*(arr + i) = 0;
	}
	return arr;
}

bool ESDelayInitialise(ESDelay **delay, unsigned int sample_count, unsigned length)
{
	ESDelay *new_delay = (ESDelay *)malloc(sizeof(ESDelay));
    if (new_delay == NULL)
	{
		return false;
	}
	
	new_delay->delay_buffer = ESDelayCreateArray(sample_count);
	new_delay->temp_buffer = ESDelayCreateArray(length);
	new_delay->sample_count = sample_count;
	new_delay->index = length;
	new_delay->length = length;
	
	*delay = new_delay;
}

bool ESDelayCreate(ESDelay **delay, unsigned int sample_count, unsigned length)
{
	return ESDelayInitialise(delay, sample_count, length);
}

bool ESDelayDestroy(ESDelay *delay)
{
    free(delay->delay_buffer);
    free(delay->temp_buffer);
	free(delay);
    return true;
}

MySampleType* ESDelayDelayChannels(ESDelay* delay, MySampleType* samples, unsigned int sample_count)
{	
	int i;
	for (i=0; i < delay->length; i++)
	{
		delay->delay_buffer[i] = delay->temp_buffer[i];
	}
	
	for (i=delay->length; i < sample_count; i++)
	{
		delay->delay_buffer[i] = samples[i-delay->length];
	}
	
	unsigned count = 0;
	for (i=sample_count; i < sample_count+delay->length; i++)
	{
		delay->temp_buffer[count] = samples[i-delay->length];
		count++;
	}
	
	return delay->delay_buffer;
}
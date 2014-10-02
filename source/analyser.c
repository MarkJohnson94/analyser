#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <fftw3.h>
#include <inttypes.h>
#include <stdbool.h>

#include "analyser.h"
#include "analyser_internal.h"

prof_time start;

float total_time = 0;

MySampleType* createArray(unsigned int array_size);
double* createArrayDouble(unsigned int array_size);
uint8_t* createArrayUInt(unsigned int array_size);
void build_linked_list(ESAnalyser* analyser);
input* BuildLinkedList();
void Push(input** head, double* mag, int sample_rate);

bool ESAnalyserCreate(ESAnalyser **analyser, ESAnalyserOptions *options, unsigned int sample_rate)
{
	ESAnalyser *new_analyser = (ESAnalyser *)malloc(sizeof(ESAnalyser));
    if (new_analyser == NULL)
	{
		return false;
	}

	if (options->levelmeter.active == true)
	{
		ESAnalyserSetLevelmeter(new_analyser, options);
	}
	if (options->histogram.active == true)
	{
		ESAnalyserSetHistogram(new_analyser, options);
	}
	if (options->waveform.active == true)
	{
		ESAnalyserSetWaveform(new_analyser, options);
	}
	if (options->spectrogram.active == true)
	{
		ESAnalyserSetSpectrogram(new_analyser, options);
	}

	new_analyser->sample_rate = sample_rate;
	new_analyser->callback = options->lib.callback;
	new_analyser->counter = 0;
	new_analyser->crossing_points = 0;
	new_analyser->old_crossing_points = 0;


	/*			R128 Loudness Curve				*/
	//constructor
	LevelAudioBlockSingleBiquad biquad;
	LevelAudioBlockSingleBiquadCreate(&biquad);
	
	//setter
	LevelAudioBlockSingleBiquadParams params = {
	.type = (LevelAudioBlockBiquadTypeHighPass),
	.freq_norm = (((MySampleType)38) / (sample_rate)),
	.quality = (0.5),
	.gain = (0)
	};
	LevelAudioBlockSingleBiquadSetParams(&biquad, &params);
	new_analyser->biquad = biquad;
	/*											*/

    *analyser = new_analyser;
    return true;
}

bool ESAnalyserDestroy(ESAnalyser *analyser)
{
	if (analyser->options.waveform.active == true)
	{
		free(analyser->options.waveform.next_value_buffer);
		free(analyser->options.waveform.internal_buffer);
	}

	if (analyser->options.histogram.active == true)
	{
		free(analyser->options.histogram.array);
		free(analyser->options.histogram.normalised_array);
	}

	if (analyser->options.spectrogram.active == true)
	{
		free(analyser->options.spectrogram.array);
	}
	
	free(analyser);
    return true;
}

MySampleType* ESAnalyserGetLoudnessSamples(ESAnalyser *analyser, MySampleType* buffer, unsigned num_of_samples)
{
	MySampleType value;
	for (unsigned i = 0; i < num_of_samples; i++)
	{
		value = buffer[i];
		buffer[i] = LevelAudioBlockSingleBiquadProcess(&(analyser->biquad), value);
	}
	return buffer;
}

MySampleType ESAnalyserChunkAverage(MySampleType* samples, unsigned int chunk_start, unsigned int chunk_end)
{
	MySampleType value = 0;
	MySampleType average = 0;
	for (unsigned i = chunk_start; i < chunk_end; i++)
	{
		value = samples[i];
		average += value;
	}
	average /= (chunk_end - chunk_start);
	return average;
}


MySampleType* ESAnalyserNextValue(ESAnalyser *analyser)
{
	prof_time timedif = prof_diff_time(start, prof_get_time());
	double diff = prof_time_as_milliseconds(timedif);
	
	if (diff > 0)
	{
		total_time += diff;
	}
	
	if (total_time < analyser->options.waveform.time_to_deliver)
	{
		start = prof_get_time();
		return analyser->options.waveform.next_value_buffer;
	}else
	{
		double n = floor(total_time/analyser->options.waveform.time_to_deliver);
		total_time = total_time - (float)(n*analyser->options.waveform.time_to_deliver);
		
		int count = 0;
		while ((count < n) && (count < analyser->options.waveform.points_to_store*8))
		{
			if (analyser->options.waveform.next_value_count >= analyser->options.waveform.num_of_points)
			{
				analyser->options.waveform.next_value_count = 0;
			}
			if (count >= analyser->options.waveform.points_to_store*8)
			{
				count = 0;
			}
			analyser->options.waveform.next_value_buffer[analyser->options.waveform.next_value_count] = analyser->options.waveform.internal_buffer[count];
			analyser->options.waveform.next_value_count++;
			count++;
		}
		start = prof_get_time();
		return analyser->options.waveform.next_value_buffer;
	}
}

MySampleType ESAnalyserLinearToDB(MySampleType value, unsigned* peak, unsigned* range)
{
	//to be valid, range > 0
	//0 >= peak >= upper limit + range
	//ie if range is 60, the lowest peak you allow is 84 as the lowest dB value is -144.
	unsigned high;
	unsigned range_val;

	if (peak && range)
	{
		unsigned limit = -144;
		high = *peak;
		if (*range > 0)
		{
			range_val = *range;
			if (high-range_val < limit)
			{
				range_val = limit-high;
			}
		}else
		{
			high = 0;
			if (range_val > abs(limit))
			{
				range_val = abs(limit);
			}
		}
	}else
	{
		high = 0;
		range_val = 60;
	}
	if (value==0)
	{
		return value;
	}
	
	MySampleType db_value = 20*log10f(value);
	MySampleType return_value = ((db_value+(MySampleType)range_val)/(MySampleType)range_val);
	
	return return_value;
}

MySampleType ESAnalyserLinearToDBValue(MySampleType value, unsigned* peak, unsigned* range)
{
	unsigned high;
	unsigned range_val;

	if (peak && range)
	{
		unsigned limit = -144;
		high = *peak;
		if (*range > 0)
		{
			range_val = *range;
			if (high-range_val < limit)
			{
				range_val = limit-high;
			}
		}else
		{
			high = 0;
			if (range_val > abs(limit))
			{
				range_val = abs(limit);
			}
		}
	}else
	{
		high = 0;
		range_val = 60;
	}
	if (value==0)
	{
		return value;
	}
	
	MySampleType db_value = 20*log10f(value);
	
	return db_value;
}

MySampleType ESAnalyserGetBiggestValue(MySampleType* samples, unsigned int num_of_samples)
{
	MySampleType biggest_value = 0;
	for (unsigned i = 0; i < num_of_samples; i++)
	{
		if ((float)samples[i] > biggest_value)
		{
			biggest_value = (float)samples[i];
		}
	}
	return biggest_value;
}

MySampleType ESAnalyserGetNumOfValues(MySampleType* samples, unsigned int num_of_samples)
{
	MySampleType total = 0;
	for (unsigned i = 0; i < num_of_samples; i++)
	{	
		total += (float)samples[i];
	}
	return total;
}

MySampleType* internal_TruncateBuffer(MySampleType *samples, unsigned int sample_count, unsigned int truncate_from)
{
	unsigned int position = truncate_from;
	MySampleType *newBuffer;
	newBuffer = createArray(sample_count-1-position); //need to work out how to free this..
	unsigned int count = 0;
	while (position < sample_count)
	{
		newBuffer[count] = samples[position];
		position++;
		count++;
	}
	return newBuffer;
}

bool ESAnalyserTruncate(ESTuple *aTuple, MySampleType *samples, unsigned int sample_count)
{
	//FILE *f = fopen("textfile.txt", "a+");
	MySampleType value1;
	MySampleType value2;

	/*tTuple *new_tuple = (tTuple *)malloc(sizeof(tTuple));
	if (new_tuple == NULL) 
	{
		return false;
	}*/
	
	///////////////////////////////////////////////////////////////////////////////
	//First of all search for a negative to positive section in the sample
	//if it is found, then truncate the sample to begin at this point
	//////////////////////////////////////////////////////////////////////////////
	for (unsigned i = 0; i < sample_count-1; i++) 
	{
		value1 = samples[i];
		value2 = samples[i+1];
		if ((value1 <= 0) && (value2 > 0))
		{
			MySampleType *newer_buffer = internal_TruncateBuffer(samples, sample_count, i);
			aTuple->buffer=newer_buffer;
			aTuple->size = i;
			return true;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	//If there is not a negative to positive section in the sample
	//then look for a peak in the sample, ie find a value greater than zero
	//where the next value is larger, and keep searching until the next value is smaller
	//this represents the first peak
	//////////////////////////////////////////////////////////////////////////////
	for (unsigned i = 0; i < sample_count-1; i++) 
	{
		float value1 = samples[i];
		float value2 = samples[i+1];
		if ((value1 > 0) && (value2 > value1))
		{
			unsigned int count = i;
			while ((value1 > 0) && (value2 > value1) && (count < sample_count-1))
			{
				count++;
				float value1 = samples[count];
				float value2 = samples[count+1];
			}
			MySampleType *newer_buffer = internal_TruncateBuffer(samples, sample_count, count-1);
			aTuple->buffer=newer_buffer;
			aTuple->size = i;
			return true;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	//If there is not a peak in the sample that is greater than zero
	//then calculate the DC (average) of the sample, and look for the first rise
	//from below the average to above the average
	//if this is found then truncate the sample to begin at this point
	//////////////////////////////////////////////////////////////////////////////
	float average = ESAnalyserSimpleAverage(samples, sample_count);
	for (unsigned i = 0; i < sample_count-1; i++)
	{
		value1 = samples[i];
		value2 = samples[i+1];
		if ((value1 <= average) && (value2 > average))
		{
			MySampleType *newer_buffer = internal_TruncateBuffer(samples, sample_count, i);
			aTuple->buffer=newer_buffer;
			aTuple->size = i;
			return true;
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////
	//If all of these fail, then return an empty buffer
	//////////////////////////////////////////////////////////////////////////////
	MySampleType *buffer;
	buffer = createArray(sample_count);
	for (unsigned i = 0; i < sample_count; i++)
	{
		buffer[i] = 0;
	}
	aTuple->buffer=buffer;
	aTuple->size = sample_count;
	//fprintf(f, "returning empty buffer\n");
	//fclose(f);
	return false;
}

MySampleType ESAnalyserLongZeroCrossings(ESAnalyser* analyser, MySampleType *samples, unsigned int sample_count, unsigned globalCounter)
{
	analyser->crossing_points += ESAnalyserNumOfZeroCrossingPoints(samples, sample_count);
	if (globalCounter >= 42)
	{
		analyser->old_crossing_points = analyser->crossing_points;
		analyser->crossing_points=0;
	}
	return analyser->old_crossing_points;
}

MySampleType ESAnalyserNumOfZeroCrossingPoints(MySampleType *samples, unsigned int sample_count)
{
	MySampleType value1;
	MySampleType value2;
	MySampleType count = 0;

	for (unsigned i = 1; i < sample_count; i++) 
	{
		value1 = samples[i-1];
		value2 = samples[i];
		if ((value1 < 0) && (value2 > 0))
		{
			count++;
		}else if ((value1 > 0) && (value2 < 0))
		{
			count++;
		}
	}
	
	return count;
}

MySampleType ESAnalyserZeroCrossing(MySampleType *samples, unsigned int sample_count)
{
	MySampleType value1;
	MySampleType value2;

	for (unsigned i = 1; i < sample_count; i++) 
	{
		value1 = samples[i-1];
		value2 = samples[i];
		if ((0 < value1) && (value1 < value2))
		{
			return (MySampleType)i-1; //returns position of first negative to positive
		}
	}
	
	return -1;
}

//////////INTERNAL METHODS/////////////////////

MySampleType* createArray(unsigned int array_size)
{
	MySampleType *arr = (MySampleType*)calloc(array_size, sizeof(MySampleType));
	return arr;
}

double* createArrayDouble(unsigned int array_size)
{
	double *arr = (double*)calloc(array_size, sizeof(double));
	return arr;
}

uint8_t* createArrayUInt(unsigned int array_size)
{
	uint8_t *arr = (uint8_t*)calloc(array_size, sizeof(uint8_t));
	return arr;
}
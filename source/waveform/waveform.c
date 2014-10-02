#include <stdint.h>
#include "../analyser.h"
#include "../analyser_internal.h"
//#include "waveform.h"

bool ESAnalyserSetWaveform(ESAnalyser *analyser, ESAnalyserOptions *options)
{
	analyser->options.waveform.sample_rate = options->waveform.sample_rate; //normally 44100
	analyser->options.waveform.channel_count = options->waveform.channel_count; //normally 2
	analyser->options.waveform.chunk_size = options->waveform.chunk_size; //normally 512
	
	analyser->options.waveform.num_of_intervals = options->waveform.num_of_intervals;

	analyser->options.waveform.num_of_points = options->waveform.num_of_points;
	analyser->options.waveform.num_of_seconds = options->waveform.num_of_seconds;
	analyser->options.waveform.points_to_store = options->waveform.num_of_points;

	analyser->options.waveform.time_to_deliver = (float)((options->waveform.num_of_seconds*1000)/options->waveform.num_of_points);

	analyser->options.waveform.next_value_buffer = createArray(options->waveform.num_of_points);
	analyser->options.waveform.next_value_count = 0;

	analyser->options.waveform.internal_buffer = createArray(options->waveform.num_of_points);
	
	unsigned int points_per_sec = options->waveform.num_of_points/options->waveform.num_of_seconds;
	unsigned int chunks_per_sec = options->waveform.sample_rate/options->waveform.chunk_size;
	analyser->options.waveform.to_store = points_per_sec/chunks_per_sec;

	analyser->options.waveform.current_count = 0;

	return true;
}

void ESAnalyserWaveform(ESAnalyser* analyser, MySampleType* samples)
{
	unsigned int new_chunk = analyser->options.waveform.chunk_size/analyser->options.waveform.to_store;
	
	for (int count = 0; count < analyser->options.waveform.to_store; count++)
	{
		analyser->options.waveform.internal_buffer[analyser->options.waveform.current_count] = ESAnalyserChunkAverage(samples, new_chunk*count, (new_chunk*count)+count); 
		
		analyser->options.waveform.current_count++;
		if (analyser->options.waveform.current_count > analyser->options.waveform.points_to_store)
		{
			analyser->options.waveform.current_count = 0;
		}
	}
}

MySampleType* ESAnalyserReturnWaveform(ESAnalyser *analyser)
{
	return analyser->options.waveform.internal_buffer;
}

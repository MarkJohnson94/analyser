#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../analyser.h"
#include "../analyser_internal.h"

MySampleType ESAnalyserHighestProbablility(MySampleType* samples, unsigned int num_of_samples);

bool ESAnalyserSetHistogram(ESAnalyser *analyser, ESAnalyserOptions *options)
{
	analyser->options.histogram.period = options->histogram.period/(float)1000;
	analyser->options.histogram.high = options->histogram.high;
	analyser->options.histogram.low = options->histogram.low;
	analyser->options.histogram.interval = options->histogram.interval;
	analyser->options.histogram.sample_rate = options->histogram.sample_rate;
	float val = ((float)options->histogram.high-(float)options->histogram.low)/options->histogram.interval;
	analyser->options.histogram.num_of_points = (int)val;
	analyser->options.histogram.array = createArray(analyser->options.histogram.num_of_points);
	analyser->options.histogram.normalised_array = createArray(analyser->options.histogram.num_of_points);
	
	analyser->options.histogram.count = 0;
	analyser->options.histogram.times_called = 0;
	analyser->options.histogram.total_times_called = 0;
	analyser->options.histogram.old_time_covered = 0;

	return true;
}

void ESAnalyserNormaliseHistogram(ESAnalyser* analyser, MySampleType* samples, int num_of_samples)
{
	MySampleType highest_probability = ESAnalyserHighestProbablility(samples, num_of_samples);

	if (highest_probability > 0)
	{
		for (unsigned i = 0; i < num_of_samples; i++)
		{	
			MySampleType val = analyser->options.histogram.array[i];
			analyser->options.histogram.normalised_array[i] = val / highest_probability;
		}
	}
}

void ESAnalyserHistogram(ESAnalyser *analyser, MySampleType* samples, unsigned int num_of_samples)
{
	analyser->options.histogram.times_called += 1;
	analyser->options.histogram.total_times_called += 1;
	MySampleType time_covered = (float)analyser->options.histogram.times_called * \
	((float)num_of_samples / ((float)analyser->options.histogram.sample_rate)) + analyser->options.histogram.old_time_covered;
	analyser->options.histogram.old_time_covered = 0;
	analyser->options.histogram.count += num_of_samples;

	samples = ESAnalyserGetLoudnessSamples(analyser, samples, num_of_samples);

	MySampleType average = ESAnalyserChunkAverage(samples, 0, num_of_samples);
	if (average < 0)
	{
		average *= -1;
	}

	unsigned high = (unsigned)analyser->options.histogram.high;
	unsigned low = (unsigned)analyser->options.histogram.low;
	unsigned db_range = low*-1;

	MySampleType db_average = ESAnalyserLinearToDBValue(average, &high, &db_range);
	
	float interval = analyser->options.histogram.interval;
	float num_of_segments = (analyser->options.histogram.high-analyser->options.histogram.low)/interval;
	float range = (high + low)*-1;

	if ((db_average > range*-1) && (db_average < high)) //save in array
	{
		MySampleType value = (((db_average/range)*num_of_segments)+num_of_segments);
		analyser->options.histogram.array[(int)value] += 1;
	}

	if (time_covered > analyser->options.histogram.period)
	{
		analyser->options.histogram.old_time_covered = time_covered - analyser->options.histogram.period;
		ESAnalyserNormaliseHistogram(analyser, analyser->options.histogram.array, analyser->options.histogram.num_of_points);
		analyser->options.histogram.times_called = 0;
	}
}

/*						private methods						*/
MySampleType ESAnalyserHighestProbablility(MySampleType* samples, unsigned int num_of_samples)
{
	MySampleType total = 0;
	for (unsigned i = 0; i < num_of_samples; i++)
	{	
		if ((float)samples[i] > total)
		{
			total = (float)samples[i];
		}
	}
	return total;
}

MySampleType* ESAnalyserGetHistogram(ESAnalyser *analyser)
{
	return analyser->options.histogram.normalised_array;
}

MySampleType* ESAnalyserGetNormalisedHistogram(ESAnalyser *analyser)
{
	ESAnalyserNormaliseHistogram(analyser, analyser->options.histogram.array, analyser->options.histogram.num_of_points);
	return analyser->options.histogram.normalised_array;
}


MySampleType* ESAnalyserGetHistogramTrueVal(ESAnalyser *analyser)
{
	return analyser->options.histogram.array;
}

MySampleType* ESAnalyserSaveHistogram(ESAnalyser *analyser, bool options)
{
	ESAnalyserNormaliseHistogram(analyser, analyser->options.histogram.array, analyser->options.histogram.num_of_points);
	FILE *f = fopen("histogram.txt", "a+");

	if (options == true)
	{
		fprintf(f, "----------------------\n");
		fprintf(f, "HISTOGRAM DATA:\n\n");
		fprintf(f, "High dB: %f\n", analyser->options.histogram.high);
		fprintf(f, "Low dB: %f\n", analyser->options.histogram.low);
		fprintf(f, "dB interval: %f\n", analyser->options.histogram.interval);
		fprintf(f, "Num of points: %d\n\n", analyser->options.histogram.num_of_points);
	}
	
	for (int i = 0; i < analyser->options.histogram.num_of_points; ++i)
	{
		fprintf(f, "%f\n", analyser->options.histogram.normalised_array[i]);
	}

	if (options == true)
	{
		fprintf(f, "\n----------------------\n");
		fprintf(f, "END OF HISTOGRAM\n\n");
	}
	fclose(f);
}

int ESAnalyserGetHistogramSegments(ESAnalyser* analyser)
{
	return analyser->options.histogram.num_of_points;
}

void ESAnalyserResetHistogram(ESAnalyser *analyser)
{
	for (int i = 0; i < analyser->options.histogram.num_of_points; ++i)
	{
		analyser->options.histogram.array[i] = 0;
	}
}

#include <stdint.h>
#include "../analyser.h"
#include "../analyser_internal.h"
//#include "levelmeter.h"


bool ESAnalyserSetLevelmeter(ESAnalyser *analyser, ESAnalyserOptions *options)
{
	analyser->options.levelmeter.smoothing_coefficient = options->levelmeter.smoothing_coefficient;
	analyser->options.levelmeter.peak_drop_off = options->levelmeter.peak_drop_off;

	analyser->options.levelmeter.reducing_peak = 0;
	analyser->options.levelmeter.peak = 0;
	analyser->options.levelmeter.peak_average = 0;
	
	analyser->options.levelmeter.rms = 0;
	analyser->options.levelmeter.one_pole_peak = 0;
	analyser->options.levelmeter.average = 0;

	analyser->options.levelmeter.three_pole_average = 0;
	analyser->options.levelmeter._firstAverage = 0; 
	analyser->options.levelmeter._secondAverage = 0;
	analyser->options.levelmeter._thirdAverage = 0;

	return true;
}

MySampleType ESAnalyserCurrentPeakFall(ESAnalyser *analyser)
{
	if (analyser->options.levelmeter.reducing_peak < analyser->options.levelmeter.peak)
	{
		analyser->options.levelmeter.reducing_peak = analyser->options.levelmeter.peak;
		return analyser->options.levelmeter.reducing_peak;
	}else
	{
		analyser->options.levelmeter.reducing_peak *= analyser->options.levelmeter.peak_drop_off;
		return analyser->options.levelmeter.reducing_peak;
	}
}


MySampleType ESAnalyserRMS(MySampleType *samples, unsigned int num_of_samples) 
{
	MySampleType squared_value = 0.0f;
	MySampleType average = 0.0f;

	for (unsigned i = 0; i < num_of_samples; i++) 
	{
		squared_value = pow(samples[i], 2);
		average += squared_value;
	}

	if (average==0)
	{
		return average;
	}
	average /= num_of_samples;
	average = sqrt(average);
	
	return average;
}

MySampleType ESAnalyserRMSLoudness(ESAnalyser *analyser, MySampleType *samples, unsigned int sample_count) 
{
	samples = ESAnalyserGetLoudnessSamples(analyser, samples, sample_count);
	return ESAnalyserRMS(samples, sample_count);
}

MySampleType ESAnalyserOnePolePeak(ESAnalyser* analyser, MySampleType *samples, unsigned int sample_count)
{
	MySampleType peak = 0.0f;
	MySampleType absolute_value = 0.0f;

	for (unsigned i = 0; i < sample_count; i++) 
	{
		absolute_value = fabsf(samples[i]);
		peak = (absolute_value > peak) ? absolute_value : peak;
	}
	
	analyser->options.levelmeter.peak_average += (peak - analyser->options.levelmeter.peak_average) * analyser->options.levelmeter.smoothing_coefficient;
	
	return analyser->options.levelmeter.peak_average;
}

MySampleType ESAnalyserOnePolePeakLoudness(ESAnalyser* analyser, MySampleType *samples, unsigned sample_count)
{
	samples = ESAnalyserGetLoudnessSamples(analyser, samples, sample_count);
	return ESAnalyserOnePolePeak(analyser, samples, sample_count);
}

MySampleType ESAnalyserPeak(ESAnalyser* analyser, MySampleType *samples, unsigned sample_count)
{
	MySampleType peak = 0.0f;
	MySampleType absolute_value;
	
	for (unsigned i = 0; i < sample_count; i++) 
	{
		absolute_value = fabsf(samples[i]);
		peak = (absolute_value > peak) ? absolute_value : peak;
	}
	
	analyser->options.levelmeter.peak = peak;
	return analyser->options.levelmeter.peak;
}

MySampleType ESAnalyserPeakLoudness(ESAnalyser* analyser, MySampleType *samples, unsigned sample_count)
{
	samples = ESAnalyserGetLoudnessSamples(analyser, samples, sample_count);
	return ESAnalyserPeak(analyser, samples, sample_count);
}

MySampleType ESAnalyserSimpleAverage(MySampleType *samples, unsigned int sample_count)
{
	MySampleType value = 0.0f;
	MySampleType average = 0.0f;

	for (unsigned i = 0; i < sample_count; i++) 
	{
		value = samples[i];
		average += value;
	}
	average /= sample_count;
	return average;
}

MySampleType ESAnalyserThreePoleAverage(ESAnalyser* analyser, MySampleType *samples, unsigned int sample_count)
{
	MySampleType value = 0.0f;
	MySampleType average = 0.0f;

	for (unsigned i = 0; i < sample_count; i++) 
	{
		value = fabsf(samples[i]);
		average += value;
	}

	average /= sample_count;
	
	analyser->options.levelmeter.three_pole_average = (average + analyser->options.levelmeter._thirdAverage + analyser->options.levelmeter._secondAverage + analyser->options.levelmeter._firstAverage)/4;
	
	analyser->options.levelmeter._firstAverage = analyser->options.levelmeter._secondAverage;
	analyser->options.levelmeter._secondAverage = analyser->options.levelmeter._thirdAverage;
	analyser->options.levelmeter._thirdAverage = analyser->options.levelmeter.three_pole_average;
	
	return analyser->options.levelmeter.three_pole_average;
}
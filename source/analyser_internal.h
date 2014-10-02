#ifndef ANALYSER_INTERNAL_H
#define ANALYSER_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../libraries/timer/prof_time.h" //timers
#include "../libraries/level/single.h" //R128

typedef struct input input;

struct input {
	uint8_t *array;
	struct input* next;
};

struct ESAnalyser
{
	ESAnalyserOptions options;
	
	unsigned int points_to_store;
	double* magArray;

	input* head;
	input* cur;

	MyLibDrawingCallback callback;

	int sample_rate;
	MySampleType crossing_points;
	MySampleType old_crossing_points;

	int counter;
	LevelAudioBlockSingleBiquad biquad;
};

/** @brief  Saves the options into the internal analyser object, hiding the data from the client

@param[in] analyser The audio samples
@param[in] chunk_start the sample which the averaging starts
@param[in] chunk_start the sample which the averaging ends
@ingroup internal
*/
bool ESAnalyserSetLevelmeter(ESAnalyser *analyser, ESAnalyserOptions *options);
bool ESAnalyserSetHistogram(ESAnalyser *analyser, ESAnalyserOptions *options);
bool ESAnalyserSetSpectrogram(ESAnalyser *analyser, ESAnalyserOptions *options);
bool ESAnalyserSetWaveform(ESAnalyser *analyser, ESAnalyserOptions *options);


/** @brief  Returns the average of the samples passed in, within a specified region

@param[in] samples The audio samples
@param[in] chunk_start the sample which the averaging starts
@param[in] chunk_start the sample which the averaging ends
@ingroup internal
*/
MySampleType ESAnalyserChunkAverage(MySampleType* samples, unsigned int chunk_start, unsigned int chunk_end);

#ifdef __cplusplus
}
#endif

#endif /* ANALYSER_INTERNAL_H */
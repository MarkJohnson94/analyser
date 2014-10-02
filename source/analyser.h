/* -------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------
                                        
                                    	Analyser
                              	The Audio Analysis Library
                              	  Mark Johnson - EarSoft

----------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */

#ifndef ANALYSER_H
#define ANALYSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef float MySampleType;
typedef struct ESAnalyser ESAnalyser;
typedef struct ESTuple ESTuple;
typedef struct ESAnalyserOptions ESAnalyserOptions;
typedef struct input input;
typedef void (*MyLibDrawingCallback)(uint8_t *array, int current_array, int total_arrays);

/*							ESAnalyserOptions							*/
//This structure is used by the client app to specify which features of the 
//library they plan to use and options for how the library features will run
//
// Below sets the histogram to output between 0 to -60dB, interval, period
// and sample rate, and sets other areas as false, ie not being used.
//
//	options1.histogram.active = true;
//	options1.histogram.high = 0.0f;
//	options1.histogram.low = -60.0f;
//	options1.histogram.interval = 1.0f;
//	options1.histogram.period = 400.0f;
//	options1.histogram.sample_rate = sample_rate;
//	options1.waveform.active = false;
//	options1.levelmeter.active = false;
//  options1.spectrogram.active = false;
//
/*							ESAnalyserOptions							*/

struct ESAnalyserOptions{
struct {
	bool active;

	MySampleType *array;
	MySampleType *normalised_array;
	MySampleType period;
	MySampleType high;
	MySampleType low;
	MySampleType interval;
	MySampleType old_time_covered;

	MySampleType sample_rate;

	int num_of_points;
	long int count;
	long int times_called;
	long int total_times_called;
} histogram;
struct {
	bool active;

	unsigned int sample_rate;
	unsigned int channel_count;
	unsigned int chunk_size;

	unsigned int num_of_intervals;
	unsigned int num_of_points;
	unsigned int num_of_seconds;
	
	unsigned int points_to_store;
	unsigned int to_store;
	unsigned int next_value_count;
	unsigned int current_count;

	MySampleType time_to_deliver;

	MySampleType *internal_buffer;
	MySampleType *next_value_buffer;
} waveform;
struct {
	bool active;

	MySampleType smoothing_coefficient;
	MySampleType peak_drop_off;

	MySampleType reducing_peak;
	MySampleType peak;
	MySampleType peak_average;

	MySampleType rms;
	MySampleType one_pole_peak;
	MySampleType average;
	
	MySampleType three_pole_average;

	//should be private variables
	MySampleType _firstAverage;
	MySampleType _secondAverage;
	MySampleType _thirdAverage;
} levelmeter;
struct{
	bool active;

	double overlap;
	int counter;
	double* array;
}spectrogram;
struct{
	MyLibDrawingCallback callback;
}lib;
};

/** @brief  Creates the ESAnalyser object used internally by the library

These objects have a short life, it is expected you will create and free an object for each new
audio file. If you chose to change the options you have specified, you will have to create a new
object for these changes to be saved internally and affect the output of the library.

Takes in the options and saves them into the analyser object, used internally by the library and
hidden from the end user.

@param[in] analyser The ESAnalyser object
@param[in] options The ESAnalyserOptions specified by the client
@param[in] sample_rate The sample rate per second of the audio file (typically 44100)
@return true if memory is successfully allocated and data is saved, false if not
@ingroup analyser
*/
bool ESAnalyserCreate(ESAnalyser **analyser, ESAnalyserOptions *options, unsigned int sample_rate);

/** @brief  Destroys the ESAnalyser object used internally by the library

Memory leakage can occur if you do not destroy the object after it's use.

@param[in] analyser The ESAnalyser object
@return true if object is successfully destroyed, and false if an error occurs
@ingroup analyser
*/
bool ESAnalyserDestroy(ESAnalyser *analyser);


/* -------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------
									SPECTROGRAM FUNCTIONS
----------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */

void printTheLinkedList(ESAnalyser* analyser, int sample_rate);
void MotionAdvance(ESAnalyser* analyser);

/** @brief  Main spectrogram method, pass each chunk of audio data into this method

Passes the samples into the fftw library, storing a magnitude array from the fftw process.

@param[in] analyser The ESAnalyser object
@param[in] samples The audio samples
@param[in] num_of_samples The number of audio samples
@ingroup spectrogram
*/
void ESAnalyserSpectrogram(ESAnalyser* analyser, MySampleType* samples, int num_of_samples);

/** @brief  Returns the spectrogram array

@param[in] analyser The ESAnalyser object
@return spectrogram_array The spectrogram array
@ingroup spectrogram
*/
double* ESAnalyserReturnSpectrogram(ESAnalyser* analyser);

/** @brief  Returns the number of spectrogram arrays

@param[in] analyser The ESAnalyser object
@return val The number of spectrogram arrays
@ingroup histogram
*/
int ESAnalyserSpectrogramCount(ESAnalyser* analyser);


uint8_t* ESAnalyserGetSpectrogramArray(ESAnalyser* analyser, int position);


/* -------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------
									WAVEFORM FUNCTIONS
----------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */

/** @brief  Main waveform method, pass each chunk of audio data into this method

Adds samples into the waveform array which can then be displayed using the return waveform method.

@param[in] analyser The ESAnalyser object
@param[in] samples The audio samples
@return void
@ingroup waveform
*/
void ESAnalyserWaveform(ESAnalyser* analyser, MySampleType* samples);

/** @brief  Returns the waveform for the client to draw to the screen

@param[in] analyser The ESAnalyser object
@return array The waveform array to be drawn to the screen 
@ingroup waveform
*/
MySampleType* ESAnalyserReturnWaveform(ESAnalyser *analyser);

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------
									HISTOGRAM FUNCTIONS
----------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */

/** @brief  Main histogram method, pass each chunk of audio data into this method

Converts the samples using R128 loudness curve, before converting to dB values within the specified
range. The histogram data is updated each time the period has elapsed, eg 400ms so that it can then
be drawn to the screen.

@param[in] analyser The ESAnalyser object
@param[in] samples The audio samples
@param[in] num_of_samples The number of audio samples
@ingroup histogram
*/
void ESAnalyserHistogram(ESAnalyser *analyser, MySampleType* samples, unsigned int num_of_samples);

/** @brief  Normalises the histogram data between 0 and 1.

@param[in] analyser The ESAnalyser object
@param[in] samples The audio samples
@param[in] num_of_samples The number of audio samples
@ingroup histogram
*/
void ESAnalyserNormaliseHistogram(ESAnalyser* analyser, MySampleType* samples, int num_of_samples);

/** @brief  Returns the histogram for the client to draw to the screen

@param[in] analyser The ESAnalyser object
@return array The histogram array to be drawn to the screen 
@ingroup histogram
*/
MySampleType* ESAnalyserGetHistogram(ESAnalyser *analyser);

/** @brief  Returns normalised histogram using most recent data

If you use this method instead of ESAnalyserGetHistogram, then your histogram will update its view
every frame, whereas ESAnalyserGetHistogram will only update as often as the interval period.

@param[in] analyser The ESAnalyser object
@return array The histogram array to be drawn to the screen
@ingroup histogram
*/
MySampleType* ESAnalyserGetNormalisedHistogram(ESAnalyser *analyser);

/** @brief  Returns the non-normalised histogram

Maximum value of the true histogram depends on how many averages are taken of each audio chunk.
Each time an audio chunk average falls in a certain bin, the array position of this bin's value is
incremented by 1. 

@param[in] analyser The ESAnalyser object
@return array The non-normalised histogram array to be drawn to the screen
@ingroup histogram
*/
MySampleType* ESAnalyserGetHistogramTrueVal(ESAnalyser *analyser);

/** @brief  Resets the values in the histogram

Allows you to manually reset the histogram, histogram continues from fresh after this point.
Allows you to see the difference between two sections by resetting the histogram between them.

@param[in] analyser The ESAnalyser object
@return void
@ingroup histogram
*/
void ESAnalyserResetHistogram(ESAnalyser *analyser);

/** @brief  Saves the values of the histogram into a text file

Set bool to true to print out information from the histogram such as dB values and bin sizes.
Set bool to false to print out only the values of the histogram

@param[in] analyser The ESAnalyser object
@param[in] options Set true to output extra histogram information
@return void
@ingroup histogram
*/
MySampleType* ESAnalyserSaveHistogram(ESAnalyser *analyser, bool options);

/** @brief  Returns the number of intervals in the analyser objects histogram

@param[in] analyser The ESAnalyser object
@return int The number of bins in the histogram
@ingroup histogram
*/
int ESAnalyserGetHistogramSegments(ESAnalyser *analyser);

/** @brief  Converts the linear value and returns a dB value normalised between 0 and 1

Range must be > 0. Peak must be < 0. The lowest dB value allowed is -144. 
Therefore if range-peak > 144, then range will be set to the largest value where range = 144+peak.
Will return between 0 and 1.

@param[in] value The linear value
@param[in] peak The highest dB value
@param[in] range The range of dB values
@return void
*/
MySampleType ESAnalyserLinearToDB(MySampleType value, unsigned* peak, unsigned* range);

/** @brief  Converts the linear value and returns a dB value within a user defined dB range

Range must be > 0. Peak must be < 0. The lowest dB value allowed is -144. 
Therefore if range-peak > 144, then range will be set to the largest value where range = 144+peak.

@param[in] value The linear value
@param[in] peak The highest dB value
@param[in] range The range of dB values
@return void
*/
MySampleType ESAnalyserLinearToDBValue(MySampleType value, unsigned* peak, unsigned* range);

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------
									LEVELMETER FUNCTIONS
----------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
/** @brief  Returns the Root Mean Squared value of the samples

@param[in] samples The audio samples
@param[in] num_of_samples The number of audio samples
@return rms_value The RMS value
@ingroup levelmeter
*/
MySampleType ESAnalyserRMS(MySampleType *samples, unsigned int num_of_samples);

/** @brief  Returns the Root Mean Squared value of the samples after being processed with the 
			R128 loudness curve

@param[in] analyser The ESAnalyser object
@param[in] samples The audio samples
@param[in] num_of_samples The number of audio samples
@return rms_value The RMS value
@ingroup levelmeter
*/
MySampleType ESAnalyserRMSLoudness(ESAnalyser *analyser, MySampleType *samples, unsigned int num_of_samples);

/** @brief  Returns the One Pole Peak value of the samples

@param[in] analyser The ESAnalyser object
@param[in] samples The audio samples
@param[in] num_of_samples The number of audio samples
@return one_pole_peak_value The one pole peak value
@ingroup levelmeter
*/
MySampleType ESAnalyserOnePolePeak(ESAnalyser* analyser, MySampleType *samples, unsigned int num_of_samples);

/** @brief  Returns the One Pole Peak value of the samples after being processed with the R128 
			loudness curve

@param[in] analyser The ESAnalyser object
@param[in] samples The audio samples
@param[in] num_of_samples The number of audio samples
@return one_pole_peak_value The one pole peak value
@ingroup levelmeter
*/
MySampleType ESAnalyserOnePolePeakLoudness(ESAnalyser* analyser, MySampleType *samples, unsigned int num_of_samples);

/** @brief  Returns the peak value from the samples

@param[in] analyser The ESAnalyser object
@param[in] samples The audio samples
@param[in] num_of_samples The number of audio samples
@return peak_value The peak value
@ingroup levelmeter
*/
MySampleType ESAnalyserPeak(ESAnalyser* analyser, MySampleType *samples, unsigned int num_of_samples);

/** @brief  Returns the peak value from the samples after being processed by the R128 loudness curve

@param[in] analyser The ESAnalyser object
@param[in] samples The audio samples
@param[in] num_of_samples The number of audio samples
@return peak_value The peak value
@ingroup levelmeter
*/
MySampleType ESAnalyserPeakLoudness(ESAnalyser* analyser, MySampleType *samples, unsigned int num_of_samples);

/** @brief  Returns the average value of the samples

@param[in] samples The audio samples
@return average_value The average value
@ingroup levelmeter
*/
MySampleType ESAnalyserSimpleAverage(MySampleType *samples, unsigned int sample_count);

/** @brief  Returns the three pole average value of the samples

@param[in] samples The audio samples
@return three_pole_average_value The three pole average value
@ingroup levelmeter
*/
MySampleType ESAnalyserThreePoleAverage(ESAnalyser* analysis_dll, MySampleType *samples, unsigned int sample_count);

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------
									GENERAL FUNCTIONS
----------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */

/** @brief  Returns the samples after being processed with the R128 loudness curve

@param[in] analyser The ESAnalyser object
@param[in] samples The audio samples
@return three_pole_average_value The three pole average value
@ingroup general
*/
MySampleType* ESGetLoudnessSamples(ESAnalyser *analyser, MySampleType* buffer, unsigned int sample_rate);


/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------
								MISC FUNCTIONS TO BE LOOKED AT
----------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */

struct ESTuple
{
	MySampleType *buffer;
	int size;
};

MySampleType* ESAnalyserNextValue(ESAnalyser *analyser);
bool ESAnalyserTruncate(ESTuple *aTuple, MySampleType *samples, unsigned int sample_count);
MySampleType ESAnalyserLongZeroCrossings(ESAnalyser* analyser, MySampleType *samples, unsigned int sample_count, unsigned global_counter);
MySampleType ESAnalyserNumOfZeroCrossingPoints(MySampleType *samples, unsigned int sample_count);
MySampleType ESAnalyserZeroCrossing(MySampleType *samples, unsigned int sample_count);


#ifdef __cplusplus
}
#endif

#endif /* ANALYSER_H */
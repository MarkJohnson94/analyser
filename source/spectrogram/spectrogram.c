#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fftw3.h>
#include "../analyser.h"
#include "../analyser_internal.h"
//#include "spectrogram.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

input* BuildLinkedList();

void MotionAdvance(ESAnalyser* analyser)
{
	input *ptr = analyser->head;
	FILE *f = fopen("output.txt", "a+");
	int count = 0;

	while(ptr != NULL)
	{
		//double *magArray = ESAnalyserReturnSpectrogram(analyser);
		analyser->callback(ptr->array, count, analyser->options.spectrogram.counter);
		//fprintf(f, "%" PRIu8 "\n", ptr->array[i]);
		ptr = ptr->next;
		count++;
	}

	fclose(f);
}

int ESAnalyserSpectrogramCount(ESAnalyser* analyser)
{
	return analyser->options.spectrogram.counter;
}

bool ESAnalyserSetSpectrogram(ESAnalyser *analyser, ESAnalyserOptions *options)
{
	analyser->options.spectrogram.overlap = options->spectrogram.overlap;
	analyser->options.spectrogram.counter = 0;

	double overlap_factor = analyser->options.spectrogram.overlap; //enter value between 0 and 1
	double lower_value = (1-overlap_factor)/2;
	double higher_value = 1-lower_value;
	int lower = (int)((double)analyser->sample_rate*lower_value);
	int upper = (int)((double)analyser->sample_rate*higher_value);

	analyser->options.spectrogram.array = createArrayDouble(analyser->sample_rate/2);

	analyser->head = BuildLinkedList();

	return true;
}

uint8_t* ESAnalyserGetSpectrogramArray(ESAnalyser* analyser, int position)
{
	input *ptr = analyser->head;
	//ptr = ptr->next;
	/*int counter = 0;
	while ((ptr != NULL) && (counter < position))
    {
        ptr = ptr->next;
        counter++;
    }*/
    return ptr->array;
    //uint8_t *an_array = createArrayUInt(1024);
    //return an_array;
}

double windowHamming(int index, int transform_size)
{
	return (0.54 - (0.46 * cos(2 * M_PI * (index / ((transform_size - 1) * 1.0)))));
}

input* BuildLinkedList() 
{
	input* head = NULL;
	head = (input *) malloc(sizeof(input));
	head->array = createArrayUInt(2048);
	head->next = NULL;
	return head;
}

void Push(input** head_ref, double* mag, int sample_rate) 
{
	input* new_node = (input *) malloc(sizeof(input));

	new_node->array = createArrayUInt(sample_rate/2);
    for (int i = 0; i < (sample_rate/2); ++i)
	{
		new_node->array[i] = (uint8_t)mag[i]; ///(sample_rate/UINT8_MAX)
	}

	new_node->next = *head_ref; // The '*' to dereferences back to the real head
	*head_ref = new_node; // ditto
}

void printLinkedList(input* head, int sample_rate, ESAnalyser* analyser)
{
	input *ptr = head;
	FILE *f = fopen("output.txt", "a+");
	
	fprintf(f, "START\n");
	while(ptr != NULL)
    {
    	fprintf(f, "\n -------Printing list Start------- \n");
        for (int i = 0; i < sample_rate/2; ++i)
		{
			fprintf(f, "%" PRIu8 "\n", ptr->array[i]);
		}
		 fprintf(f, "\n -------Printing list End------- \n");
        ptr = ptr->next;
    }
    fprintf(f, "\nEND\n");
   
    fclose(f);
}

void printTheLinkedList(ESAnalyser* analyser, int sample_rate)
{
	printLinkedList(analyser->head, sample_rate, analyser);
}

void ESAnalyserSpectrogram(ESAnalyser* analyser, MySampleType* samples, int num_of_samples)
{
	int size = sample_rate;
	int transform_size = size/2;
	int step_size = size/4;
    
	//50% overlap factor?
	double overlap_factor = analyser->options.spectrogram.overlap; //enter value between 0 and 1
	double lower_value = (1-overlap_factor)/2;
	double higher_value = 1-lower_value;
	int lower = (int)((double)size*lower_value);
	int upper = (int)((double)size*higher_value);

	double *array = (double *) malloc(size*sizeof(double));

	double *initialArray1 = (double *) malloc(upper*sizeof(double));
	double *initialArray2 = (double *) malloc(upper*sizeof(double));
	
	for (int i = 0; i < upper; i++)
	{
		initialArray1[i] = (double)samples[i];
		initialArray2[i] = (double)samples[i+lower];
	}
	
	for(int i = 0; i < upper; i++)
	{
		initialArray1[i] *= windowHamming(i, upper);
		initialArray2[i] *= windowHamming(i, upper);
	}

	//populate array with the two windowed inputs
	for (int i = 0; i < size; i++)
	{
		if ((i >= lower) && (i < upper))
		{
			array[i] = initialArray1[i] + initialArray2[i-lower];
		}else if (i < lower)
		{
			array[i] = initialArray1[i];
		}else if (i >= upper)
		{
			array[i] = initialArray2[i-lower];
		}
	}

	double *out,*mag,*phase;
	double real,imag;
	int i,j;
	fftw_complex *out_cpx, *mod_cpx;
	fftw_plan fft;
	fftw_plan ifft;

	//Allocate Memory
	out_cpx = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*(size/2+1));
	mod_cpx = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*(size/2+1));
	out = (double *) malloc(size*sizeof(double));
	mag = (double *) malloc((size)*sizeof(double));
	phase = (double *) malloc(size*sizeof(double));

	fft = fftw_plan_dft_r2c_1d(size, array, out_cpx, FFTW_ESTIMATE);  //Setup fftw plan for fft 1 dimensional, real signal
	ifft = fftw_plan_dft_c2r_1d(size, mod_cpx, out, FFTW_ESTIMATE);   //Setup fftw plan for ifft 1 dimensional, complex signal

	fftw_execute(fft);	//perform fft
	for(j=0; j<size/2+1; j++)
	{
	    real = out_cpx[j][0];	//Extract real component
		imag = out_cpx[j][1];   //Extract imaginary component
		mag[j] = sqrt((real*real)+(imag*imag));  // Calculate the Magnitude
		phase[j] = atan2(imag,real); // Calculate the Phase
	}

	for (int i = 0; i < (size/2); i++)
	{
		analyser->options.spectrogram.array[i] = mag[i];
	}
	
	//**********MODIFICATION****************************//
	//You can perform frequency domain modification here//
	//**************************************************//

	for(j=0;j<size/2+1;j++)
	{
		mod_cpx[j][0] = (mag[j]*cos(phase[j]));  //Construct new real component
		mod_cpx[j][1] = (mag[j]*sin(phase[j]));  //Construct new imaginary  component
	}

	fftw_execute(ifft); //perform ifft

	// Print input and output
	for(i=0;i<size;i++)
	{
		out[i] = out[i]/size;
	}

	/*FILE *f = fopen("textfile2.txt", "a+");
	fprintf(f, "%d\n", analyser->head);
	fprintf(f, "0x%" PRIXPTR "\n", (uintptr_t)analyser->head);
	fprintf(f, "\n");
	fprintf(f, "%d\n", analyser->cur);
	fprintf(f, "0x%" PRIXPTR "\n", (uintptr_t)analyser->cur);
	fprintf(f, "\n");
	fclose(f);*/

	// Free all memory
	fftw_destroy_plan(fft);
	fftw_destroy_plan(ifft);
	fftw_free(out_cpx);
	fftw_free(mod_cpx);
	free(out);
	free(mag);
	free(phase);

	analyser->options.spectrogram.counter++;
}

double* ESAnalyserReturnSpectrogram(ESAnalyser *analyser)
{
	return analyser->options.spectrogram.array;
}
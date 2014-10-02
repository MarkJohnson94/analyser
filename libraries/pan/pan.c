//#define _POSIX_C_SOURCE 199309L
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "pan.h"

typedef struct ESPan ESPan;

MySampleType* ESPanCreateArray(unsigned int array_size)
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

bool ESPanInitialise(ESPan **pan, unsigned int sample_count)
{
	ESPan *new_pan = (ESPan *)malloc(sizeof(ESPan));
    if (new_pan == NULL)
	{
		return false;
	}
	
	new_pan->ll = 0;
	new_pan->lr = 0;
	new_pan->rr = 0;
	new_pan->centerbuffer = ESPanCreateArray(sample_count);
	
	*pan = new_pan;
	return true;
}

bool ESPanCreate(ESPan **pan, unsigned int sample_count)
{
	return ESPanInitialise(pan, sample_count);
}

bool ESPanDestroy(ESPan *pan)
{
	free(pan->centerbuffer);
    free(pan);
    return true;
}

MySampleType ESPanGetPan(ESPan* pan, MySampleType** samples, unsigned int sample_count)
{	
	MySampleType error_const = 0.1;
	MySampleType inL, inR;
	MySampleType lllrdiff, rrlrdiff, temp;
	MySampleType ll = pan->ll;
	MySampleType lr = pan->lr;
	MySampleType rr = pan->rr;
	MySampleType pan_value = 0.5;
	
	for (unsigned i = 0; i < sample_count; i++)
	{
		inL = samples[0][i];
		inR = samples[1][i];
		ll = ll + (inL * inL-ll)*error_const;
		lr = lr + (inL * inR-lr)*error_const;
		rr = rr + (inR * inR-rr)*error_const;
		lllrdiff = fabs(ll-lr);
		rrlrdiff = fabs(rr-lr);

		temp = 0.5 + (lllrdiff-rrlrdiff);
		
		temp = (temp > 1) ? 1 : temp;
		temp = (temp < 0) ? 0 : temp;

		/*if (temp > 1)
		{
			temp = 1;
		}else if (temp < 0)
		{
			temp = 0;
		}*/

		pan_value = pan_value + (temp-pan_value)*error_const;
		//pan->centerbuffer[i] = inL*pan_value + inR*(1-pan_value);
		//samples[0][i] = inL - pan->centerbuffer[i];
		//samples[1][i] = inR - pan->centerbuffer[i];
	}

	//seems to get a bigger spread with this, may not be as accurate though
	/*if (pan_value < 0.5f)
	{
		MySampleType aVal = 0.5f-pan_value;
		pan_value -= aVal;
	}else
	{
		MySampleType aVal = pan_value-0.5f;
		pan_value += aVal;
	}*/

	pan->ll = ll;
	pan->lr = lr;
	pan->rr = rr;
	
	//return (pan->centerbuffer);
	return pan_value;
}
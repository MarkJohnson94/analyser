//analyser.h
#ifndef PAN_H
#define PAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "../../typedef.h"

typedef struct ESPan ESPan;

struct ESPan
{
	MySampleType* centerbuffer;
	MySampleType ll;
	MySampleType lr;
	MySampleType rr;
};

typedef struct ESPan ESPan;

MySampleType* ESPanCreateArray(unsigned int array_size);

bool ESPanInitialise(ESPan **pan, unsigned int sample_count);

bool ESPanCreate(ESPan **pan, unsigned int sample_count);

bool ESPanDestroy(ESPan *pan);

MySampleType ESPanGetPan(ESPan* pan, MySampleType** samples, unsigned int sample_count);

#ifdef __cplusplus
}
#endif

#endif /* PAN_H */
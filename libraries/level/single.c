#include "single.h"
#include <math.h>

//#ifdef cplusplus include etc..

typedef float LevelAudioValue;
#define LAF_PI 3.141592653589793
#define LAF_SQRT_2 1.4142135623730951

void LevelAudioBlockSingleBiquadCreate(LevelAudioBlockSingleBiquad *biquad) {

    biquad->previous.d1 = (0);
    biquad->previous.d2 = (0);
}

void LevelAudioBlockSingleBiquadSetParams(LevelAudioBlockSingleBiquad *biquad,
                                                        LevelAudioBlockSingleBiquadParams *params) {

    LevelAudioValue v = powf(10, (fabs(params->gain)) / 20);
    LevelAudioValue k = tan(params->freq_norm * LAF_PI);
    LevelAudioValue kk = k * k;
    LevelAudioValue k_mul_sqrt_of_2v = k * sqrt(2 * v);

    LevelAudioValue norm;
    switch (params->type) {

        case LevelAudioBlockBiquadTypeLowPass:

            norm = 1 / (1 + k / params->quality + kk);
            biquad->coeffs.b0 = kk * norm;
            biquad->coeffs.b1 = 2 * biquad->coeffs.b0;
            biquad->coeffs.b2 = biquad->coeffs.b0;
            biquad->coeffs.a1 = 2 * (1 - kk) * norm;
            biquad->coeffs.a2 = (k / params->quality - kk - 1) * norm;
            break;

        case LevelAudioBlockBiquadTypeHighPass:

            norm = 1 / (1 + k / params->quality + kk);
            biquad->coeffs.b0 = norm;
            biquad->coeffs.b1 = -2 * biquad->coeffs.b0;
            biquad->coeffs.b2 = biquad->coeffs.b0;
            biquad->coeffs.a1 = 2 * (1 - kk) * norm;
            biquad->coeffs.a2 = (k / params->quality - kk - 1) * norm;
            break;

        case LevelAudioBlockBiquadTypeBandPass:

            norm = 1 / (1 + k / params->quality + kk);
            biquad->coeffs.b0 = k / params->quality * norm;
            biquad->coeffs.b1 = 0;
            biquad->coeffs.b2 = -biquad->coeffs.b0;
            biquad->coeffs.a1 = 2 * (1 - kk) * norm;
            biquad->coeffs.a2 = (k / params->quality - kk - 1) * norm;
            break;

        case LevelAudioBlockBiquadTypeNotch:

            norm = 1 / (1 + k / params->quality + kk);
            biquad->coeffs.b0 = (1 + kk) * norm;
            biquad->coeffs.b1 = 2 * (kk - 1) * norm;
            biquad->coeffs.b2 = biquad->coeffs.b0;
            biquad->coeffs.a1 = -biquad->coeffs.b1;
            biquad->coeffs.a2 = (k / params->quality - kk - 1) * norm;
            break;

        case LevelAudioBlockBiquadTypePeak:
            if (params->gain >= 0) {
                norm = 1 / (1 + k / params->quality + kk);
                biquad->coeffs.b0 = (1 + v * k/ params->quality + kk) * norm;
                biquad->coeffs.b1 = 2 * (kk - 1) * norm;
                biquad->coeffs.b2 = (1 - v * k / params->quality + kk) * norm;
                biquad->coeffs.a1 = -biquad->coeffs.b1;
                biquad->coeffs.a2 = (k / params->quality - kk - 1) * norm;
            } else {
                norm = 1 / (1 + v * k / params->quality + kk);
                biquad->coeffs.b0 = (1 + k / params->quality + kk) * norm;
                biquad->coeffs.b1 = 2 * (kk - 1) * norm;
                biquad->coeffs.b2 = (1 - k / params->quality + kk) * norm;
                biquad->coeffs.a1 = -biquad->coeffs.b1;
                biquad->coeffs.a2 = (v * k / params->quality - kk - 1) * norm;
            }
            break;

        case LevelAudioBlockBiquadTypeLowShelf:

            if (params->gain >= 0) {
                norm = 1 / (1 + LAF_SQRT_2 * k / params->quality + kk);
                biquad->coeffs.b0 = (1 + k_mul_sqrt_of_2v / params->quality + v * kk) * norm;
                biquad->coeffs.b1 = 2 * (v *kk - 1) * norm;
                biquad->coeffs.b2 = (1 - k_mul_sqrt_of_2v / params->quality + v * kk) * norm;
                biquad->coeffs.a1 = 2 * (1 - kk) * norm;
                biquad->coeffs.a2 = (LAF_SQRT_2 * k / params->quality - kk - 1) * norm;
            } else {
                norm = 1 / (1 + k_mul_sqrt_of_2v / params->quality + v * kk);
                biquad->coeffs.b0 = (1 + LAF_SQRT_2 * k / params->quality + kk) * norm;
                biquad->coeffs.b1 = 2 * (k * k - 1) * norm;
                biquad->coeffs.b2 = (1 - LAF_SQRT_2 * k / params->quality + kk) * norm;
                biquad->coeffs.a1 = 2 * (1 - v * kk) * norm;
                biquad->coeffs.a2 = (k_mul_sqrt_of_2v / params->quality - v * kk - 1) * norm;
            }

            break;

        case LevelAudioBlockBiquadTypeHighShelf:

            if (params->gain >= 0) {
                norm = 1 / (1 + LAF_SQRT_2 * k / params->quality +kk);
                biquad->coeffs.b0 = (v + k_mul_sqrt_of_2v / params->quality + kk) * norm;
                biquad->coeffs.b1 = 2 * (kk - v) * norm;
                biquad->coeffs.b2 = (v - k_mul_sqrt_of_2v / params->quality + kk) * norm;
                biquad->coeffs.a1 = 2 * (1 - kk) * norm;
                biquad->coeffs.a2 = (LAF_SQRT_2 * k / params->quality - kk - 1) * norm;
            } else {
                norm = 1 / (v + k_mul_sqrt_of_2v / params->quality + kk);
                biquad->coeffs.b0 = (1 + LAF_SQRT_2 * k / params->quality + kk) * norm;
                biquad->coeffs.b1 = 2 * (kk - 1) * norm;
                biquad->coeffs.b2 = (1 - LAF_SQRT_2 * k / params->quality + kk) * norm;
                biquad->coeffs.a1 = 2 * (v - kk) * norm;
                biquad->coeffs.a2 = (k_mul_sqrt_of_2v / params->quality - kk - v) * norm;
            }
            break;

        default:
            break;
    }
}


inline LevelAudioValue LevelAudioBlockSingleBiquadProcess(LevelAudioBlockSingleBiquad *biquad,
                                                                 LevelAudioValue sample) {

    // Biquad filter using Transposed Direct Form II

    LevelAudioValue res;

    res                 = biquad->coeffs.b0 * sample + biquad->previous.d1;
    biquad->previous.d1 = biquad->coeffs.b1 * sample + biquad->coeffs.a1 * res + biquad->previous.d2;
    biquad->previous.d2 = biquad->coeffs.b2 * sample + biquad->coeffs.a2 * res;
	
    return res;
}
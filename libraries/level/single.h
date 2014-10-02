#ifndef _Level_Audio_Blocks_Single_Biquad_Header
#define _Level_Audio_Blocks_Single_Biquad_Header

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Common data for biquad filter block
@defgroup blocks_biquad Biquad filter block
@ingroup blocks */

/** @brief Types for biquad filter block
@ingroup blocks_biquad*/

typedef enum {
    LevelAudioBlockBiquadTypeLowPass,
    LevelAudioBlockBiquadTypeHighPass,
    LevelAudioBlockBiquadTypeBandPass,

    LevelAudioBlockBiquadTypeNotch,
    LevelAudioBlockBiquadTypePeak,

    LevelAudioBlockBiquadTypeLowShelf,
    LevelAudioBlockBiquadTypeHighShelf
} LevelAudioBlockBiquadType;


typedef float LevelAudioValue;

/** @brief Single Biquad

@defgroup blocks_single_biquad Single Biquad filter block
@ingroup blocks_biquad

*/

/** @brief Input parameter container for a single biquad filter block
@ingroup blocks_single_biquad
*/

typedef struct {
    LevelAudioBlockBiquadType type;

    LevelAudioValue freq_norm; // normalised (0 = 0Hz, 1 = FS)
    LevelAudioValue quality;
    LevelAudioValue gain;
} LevelAudioBlockSingleBiquadParams;

/** @brief Storage for a single biquad filter block
@ingroup blocks_single_biquad
*/

typedef struct {
    struct {
        LevelAudioValue b0;
        LevelAudioValue b1;
        LevelAudioValue b2;

        LevelAudioValue a1;
        LevelAudioValue a2;
    } coeffs;
    struct {
        LevelAudioValue d1;
        LevelAudioValue d2;
    } previous;
} LevelAudioBlockSingleBiquad;


void LevelAudioBlockSingleBiquadCreate(LevelAudioBlockSingleBiquad *biquad);

void LevelAudioBlockSingleBiquadSetParams(LevelAudioBlockSingleBiquad *biquad,
                                                        LevelAudioBlockSingleBiquadParams *params);

LevelAudioValue LevelAudioBlockSingleBiquadProcess(LevelAudioBlockSingleBiquad *biquad,
                                                          LevelAudioValue sample);


#ifdef __cplusplus
}
#endif

#endif // _Level_Audio_Blocks_Single_Biquad_Header
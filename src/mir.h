#ifndef LP_MIR_H
#define LP_MIR_H

#include "pippicore.h"

/**
 * Coyote onset detector ported with permission from 
 * the SuperCollider BatUGens by Batuhan Bozkurt.
 * https://github.com/earslap/SCPlugins
 */
typedef struct coyote_t {
    lpfloat_t track_fall_time;
    lpfloat_t slow_lag_time, fast_lag_time, fast_lag_mul;
    lpfloat_t thresh, min_dur;

    /* Constants */
    lpfloat_t log1, log01, log001;
    int samplerate;

    /* The remaining comments in this struct were copied 
     * from the original implementation. */

    /* attack and decay coefficients for the amplitude tracker. */
    lpfloat_t rise_coef, fall_coef;

    /* previous output value from the amplitude tracker */
    lpfloat_t prev_amp;

    /* coefficients for the parallel smoothers */
    lpfloat_t slow_lag_coef, fast_lag_coef;

    /* previous values of parallel smoother outputs, used in calculations */
    lpfloat_t slow_lag_prev, fast_lag_prev;

    lpfloat_t current_avg;
    lpfloat_t avg_lag_prev;
    long current_index;
    lpfloat_t avg_trig;

    int e_time;
    int gate;
} coyote_t;

typedef struct mir_pitch_factory_t {
    buffer_t * (*pitch)(buffer_t *, lpfloat_t);
} mir_pitch_factory_t;

typedef struct mir_onset_factory_t {
    coyote_t * (*coyote_create)(int samplerate);
    lpfloat_t (*coyote_process)(coyote_t * od, lpfloat_t sample);
    void (*coyote_destory)(coyote_t * od);
} mir_onset_factory_t;

extern const mir_pitch_factory_t PitchTracker;
extern const mir_onset_factory_t OnsetDetector;

#endif

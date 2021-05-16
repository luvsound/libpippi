#include "mir.h"

/**
 * Yin implementation ported from:
 * Patrice Guyot. (2018, April 19). Fast Python 
 * implementation of the Yin algorithm (Version v1.1.1). 
 * Zenodo. http://doi.org/10.5281/zenodo.1220947
 * https://github.com/patriceguyot/Yin
 *
 * Coyote onset detector ported with permission from 
 * the SuperCollider BatUGens by Batuhan Bozkurt.
 * https://github.com/earslap/SCPlugins
 */

long * yin_difference_function(buffer_t * x, size_t N, size_t tau_max) {
    long * df;
    size_t tau;
    size_t j;
    long tmp;

    df = (long *)MemoryPool.alloc(tau_max, sizeof(long));

    for(tau=1; tau < tau_max; tau++) {
        for(j=0; j < N - tau_max; j++) {
            tmp = (long)(x->data[j] - x->data[j + tau]);
            df[tau] += tmp * tmp;
        }
    }
    
    return df;
}

/* WIP
 *
 * cmndf = df[1:] * range(1, N) / np.cumsum(df[1:]).astype(float) #scipy method 
long * yin_cumulative_mean_normalized_difference_function(long * df, size_t df_length, size_t N) {
    long * cmndf;
    long prev;
    int i;
    long * numerator;
    long * denominator;

    numerator = (long *)MemoryPool.alloc(df_length, sizeof(long));
    denominator = (long *)MemoryPool.alloc(df_length, sizeof(long));

    for(i=1; i < df_length; i++) {
        numerator[i] = df[i] * i;
    }

    prev = 0;
    for(i=1; i < df_length; i++) {
        denominator[i] = df[i] + prev;
        prev = denominator[i];
    }

    for(i=1; i < df_length; i++) {
        cmndf[i] = numerator[i] / denominator[i];
    }

    return cmndf;
}*/

lpfloat_t yin_get_pitch(long * cmndf, size_t tau_min, size_t tau_max, lpfloat_t harmo_th) {
    return 0.f;
}

buffer_t * yin_pitch(buffer_t * buf, lpfloat_t threshold) {
    return buf;
}

coyote_t * coyote_create(int samplerate) {
    coyote_t * od;

    od = (coyote_t *)MemoryPool.alloc(1, sizeof(coyote_t));

    od->log1 = log(0.1f);
    od->log01 = log(0.01f);
    od->log001 = log(0.001f);

	od->track_fall_time = 0.2f;
    od->slow_lag_time = 0.2f;
    od->fast_lag_time = 0.01f;
    od->fast_lag_mul = 0.5f;
    od->thresh = 0.05f;
    od->min_dur = 0.1f;

    od->rise_coef = exp(od->log1/(0.001 * samplerate));
    od->fall_coef = exp(od->log1 / (od->track_fall_time * od->samplerate));
    od->slow_lag_coef = exp(od->log001 / (od->slow_lag_time * samplerate));
    od->fast_lag_coef = exp(od->log001 / (od->fast_lag_time * samplerate));

    od->slow_lag_prev = 0.f;
    od->fast_lag_prev = 0.f;

    od->prev_amp = 0.f;
    od->avg_lag_prev = 0.f;

    od->avg_trig = 0.f;
    od->current_avg = 0.f;
    od->current_index = 1;

    od->e_time = 0;
    od->gate = 1;

    od->samplerate = samplerate;

    return od;
}

lpfloat_t coyote_process(coyote_t * od, lpfloat_t sample) {
    lpfloat_t prev;
    lpfloat_t tracker_out, trig;
    lpfloat_t fast_val, slow_val;
    lpfloat_t avg_val, divi, out;

    od->fall_coef = exp(od->log1 / (od->track_fall_time * od->samplerate));
    od->slow_lag_coef = exp(od->log001 / (od->slow_lag_time * od->samplerate));
    od->fast_lag_coef = exp(od->log001 / (od->fast_lag_time * od->samplerate));

    prev = od->prev_amp;

    if(od->avg_trig) {
        od->current_avg = 0.f;
        od->current_index = 1;
    }

    /* Normally this would loop over a block of samples... */
    tracker_out = fabs(sample);
    if(tracker_out < prev) {
        tracker_out = tracker_out + (prev - tracker_out) * od->fall_coef;
    } else {
        tracker_out = tracker_out + (prev - tracker_out) * od->rise_coef;
    }

    divi = ((od->current_avg - tracker_out) / od->current_index);
    od->current_avg = od->current_avg - divi;
    od->current_index += 1;

    od->prev_amp = tracker_out;
    /* end loop */

    slow_val = od->slow_lag_prev = tracker_out + (od->slow_lag_coef * (od->slow_lag_prev - tracker_out));
    fast_val = od->fast_lag_prev = tracker_out + (od->fast_lag_coef * (od->fast_lag_prev - tracker_out));
    avg_val = od->avg_lag_prev = od->current_avg + (od->fast_lag_coef * (od->avg_lag_prev - od->current_avg));

    od->slow_lag_prev = zapgremlins(od->slow_lag_prev);
    od->fast_lag_prev = zapgremlins(od->fast_lag_prev);
    od->avg_lag_prev = zapgremlins(od->avg_lag_prev);

    trig = od->avg_trig = ((fast_val > slow_val) || (fast_val> avg_val)) * (tracker_out > od->thresh)  * od->gate;
    od->e_time += 1;

    out = trig;

    if((trig == 1) && (od->gate == 1)) {
        od->e_time = 0;
        od->gate = 0;
    }

    if((od->e_time > (od->samplerate * od->min_dur)) && (od->gate == 0)) {
        od->gate = 1;
    }

    return out;
}

void coyote_destroy(coyote_t * od) {
    MemoryPool.free(od);
}

const mir_pitch_factory_t PitchTracker = { yin_pitch };
const mir_onset_factory_t OnsetDetector = { coyote_create, coyote_process, coyote_destroy };


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

long * yin_difference_function(long * values, size_t N, size_t tau_max) {
    size_t tau;
    size_t j;
    long tmp;

    for(tau=1; tau < tau_max; tau++) {
        for(j=0; j < N - tau_max; j++) {
            tmp = (long)(values[j] - values[j + tau]);
            values[tau] += tmp * tmp;
        }
    }
    
    return values;
}

/**
 * `values` in this routine must be the result of the call to `yin_difference_function()`. 
 */
long * yin_cumulative_mean_normalized_difference_function(long * values, size_t N) {
    /* cmndf = df[1:] * range(1, N) / np.cumsum(df[1:]).astype(float) #scipy method */
    long prev, denominator, value;
    size_t i;

    prev = 0;
    for(i=1; i < N; i++) {
        denominator = values[i] + prev;
        value = (values[i] * i) / denominator;
        prev = denominator;
        values[i] = value;
    }

    return values;
}

/**
 * Return fundamental period of a frame based on CMND function.
 *
 * `values`: Cumulative Mean Normalized Difference function
 * `tau_min`: minimum period for speech
 * `tau_max`: maximum period for speech
 * `harmo_th`: harmonicity threshold to determine if it is necessary to compute pitch frequency
 *
 * returns: fundamental period if there is values under threshold, 0 otherwise
 */
lpfloat_t yin_get_pitch(long * values, size_t tau_min, size_t tau_max, lpfloat_t harmo_th) {
    size_t tau;

    tau = tau_min;
    while (tau < tau_max) {
        if(values[tau] < harmo_th) {
            while(tau + 1 < tau_max && values[tau + 1] < values[tau]) {
                tau += 1;
            }
            return tau;
        }
        tau += 1;
    }

    /* unvoiced */
    return 0.f;
}

buffer_t * yin_pitch(buffer_t * buf, lpfloat_t threshold) {
    size_t num_pitches, count, i, j;
    
    size_t w_len, w_step, tau_min, tau_max; 
    lpfloat_t f0_min, f0_max;
    lpfloat_t p, last_p;

    long * block;
    buffer_t * out; /* Frequencies for each analysis window. */

    /* assume one channel to start */
    assert(buf->channels == 1); 
 
    w_len = 512; /* analysis window size */
    w_step = 256; /* lag between consecutive windows in samples */
    f0_min = 20.f; /* Minimum possible frequency in hertz */
    f0_max = 1000.f; /* Maximum possible frequency in hertz */
    threshold = 0.1f; /* Threshold of detection. */
    p = 0.f; /* Yin result */
    last_p = 0.f; /* Last good freq -- set to default val */
    tau_min = (size_t)(buf->samplerate / f0_max);
    tau_max = (size_t)(buf->samplerate / f0_min);

    block = (long *)MemoryPool.alloc(w_len, sizeof(lpfloat_t));
    num_pitches = (buf->length - w_len) / w_step;
    out = Buffer.create(num_pitches, 1, buf->samplerate);

    count = 0;
    for(i=0; i < buf->length - w_len; i += w_step) {
        /* Copy values into temp buffer */
        for(j=0; j < w_step; j++) {
            block[j] = (long)(buf->data[i + j] * buf->samplerate); 
        }

        /* Compute Yin */
        yin_difference_function(block, w_len, tau_max);
        yin_cumulative_mean_normalized_difference_function(block, tau_max);
        p = yin_get_pitch(block, tau_min, tau_max, threshold);

        /* Copy any pitches found to output buffer. */
        if(p != 0) {
            out->data[count] = buf->samplerate / p;
            last_p = p;
        } else {
            out->data[count] = last_p;
        }

        count += 1;
    }

    return out;
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


#include "pippicore.h"
#include "oscs.sine.h"

lpsineosc_t * create_sineosc(void);
lpfloat_t process_sineosc(lpsineosc_t * osc);
lpbuffer_t * render_sineosc(lpsineosc_t * osc, size_t length, lpbuffer_t * freq, lpbuffer_t * amp, int channels);
void destroy_sineosc(lpsineosc_t * osc);

const lpsineosc_factory_t LPSineOsc = { create_sineosc, process_sineosc, render_sineosc, destroy_sineosc };

lpsineosc_t * create_sineosc(void) {
    lpsineosc_t * osc = (lpsineosc_t *)LPMemoryPool.alloc(1, sizeof(lpsineosc_t));
    osc->phase = 0;
    osc->freq = 220.0;
    osc->samplerate = 48000.0;
    return osc;
}

lpfloat_t process_sineosc(lpsineosc_t* osc) {
    lpfloat_t sample;
    
    sample = sin(PI2 * osc->phase);

    osc->phase += osc->freq * (1.0/osc->samplerate);

    while(osc->phase >= 1) {
        osc->phase -= 1;
    }

    return sample;
}

lpbuffer_t * render_sineosc(lpsineosc_t * osc, size_t length, lpbuffer_t * freq, lpbuffer_t * amp, int channels) {
    lpbuffer_t * out;
    lpfloat_t sample, _amp;
    size_t i, c;
    float pos;

    pos = 0.f;
    out = LPBuffer.create(length, channels, osc->samplerate);
    for(i=0; i < length; i++) {
        pos = (float)i/length;
        osc->freq = LPInterpolation.linear_pos(freq, pos);
        _amp = LPInterpolation.linear_pos(amp, pos);
        sample = process_sineosc(osc) * _amp;
        for(c=0; c < channels; c++) {
            out->data[i * channels + c] = sample;
        }
    }

    return out;
}

void destroy_sineosc(lpsineosc_t * osc) {
    LPMemoryPool.free(osc);
}



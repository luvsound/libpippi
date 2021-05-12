#include "pippicore.h"
#include "sineosc.h"

sineosc_t* create_sineosc(void) {
    sineosc_t* osc = (sineosc_t*)MemoryPool.alloc(1, sizeof(sineosc_t));
    osc->phase = 0;
    osc->freq = 220.0;
    osc->samplerate = 48000.0;
    return osc;
}

lpfloat_t process_sineosc(sineosc_t* osc) {
    lpfloat_t sample;
    
    sample = sin(PI2 * osc->phase);

    osc->phase += osc->freq * (1.0/osc->samplerate);

    while(osc->phase >= 1) {
        osc->phase -= 1;
    }

    return sample;
}

buffer_t * render_sineosc(sineosc_t * osc, size_t length, buffer_t * freq, buffer_t * amp, int channels) {
    buffer_t * out;
    lpfloat_t sample, _amp;
    size_t i, c;
    float pos;

    pos = 0.f;
    out = Buffer.create(length, channels, osc->samplerate);
    for(i=0; i < length; i++) {
        pos = (float)i/length;
        osc->freq = Interpolation.linear_pos(freq, pos);
        _amp = Interpolation.linear_pos(amp, pos);
        sample = process_sineosc(osc) * _amp;
        for(c=0; c < channels; c++) {
            out->data[i * channels + c] = sample;
        }
    }

    return out;
}

void destroy_sineosc(sineosc_t* osc) {
    MemoryPool.free(osc);
}


const sineosc_factory_t SineOsc = { create_sineosc, process_sineosc, render_sineosc, destroy_sineosc };

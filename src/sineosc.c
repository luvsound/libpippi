#include "pippicore.h"
#include "sineosc.h"

sineosc_t* create_sineosc(void) {
    sineosc_t* osc = (sineosc_t*)calloc(1, sizeof(sineosc_t));
    osc->phase = 0;
    osc->freq = 220.0;
    osc->samplerate = 48000.0;
    return osc;
}

lpfloat_t process_sineosc(sineosc_t* osc) {
    lpfloat_t sample;
    
#ifdef LP_FLOAT
    sample = sinf(PI2 * osc->phase);
#else
    sample = sin(PI2 * osc->phase);
#endif

    osc->phase += osc->freq * (1.0/osc->samplerate);

    while(osc->phase >= 1) {
        osc->phase -= 1;
    }

    return sample;
}

void destroy_sineosc(sineosc_t* osc) {
    free(osc);
}


const sineosc_factory_t SineOsc = { create_sineosc, process_sineosc, destroy_sineosc };

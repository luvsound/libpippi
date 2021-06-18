#include "pippi.h"

#define BS 4096
#define SR 48000
#define CHANNELS 2

int main() {
    size_t i, c, length, minlength, maxlength;
    lpbuffer_t * out;
    lpbuffer_t * sine;
    lpbuffer_t * sineamp;
    lpbuffer_t * sinefreq;
    lpbuffer_t * freqs;
    lpsineosc_t * sineosc;
    lptapeosc_t * osc;
    lpfloat_t freqphase, freqphaseinc;

    length = 10 * SR;
    minlength = SR/100.;
    maxlength = SR;

    sineosc = LPSineOsc.create();
    sinefreq = LPParam.from_float(80.f);
    sineamp = LPParam.from_float(0.2f);
    sine = LPSineOsc.render(sineosc, length, sinefreq, sineamp, CHANNELS);

    freqs = LPWindow.create("sine", BS);
    LPBuffer.scale(freqs, 0, 1, 1.f/maxlength, 1.f/minlength);

    out = LPBuffer.create(length, CHANNELS, SR);
    osc = LPTapeOsc.create(sine);
    osc->samplerate = SR;

    freqphaseinc = 1.f/freqs->length;
    freqphase = 0.f;

    for(i=0; i < length; i++) {
        osc->freq = LPInterpolation.linear(freqs, freqphase);
        LPTapeOsc.process(osc);
        for(c=0; c < CHANNELS; c++) {
            out->data[i * CHANNELS + c] = osc->current_frame->data[c];
        }
        freqphase += freqphaseinc;
        if(freqphase >= freqs->length) {
            freqphase -= freqs->length;
        }
    }

    LPSoundFile.write("renders/tapeosc-out.wav", out);

    LPSineOsc.destroy(sineosc);
    LPTapeOsc.destroy(osc);
    LPBuffer.destroy(out);
    LPBuffer.destroy(sine);
    LPBuffer.destroy(sinefreq);
    LPBuffer.destroy(sineamp);

    return 0;
}

#include "pippi.h"

#define BS 4096
#define SR 48000
#define CHANNELS 2

int main() {
    lpfloat_t amp;
    size_t i, c, length;
    lpbuffer_t * out;
    lpbuffer_t * sine;
    lpbuffer_t * sineamp;
    lpbuffer_t * sinefreq;
    lpsineosc_t * sineosc;
    lptapeosc_t * osc;

    amp = 0.2;
    length = 10 * SR;

    sineosc = LPSineOsc.create();
    sinefreq = LPParam.from_float(220.f);
    sineamp = LPParam.from_float(0.2f);
    sine = LPSineOsc.render(sineosc, length, sinefreq, sineamp, CHANNELS);

    out = LPBuffer.create(length, CHANNELS, SR);
    osc = LPTapeOsc.create(sine);
    osc->samplerate = SR;

    for(i=0; i < length; i++) {
        LPTapeOsc.process(osc);
        for(c=0; c < CHANNELS; c++) {
            out->data[i * CHANNELS + c] = osc->current_frame->data[c] * amp;
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

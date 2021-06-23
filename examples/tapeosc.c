#include "pippi.h"

#define BS 4096
#define SR 48000
#define CHANNELS 2

int main() {
    size_t i, c, length;
    lpbuffer_t * out;
    lpbuffer_t * sine;
    lpbuffer_t * sineamp;
    lpbuffer_t * sinefreq;
    lpbuffer_t * speeds;
    lpsineosc_t * sineosc;
    lptapeosc_t * osc;
    lpfloat_t speedphase, speedphaseinc;

    length = 10 * SR;

    sineosc = LPSineOsc.create();
    sinefreq = LPParam.from_float(80.f);
    sineamp = LPParam.from_float(0.2f);
    sine = LPSineOsc.render(sineosc, length, sinefreq, sineamp, CHANNELS);

    speeds = LPWindow.create("tri", BS);
    LPBuffer.scale(speeds, 0, 1, 0.5f, 2.f);

    out = LPBuffer.create(length, CHANNELS, SR);
    osc = LPTapeOsc.create(sine, SR);
    osc->samplerate = SR;

    speedphaseinc = 1.f/speeds->length;
    speedphase = 0.f;

    for(i=0; i < length; i++) {
        osc->speed = LPInterpolation.linear(speeds, speedphase);

        LPTapeOsc.process(osc);
        for(c=0; c < CHANNELS; c++) {
            out->data[i * CHANNELS + c] = osc->current_frame->data[c];
        }
        speedphase += speedphaseinc;
        if(speedphase >= speeds->length) {
            speedphase -= speeds->length;
        }
    }

    LPSoundFile.write("renders/tapeosc-out.wav", out);

    LPSineOsc.destroy(sineosc);
    LPTapeOsc.destroy(osc);
    LPBuffer.destroy(out);
    LPBuffer.destroy(sine);
    LPBuffer.destroy(sinefreq);
    LPBuffer.destroy(sineamp);
    LPBuffer.destroy(speeds);

    return 0;
}

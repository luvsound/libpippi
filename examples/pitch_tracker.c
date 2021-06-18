#include "pippi.h"

#define BS 4096
#define SR 48000
#define CHANNELS 1

int main() {
    lpfloat_t minfreq, maxfreq, sample;
    size_t length;
    lpbuffer_t * out;
    lpbuffer_t * reference;
    lpbuffer_t * amp;
    lpbuffer_t * freq;
    lpsineosc_t * osc;
    lpsineosc_t * osc2;
    lpyin_t * yin;
    int i, c;
    lpfloat_t p, last_p;

    minfreq = 80.0;
    maxfreq = 800.0;
    amp = LPParam.from_float(0.2);

    length = 3 * SR;

    /* Make an LFO table to use as a frequency curve for the osc */
    freq = LPWindow.create("sine", BS);

    /* Scale it from a range of -1 to 1 to a range of minfreq to maxfreq */
    LPBuffer.scale(freq, 0, 1, minfreq, maxfreq);

    osc = LPSineOsc.create();
    osc->samplerate = SR;

    reference = LPSineOsc.render(osc, length, freq, amp, CHANNELS);
    out = LPBuffer.create(length, CHANNELS, SR);

    osc2 = LPSineOsc.create();
    osc2->freq = 220.f;

    yin = LPPitchTracker.yin_create(4096, SR);
    yin->fallback = 220.f;

    last_p = -1;
    p = 0;
    for(i=0; i < length; i++) {
        p = LPPitchTracker.yin_process(yin, reference->data[i * CHANNELS]);
        if(p > 0 && p != last_p) {
            osc2->freq = p;
            last_p = p;
        }

        sample = LPSineOsc.process(osc2);

        for(c=0; c < CHANNELS; c++) {
            out->data[i * CHANNELS + c] = sample;
        }
    }

    LPSoundFile.write("renders/pitch_tracker-out.wav", out);

    LPSineOsc.destroy(osc);
    LPSineOsc.destroy(osc2);
    LPBuffer.destroy(reference);
    LPBuffer.destroy(out);
    LPBuffer.destroy(freq);
    LPBuffer.destroy(amp);
    LPPitchTracker.yin_destroy(yin);

    return 0;
}

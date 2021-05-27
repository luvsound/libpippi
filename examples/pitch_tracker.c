#include "pippi.h"

#define BS 4096
#define SR 48000
#define CHANNELS 1

int main() {
    lpfloat_t minfreq, maxfreq, sample;
    size_t length;
    buffer_t * out;
    buffer_t * reference;
    buffer_t * amp;
    buffer_t * freq;
    sineosc_t * osc;
    sineosc_t * osc2;
    yin_t * yin;
    int i, c;
    lpfloat_t p, last_p;

    minfreq = 80.0;
    maxfreq = 800.0;
    amp = Param.from_float(0.2);

    length = 3 * SR;

    /* Make an LFO table to use as a frequency curve for the osc */
    freq = Window.create("sine", BS);

    /* Scale it from a range of -1 to 1 to a range of minfreq to maxfreq */
    Buffer.scale(freq, 0, 1, minfreq, maxfreq);

    osc = SineOsc.create();
    osc->samplerate = SR;

    reference = SineOsc.render(osc, length, freq, amp, CHANNELS);
    out = Buffer.create(length, CHANNELS, SR);

    osc2 = SineOsc.create();
    osc2->freq = 220.f;

    yin = PitchTracker.yin_create(4096, SR);
    yin->fallback = 220.f;

    last_p = -1;
    p = 0;
    for(i=0; i < length; i++) {
        p = PitchTracker.yin_process(yin, reference->data[i * CHANNELS]);
        if(p > 0 && p != last_p) {
            osc2->freq = p;
            last_p = p;
        }

        sample = SineOsc.process(osc2);

        for(c=0; c < CHANNELS; c++) {
            out->data[i * CHANNELS + c] = sample;
        }
    }

    SoundFile.write("renders/pitch_tracker-out.wav", out);

    SineOsc.destroy(osc);
    SineOsc.destroy(osc2);
    Buffer.destroy(reference);
    Buffer.destroy(out);
    Buffer.destroy(freq);
    Buffer.destroy(amp);
    PitchTracker.yin_destroy(yin);

    return 0;
}

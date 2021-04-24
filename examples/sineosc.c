#include "pippi.h"

#define BS 4096
#define SR 48000
#define CHANNELS 2

int main() {
    lpfloat_t minfreq, maxfreq, amp, sample;
    size_t i, c, length;
    buffer_t* freq_lfo;
    buffer_t* out;
    sineosc_t* osc;

    minfreq = 80.0;
    maxfreq = 800.0;
    amp = 0.2;

    length = 10 * SR;

    /* Make an LFO table to use as a frequency curve for the osc */
    freq_lfo = Wavetable.create("sine", BS);

    /* Scale it from a range of -1 to 1 to a range of minfreq to maxfreq */
    Buffer.scale(freq_lfo, -1, 1, minfreq, maxfreq);

    out = Buffer.create(length, CHANNELS, SR);
    osc = SineOsc.create();
    osc->samplerate = SR;

    for(i=0; i < length; i++) {
        osc->freq = Interpolation.linear_pos(freq_lfo, (double)i/(double)length);
        sample = SineOsc.process(osc) * amp;
        for(c=0; c < CHANNELS; c++) {
            out->data[i * CHANNELS + c] = sample;
        }
    }

    SoundFile.write("renders/sineosc-out.wav", out);

    SineOsc.destroy(osc);
    Buffer.destroy(out);

    return 0;
}

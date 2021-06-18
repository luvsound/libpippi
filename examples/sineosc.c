#include "pippi.h"

#define BS 4096
#define SR 48000
#define CHANNELS 2

int main() {
    lpfloat_t minfreq, maxfreq, amp, sample;
    size_t i, c, length;
    lpbuffer_t * freq_lfo;
    lpbuffer_t * out;
    lpsineosc_t * osc;

    minfreq = 80.0;
    maxfreq = 800.0;
    amp = 0.2;

    length = 10 * SR;

    /* Make an LFO table to use as a frequency curve for the osc */
    freq_lfo = LPWindow.create("sine", BS);

    /* Scale it from a range of -1 to 1 to a range of minfreq to maxfreq */
    LPBuffer.scale(freq_lfo, 0, 1, minfreq, maxfreq);

    out = LPBuffer.create(length, CHANNELS, SR);
    osc = LPSineOsc.create();
    osc->samplerate = SR;

    for(i=0; i < length; i++) {
        osc->freq = LPInterpolation.linear_pos(freq_lfo, (double)i/length);
        sample = LPSineOsc.process(osc) * amp;
        for(c=0; c < CHANNELS; c++) {
            out->data[i * CHANNELS + c] = sample;
        }
    }

    LPSoundFile.write("renders/sineosc-out.wav", out);

    LPSineOsc.destroy(osc);
    LPBuffer.destroy(out);
    LPBuffer.destroy(freq_lfo);

    return 0;
}

/**
 * ring buffer
 * */
#include "pippi.h"

/* Define some local constants */
#define SR 48000 /* Sampling rate */
#define CHANNELS 2 /* Number of output channels */


int main() {
    size_t length;
    buffer_t* amp;
    buffer_t* freq;
    buffer_t* out;
    sineosc_t* osc;

    freq = Param.createf(200.0f);
    amp = Param.createf(0.2f);

    length = 10 * SR;

    osc = SineOsc.create();
    osc->samplerate = SR;

    out = SineOsc.render(osc, length, freq, amp, CHANNELS);

    SoundFile.write("renders/memorypool-out.wav", out);

    SineOsc.destroy(osc);
    Buffer.destroy(out);
    Buffer.destroy(freq);
    Buffer.destroy(amp);

    return 0;
}

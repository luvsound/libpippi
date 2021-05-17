#include "pippi.h"

#define BS 4096
#define SR 48000
#define CHANNELS 1

int main() {
    lpfloat_t minfreq, maxfreq, sample;
    size_t i, c, length;
    buffer_t * out;
    buffer_t * pitches;
    buffer_t * amp;
    buffer_t * freq;
    sineosc_t * osc;

    minfreq = 80.0;
    maxfreq = 800.0;
    amp = Param.from_float(0.2);

    length = 10 * SR;

    /* Make an LFO table to use as a frequency curve for the osc */
    freq = Window.create("sine", BS);

    /* Scale it from a range of -1 to 1 to a range of minfreq to maxfreq */
    Buffer.scale(freq, 0, 1, minfreq, maxfreq);

    osc = SineOsc.create();
    osc->samplerate = SR;

    out = SineOsc.render(osc, length, freq, amp, CHANNELS);

    pitches = PitchTracker.process(out, 0.85f);
    Buffer.scale(pitches, 0, 1000, 0, 1);

    SoundFile.write("renders/pitch_tracker-out.wav", pitches);

    SineOsc.destroy(osc);
    Buffer.destroy(out);
    Buffer.destroy(freq);
    Buffer.destroy(amp);
    Buffer.destroy(pitches);

    return 0;
}

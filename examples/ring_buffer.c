/**
 * ring buffer
 * */
#include "pippi.h"

/* Define some local constants */
#define SR 48000 /* Sampling rate */
#define CHANNELS 2 /* Number of output channels */


int main() {
    size_t length;
    lpbuffer_t * amp;
    lpbuffer_t * env;
    lpbuffer_t * freq;
    lpbuffer_t * out;
    lpsineosc_t * osc;

    lpbuffer_t * ringbuf;
    lpbuffer_t * halfsec;

    freq = LPParam.from_float(200.0f);
    amp = LPParam.from_float(0.3f);
    env = LPWindow.create("sine", 4096);
    ringbuf = LPRingBuffer.create(SR, CHANNELS, SR); /* 1 second ring buffer */

    length = 4410;

    osc = LPSineOsc.create();
    osc->samplerate = SR;

    out = LPSineOsc.render(osc, length, freq, amp, CHANNELS);
    LPBuffer.env(out, env);

    LPRingBuffer.write(ringbuf, out);

    LPSoundFile.write("renders/ring_buffer-write-out.wav", ringbuf);

    halfsec = LPRingBuffer.read(ringbuf, SR/2);

    LPSoundFile.write("renders/ring_buffer-read-out.wav", halfsec);

    LPSineOsc.destroy(osc);
    LPBuffer.destroy(out);
    LPBuffer.destroy(freq);
    LPBuffer.destroy(amp);
    LPBuffer.destroy(halfsec);

    return 0;
}

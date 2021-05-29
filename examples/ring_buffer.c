/**
 * ring buffer
 * */
#include "pippi.h"

/* Define some local constants */
#define SR 48000 /* Sampling rate */
#define CHANNELS 2 /* Number of output channels */


int main() {
    size_t length;
    buffer_t * amp;
    buffer_t * env;
    buffer_t * freq;
    buffer_t * out;
    sineosc_t * osc;

    ringbuffer_t * ringbuf;
    buffer_t * halfsec;

    freq = Param.from_float(200.0f);
    amp = Param.from_float(0.3f);
    env = Window.create("sine", 4096);
    ringbuf = RingBuffer.create(SR, CHANNELS, SR); /* 1 second ring buffer */

    length = 4410;

    osc = SineOsc.create();
    osc->samplerate = SR;

    out = SineOsc.render(osc, length, freq, amp, CHANNELS);
    Buffer.env(out, env);

    RingBuffer.write(ringbuf, out);

    SoundFile.write("renders/ring_buffer-write-out.wav", ringbuf->buf);

    halfsec = RingBuffer.read(ringbuf, SR/2);

    SoundFile.write("renders/ring_buffer-read-out.wav", halfsec);

    SineOsc.destroy(osc);
    Buffer.destroy(out);
    Buffer.destroy(freq);
    Buffer.destroy(amp);
    Buffer.destroy(halfsec);

    return 0;
}

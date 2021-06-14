/**
 * onset detector
 * */
#include "pippi.h"

/* Define some local constants */
#define SR 48000 /* Sampling rate */
#define CHANNELS 1 /* Number of output channels */


int main() {
    size_t length;
    buffer_t * amp;
    buffer_t * env;
    buffer_t * freq;
    buffer_t * wavelet;
    buffer_t * silence;

    sineosc_t * osc;
    buffer_t * ringbuf;
    int i, event, numticks;
    coyote_t * od;
    int gate;


    /* 1 second ring buffer */
    ringbuf = LPRingBuffer.create(SR, CHANNELS, SR); 

    /* 60 frames of silence */
    silence = Buffer.create(600, CHANNELS, SR);

    /* Setup the SineOsc params */
    freq = Param.from_float(2000.0f);
    amp = Param.from_float(0.2f);
    env = Window.create("sine", 128);

    osc = SineOsc.create();
    osc->samplerate = SR;

    /* 1ms sine wavelet */
    length = 48;
    wavelet = SineOsc.render(osc, length, freq, amp, CHANNELS);
    Buffer.env(wavelet, env);

    /* Write some wavelet blips into the ring buffer */
    numticks = 80;
    for(i=0; i < numticks; i++) {
        event = (i % 11 == 0) ? 1 : 0;
        if(event) {
            LPRingBuffer.dub(ringbuf, wavelet);
        } else {
            LPRingBuffer.dub(ringbuf, silence);
        }
    }

    od = OnsetDetector.coyote_create(ringbuf->samplerate);
    gate = 1;
    for(i=0; i < SR; i++) {
        OnsetDetector.coyote_process(od, ringbuf->data[i]);
        if(gate != od->gate) {
            if(gate == 1 && od->gate == 0) {
                printf("Onset! at %f seconds. gate: %d od->gate: %d\n", (double)i/SR, gate, od->gate);
            }
            gate = od->gate;
        }
    }

    SoundFile.write("renders/onset_detector-out.wav", ringbuf);

    SineOsc.destroy(osc);
    Buffer.destroy(wavelet);
    Buffer.destroy(silence);
    Buffer.destroy(freq);
    Buffer.destroy(amp);
    OnsetDetector.coyote_destory(od);

    return 0;
}

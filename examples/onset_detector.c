/**
 * onset detector
 * */
#include "pippi.h"

/* Define some local constants */
#define SR 48000 /* Sampling rate */
#define CHANNELS 1 /* Number of output channels */


int main() {
    size_t length;
    lpbuffer_t * amp;
    lpbuffer_t * env;
    lpbuffer_t * freq;
    lpbuffer_t * wavelet;
    lpbuffer_t * silence;

    lpsineosc_t * osc;
    lpbuffer_t * ringbuf;
    int i, event, numticks;
    lpcoyote_t * od;
    int gate;


    /* 1 second ring buffer */
    ringbuf = LPRingBuffer.create(SR, CHANNELS, SR); 

    /* 60 frames of silence */
    silence = LPBuffer.create(600, CHANNELS, SR);

    /* Setup the SineOsc params */
    freq = LPParam.from_float(2000.0f);
    amp = LPParam.from_float(0.2f);
    env = LPWindow.create("sine", 128);

    osc = LPSineOsc.create();
    osc->samplerate = SR;

    /* 1ms sine wavelet */
    length = 48;
    wavelet = LPSineOsc.render(osc, length, freq, amp, CHANNELS);
    LPBuffer.env(wavelet, env);

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

    od = LPOnsetDetector.coyote_create(ringbuf->samplerate);
    gate = 1;
    for(i=0; i < SR; i++) {
        LPOnsetDetector.coyote_process(od, ringbuf->data[i]);
        if(gate != od->gate) {
            if(gate == 1 && od->gate == 0) {
                printf("Onset! at %f seconds. gate: %d od->gate: %d\n", (double)i/SR, gate, od->gate);
            }
            gate = od->gate;
        }
    }

    LPSoundFile.write("renders/onset_detector-out.wav", ringbuf);

    LPSineOsc.destroy(osc);
    LPBuffer.destroy(wavelet);
    LPBuffer.destroy(silence);
    LPBuffer.destroy(freq);
    LPBuffer.destroy(amp);
    LPOnsetDetector.coyote_destory(od);

    return 0;
}

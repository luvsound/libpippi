/**
 * scheduler
 * */
#include "pippi.h"

/* Define some local constants */
#define SR 48000 /* Sampling rate */
#define CHANNELS 2 /* Number of output channels */


int main() {
    lpbuffer_t * amp;
    lpbuffer_t * env;
    lpbuffer_t * freq;
    lpbuffer_t * wavelet1;
    lpbuffer_t * wavelet2;
    lpbuffer_t * wavelet3;
    lpbuffer_t * wavelet4;
    lpbuffer_t * out;

    lpsineosc_t * osc;
    lpscheduler_t * s;

    int i, c;
    size_t output_length;

    /* Setup the scheduler */
    s = LPScheduler.create(CHANNELS);

    /* Create an output buffer */
    output_length = 1000;
    out = LPBuffer.create(output_length, CHANNELS, SR);

    /* Setup the LPSineOsc params */
    freq = LPParam.from_float(2000.0f);
    amp = LPParam.from_float(0.2f);
    env = LPWindow.create("sine", 128);

    osc = LPSineOsc.create();
    osc->samplerate = SR;

    /* 1ms sine wavelet */
    wavelet1 = LPSineOsc.render(osc, 100, freq, amp, CHANNELS);
    LPBuffer.env(wavelet1, env);
    wavelet2 = LPSineOsc.render(osc, 100, freq, amp, CHANNELS);
    LPBuffer.env(wavelet2, env);
    wavelet3 = LPSineOsc.render(osc, 100, freq, amp, CHANNELS);
    LPBuffer.env(wavelet3, env);
    wavelet4 = LPSineOsc.render(osc, 100, freq, amp, CHANNELS);
    LPBuffer.env(wavelet4, env);

    /* Schedule some events */
    LPScheduler.schedule_event(s, wavelet1, 0);
    LPScheduler.schedule_event(s, wavelet2, 200);
    LPScheduler.schedule_event(s, wavelet3, 500);
    LPScheduler.schedule_event(s, wavelet4, 800);

    /* Render the events to a buffer. */
    for(i=0; i < output_length; i++) {
        /* Call tick before each read from s->current_frame */
        /* The scheduler will fill it with a mix of samples from 
         * every currently playing buffer at the current time index.
         */
        LPScheduler.tick(s);

        /* Copy the samples from the current frame into the output buffer. */
        for(c=0; c < CHANNELS; c++) {
            out->data[i * CHANNELS + c] = s->current_frame[c];
        }
    }

    LPSoundFile.write("renders/scheduler-out.wav", out);

    LPSineOsc.destroy(osc);
    LPBuffer.destroy(wavelet1);
    LPBuffer.destroy(wavelet2);
    LPBuffer.destroy(wavelet3);
    LPBuffer.destroy(wavelet4);
    LPBuffer.destroy(freq);
    LPBuffer.destroy(amp);
    LPBuffer.destroy(out);
    LPBuffer.destroy(env);
    LPScheduler.destroy(s);

    return 0;
}

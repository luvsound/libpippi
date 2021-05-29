/**
 * scheduler
 * */
#include "pippi.h"

/* Define some local constants */
#define SR 48000 /* Sampling rate */
#define CHANNELS 2 /* Number of output channels */


int main() {
    buffer_t * amp;
    buffer_t * env;
    buffer_t * freq;
    buffer_t * wavelet1;
    buffer_t * wavelet2;
    buffer_t * wavelet3;
    buffer_t * wavelet4;
    buffer_t * out;

    sineosc_t * osc;
    scheduler_t * s;

    int i, c;
    size_t output_length;

    /* Setup the scheduler */
    s = Scheduler.create(CHANNELS);

    /* Create an output buffer */
    output_length = 1000;
    out = Buffer.create(output_length, CHANNELS, SR);

    /* Setup the SineOsc params */
    freq = Param.from_float(2000.0f);
    amp = Param.from_float(0.2f);
    env = Window.create("sine", 128);

    osc = SineOsc.create();
    osc->samplerate = SR;

    /* 1ms sine wavelet */
    wavelet1 = SineOsc.render(osc, 100, freq, amp, CHANNELS);
    Buffer.env(wavelet1, env);
    wavelet2 = SineOsc.render(osc, 100, freq, amp, CHANNELS);
    Buffer.env(wavelet2, env);
    wavelet3 = SineOsc.render(osc, 100, freq, amp, CHANNELS);
    Buffer.env(wavelet3, env);
    wavelet4 = SineOsc.render(osc, 100, freq, amp, CHANNELS);
    Buffer.env(wavelet4, env);

    /* Schedule some events */
    Scheduler.schedule_event(s, wavelet1, 0);
    Scheduler.schedule_event(s, wavelet2, 200);
    Scheduler.schedule_event(s, wavelet3, 500);
    Scheduler.schedule_event(s, wavelet4, 800);

    /* Render the events to a buffer. */
    for(i=0; i < output_length; i++) {
        /* Call tick before each read from s->current_frame */
        /* The scheduler will fill it with a mix of samples from 
         * every currently playing buffer at the current time index.
         */
        Scheduler.tick(s);

        /* Copy the samples from the current frame into the output buffer. */
        for(c=0; c < CHANNELS; c++) {
            out->data[i * CHANNELS + c] = s->current_frame[c];
        }
    }

    SoundFile.write("renders/scheduler-out.wav", out);

    SineOsc.destroy(osc);
    Buffer.destroy(wavelet1);
    Buffer.destroy(wavelet2);
    Buffer.destroy(wavelet3);
    Buffer.destroy(wavelet4);
    Buffer.destroy(freq);
    Buffer.destroy(amp);
    Buffer.destroy(out);
    Scheduler.destroy(s);

    return 0;
}

#include "pippi.h"

#define SR 48000
#define CHANNELS 2

int main() {
    size_t i, c, length, numgrains, maxgrainlength, mingrainlength;
    lpbuffer_t * fake_input;
    lpbuffer_t * out;
    lpbuffer_t * freq;
    lpbuffer_t * amp;
    lpcloud_t * cloud;
    lpsineosc_t * osc;

    length = 10 * SR;
    numgrains = 1;
    maxgrainlength = SR;
    mingrainlength = SR/10.;

    out = LPBuffer.create(length, CHANNELS, SR);
    cloud = LPCloud.create(numgrains, maxgrainlength, mingrainlength, length, CHANNELS, SR);

    /* Render a sine tone and fill the ringbuffer with it, 
     * to simulate a live input. */
    osc = LPSineOsc.create();
    osc->samplerate = SR;

    freq = LPParam.from_float(220.0f);
    amp = LPParam.from_float(0.8f);

    fake_input = LPSineOsc.render(osc, length, freq, amp, CHANNELS);
    LPRingBuffer.write(cloud->rb, fake_input);

    LPSoundFile.write("renders/graincloud-rb-out.wav", cloud->rb);

    /* Render each frame of the graincloud */
    for(i=0; i < length; i++) {
        LPCloud.process(cloud);
        for(c=0; c < CHANNELS; c++) {
            out->data[i * CHANNELS + c] = cloud->current_frame->data[c];
        }
    }

    LPSoundFile.write("renders/graincloud-out.wav", out);

    LPSineOsc.destroy(osc);
    LPBuffer.destroy(freq);
    LPBuffer.destroy(amp);
    LPBuffer.destroy(out);
    LPBuffer.destroy(fake_input);
    LPCloud.destroy(cloud);

    return 0;
}

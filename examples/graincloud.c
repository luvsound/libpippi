#include "pippi.h"

#define SR 48000
#define CHANNELS 2

int main() {
    size_t i, c, length, numgrains, maxgrainlength, mingrainlength;
    buffer_t * fake_input;
    buffer_t * out;
    buffer_t * freq;
    buffer_t * amp;
    lpcloud_t * cloud;
    sineosc_t * osc;

    length = 10 * SR;
    numgrains = 20;
    maxgrainlength = SR;
    mingrainlength = SR/10.;

    out = Buffer.create(length, CHANNELS, SR);
    cloud = Cloud.create(numgrains, maxgrainlength, mingrainlength, length, CHANNELS, SR);

    /* Render a sine tone and fill the ringbuffer with it, 
     * to simulate a live input. */
    osc = SineOsc.create();
    osc->samplerate = SR;

    freq = Param.from_float(220.0f);
    amp = Param.from_float(0.8f);

    fake_input = SineOsc.render(osc, length, freq, amp, CHANNELS);
    LPRingBuffer.write(cloud->rb, fake_input);

    /* Render each frame of the graincloud */
    for(i=0; i < length; i++) {
        Cloud.process(cloud);
        for(c=0; c < CHANNELS; c++) {
            out->data[i * CHANNELS + c] = cloud->current_frame->data[c];
        }
    }

    SoundFile.write("renders/graincloud-out.wav", out);

    SineOsc.destroy(osc);
    Buffer.destroy(freq);
    Buffer.destroy(amp);
    Buffer.destroy(out);
    Buffer.destroy(fake_input);
    Cloud.destroy(cloud);

    return 0;
}

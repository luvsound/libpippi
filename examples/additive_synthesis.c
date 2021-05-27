#include "pippi.h"

#define BS 4096
#define SR 48000
#define CHANNELS 2
#define PARTIALS 100
#define SEED 3.999

lpfloat_t x;

lpfloat_t logistic(lpfloat_t low, lpfloat_t high) {
    x = SEED * x * (1.0 - x);
    return x * (high - low) + low;
}


int main() {
    lpfloat_t freqdrift, minfreq, maxfreq, basefreq;
    lpfloat_t a, ampdrift, sample, pos;
    size_t i, c, p, length;
    buffer_t * out;

    buffer_t * freq[PARTIALS];
    buffer_t * amp[PARTIALS];
    sineosc_t * osc[PARTIALS];

    /* Rand is used internally for window selection.
     * Every call to Window.create("rnd", BS) invokes 
     * the random number generator when choosing a random 
     * window type from libpippi's internal windows.
     **/
    Rand.seed(888); 

    x = 0.555f;
    freqdrift = 30.f;
    ampdrift = 0.01f;

    length = 10 * SR;
    basefreq = 60.f;

    /* Make an LFO table to use as a frequency curve for the osc */
    for(i=0; i < PARTIALS; i++) {
        freq[i] = Window.create("rnd", BS);
        minfreq = (basefreq * (i+1)) + logistic(-freqdrift, 0.f);
        maxfreq = (basefreq * (i+1)) + logistic(0.f, freqdrift);
        Buffer.scale(freq[i], 0, 1, minfreq, maxfreq);

        amp[i] = Window.create("rnd", BS);
        Buffer.scale(amp[i], 0, 1, 0.f, Rand.rand(ampdrift * 0.1, ampdrift));

        osc[i] = SineOsc.create();
        osc[i]->phase = Rand.rand(0.f, 1.f); /* scramble phase */
        osc[i]->samplerate = SR;
        osc[i]->freq = freq[i]->data[0];
    }

    out = Buffer.create(length, CHANNELS, SR);
    for(i=0; i < length; i++) {
        pos = (lpfloat_t)i/length;
        for(p=0; p < PARTIALS; p++) {
            osc[p]->freq = Interpolation.linear_pos(freq[p], pos);
            a = Interpolation.linear_pos(amp[p], pos);
            sample = SineOsc.process(osc[p]) * a;
            for(c=0; c < CHANNELS; c++) {
                out->data[i * CHANNELS + c] += sample;
            }
        }
    }

    SoundFile.write("renders/additive-synthesis-out.wav", out);

    for(p=0; p < PARTIALS; p++) {
        SineOsc.destroy(osc[p]);
        Buffer.destroy(freq[p]);
        Buffer.destroy(amp[p]);
    }

    Buffer.destroy(out);

    return 0;
}

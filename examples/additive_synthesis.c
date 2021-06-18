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
    lpbuffer_t * out;

    lpbuffer_t * freq[PARTIALS];
    lpbuffer_t * amp[PARTIALS];
    lpsineosc_t * osc[PARTIALS];

    /* LPRand is used internally for window selection.
     * Every call to LPWindow.create("rnd", BS) invokes 
     * the random number generator when choosing a random 
     * window type from libpippi's internal windows.
     **/
    LPRand.seed(888); 

    x = 0.555f;
    freqdrift = 30.f;
    ampdrift = 0.01f;

    length = 10 * SR;
    basefreq = 60.f;

    /* Make an LFO table to use as a frequency curve for the osc */
    for(i=0; i < PARTIALS; i++) {
        freq[i] = LPWindow.create("rnd", BS);
        minfreq = (basefreq * (i+1)) + logistic(-freqdrift, 0.f);
        maxfreq = (basefreq * (i+1)) + logistic(0.f, freqdrift);
        LPBuffer.scale(freq[i], 0, 1, minfreq, maxfreq);

        amp[i] = LPWindow.create("rnd", BS);
        LPBuffer.scale(amp[i], 0, 1, 0.f, LPRand.rand(ampdrift * 0.1, ampdrift));

        osc[i] = LPSineOsc.create();
        osc[i]->phase = LPRand.rand(0.f, 1.f); /* scramble phase */
        osc[i]->samplerate = SR;
        osc[i]->freq = freq[i]->data[0];
    }

    out = LPBuffer.create(length, CHANNELS, SR);
    for(i=0; i < length; i++) {
        pos = (lpfloat_t)i/length;
        for(p=0; p < PARTIALS; p++) {
            osc[p]->freq = LPInterpolation.linear_pos(freq[p], pos);
            a = LPInterpolation.linear_pos(amp[p], pos);
            sample = LPSineOsc.process(osc[p]) * a;
            for(c=0; c < CHANNELS; c++) {
                out->data[i * CHANNELS + c] += sample;
            }
        }
    }

    LPSoundFile.write("renders/additive-synthesis-out.wav", out);

    for(p=0; p < PARTIALS; p++) {
        LPSineOsc.destroy(osc[p]);
        LPBuffer.destroy(freq[p]);
        LPBuffer.destroy(amp[p]);
    }

    LPBuffer.destroy(out);

    return 0;
}

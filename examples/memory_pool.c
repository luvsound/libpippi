/**
 * This example is the same as the sineosc.c example, 
 * but it demonstrates how to tell pippi to use a static 
 * memory pool under the hood instead of malloc. 
 *
 * This is especially useful for embedded environments with 
 * strict memory restrictions.
 *
 * To enable the memory pool allocator, you must define 
 * LP_STATIC before including pippi.h.
 */

/* Enable memory pool allocator */
#define LP_STATIC

/* Use single precisions floats instead of doubles 
 * so this runs better on platforms without an FPU */
#define LP_FLOAT

/* Include the core library */
#include "pippicore.h"

/* Since we are using the pippicore.h header, 
 * lets include the module(s) we wish to use.
 *
 * If we just used the fat "pippi.h" header, 
 * every module is already included.
 *
 * This example demonstrates how to use libpippi 
 * in an embedded environment so we don't want to 
 * include all the parts of the library we don't need.
 *
 * Normally in an embedded environment we probably 
 * wouldn't be including the soundfile.h module for 
 * example, but this test program writes its output 
 * to disk.
 *
 * To adapt this example, just remove soundfile.h 
 * and write the samples to your audio output codec 
 * instead of `out->data[i * CHANNELS + c]`.
 **/
#include "sineosc.h"
#include "window.h"
#include "interpolation.h"
#include "soundfile.h"

/* Define some local constants */
#define BS 1024 /* Set the block size to 1024 */
#define SR 48000 /* Sampling rate */
#define CHANNELS 2 /* Number of output channels */
#define POOLSIZE 67108864 /* Set the memory pool size to 64MB */

/* Statically allocate the memory pool */
unsigned char pool[POOLSIZE];


int main() {
    lpfloat_t minfreq, maxfreq, amp, sample;
    size_t i, c, length;
    buffer_t* freq_lfo;
    buffer_t* out;
    sineosc_t* osc;

    /* This is the final required step.
     * Pool.init() tells pippi about the
     * size & location of the memory pool. 
     *
     * Because the LP_STATIC flag is set, 
     * all internal allocations will use the
     * pool instead of dynamic allocation.
     **/
    MemoryPool.init((unsigned char *)pool, POOLSIZE);

    minfreq = 80.0;
    maxfreq = 800.0;
    amp = 0.2;

    length = 10 * SR;

    /* Make an LFO table to use as a frequency curve for the osc */
    freq_lfo = Window.create("sine", BS);

    /* Scale it from a range of -1 to 1 to a range of minfreq to maxfreq */
    Buffer.scale(freq_lfo, 0, 1, minfreq, maxfreq);

    out = Buffer.create(length, CHANNELS, SR);
    osc = SineOsc.create();
    osc->samplerate = SR;

    for(i=0; i < length; i++) {
        osc->freq = Interpolation.linear_pos(freq_lfo, (double)i/(double)length);
        sample = SineOsc.process(osc) * amp;
        for(c=0; c < CHANNELS; c++) {
            out->data[i * CHANNELS + c] = sample;
        }
    }

    SoundFile.write("renders/memorypool-out.wav", out);

    SineOsc.destroy(osc);
    Buffer.destroy(out);
    Buffer.destroy(freq_lfo);

    return 0;
}

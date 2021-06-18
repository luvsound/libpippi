/**
 * This example is the same as the sineosc.c example, 
 * but it demonstrates how to tell pippi to use a static 
 * memory pool under the hood instead of malloc. 
 *
 * This is especially useful for embedded environments with 
 * strict memory restrictions.
 *
 * To enable the memory pool allocator, you must define LP_STATIC.
 * To ensure the flag is visible to all modules, do this as an argument 
 * to the compiler:
 *
 *     gcc ... -DLP_STATIC ...
 *
 * Many microcontrollers are unable to perform well with double precision 
 * floats. To tell libpippi to use single precisions floats instead of doubles 
 * because the environment you are targeting does not provide an FPU, 
 * define the flag LP_FLOAT along with the LP_STATIC flag:
 *
 *     gcc ... -DLP_STATIC -DLP_FLOAT ...
 *
 * In desktop environments or something more powerful like a raspberry pi 
 * we can just use the fat "pippi.h" header which includes every module. 
 *
 * To save some program space in constrained environments, We can use the 
 * "pippicore.h" header instead, and include only the modules we actually 
 * want to use.
 *
 * Actually "pippicore.h" may be omitted since the modules will include it 
 * if missing, but it's nice to be explicit.
 * */
#include "pippicore.h"

/* Since we are using the pippicore.h header, 
 * lets include the module(s) we wish to use.
 **/
#include "oscs.sine.h"

/* Normally in an embedded environment we probably 
 * wouldn't be including the soundfile.h module for 
 * example, but this test program writes its output 
 * to disk. */
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
    lpbuffer_t * freq_lfo;
    lpbuffer_t * out;
    lpsineosc_t * osc;

    /* This is the final required step.
     * Pool.init() tells pippi about the
     * size & location of the memory pool. 
     *
     * Because the LP_STATIC flag is set, 
     * all internal allocations will use the
     * pool instead of dynamic allocation.
     **/
    LPMemoryPool.init((unsigned char *)pool, POOLSIZE);

    minfreq = 80.0f;
    maxfreq = 800.0f;
    amp = 0.2f;

    length = 10 * SR;

    /* Make an LFO table to use as a frequency curve for the osc */
    freq_lfo = LPWindow.create("sine", BS);

    /* Scale it from a range of -1 to 1 to a range of minfreq to maxfreq */
    LPBuffer.scale(freq_lfo, 0.f, 1.f, minfreq, maxfreq);

    out = LPBuffer.create(length, CHANNELS, SR);
    osc = LPSineOsc.create();
    osc->samplerate = SR;

    /* If we were actually running this in an embedded environment, 
     * here we'd render just enough frames to fill the output buffer 
     * for the audio codec. In this example we create a buffer big enough 
     * to hold the output and then write it all to disk. */
    for(i=0; i < length; i++) {
        osc->freq = LPInterpolation.linear_pos(freq_lfo, (float)i/length);
        sample = LPSineOsc.process(osc) * amp;
        for(c=0; c < CHANNELS; c++) {
            out->data[i * CHANNELS + c] = sample;
        }
    }

    LPSoundFile.write("renders/memorypool-out.wav", out);

    /* These are a no-op if the static allocator is enabled. */
    LPSineOsc.destroy(osc);
    LPBuffer.destroy(out);
    LPBuffer.destroy(freq_lfo);

    return 0;
}

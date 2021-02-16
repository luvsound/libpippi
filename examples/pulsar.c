#include <stdlib.h>
#include "pippi.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_libs/dr_wav.h"

#define VOICES 4
#define CHANNELS 2
#define BUFSIZE 1024
#define SR 44100

int main() {
    int length = SR * 60;
    int i, c, v;

    lpfloat_t sample = 0;
    int count;
    float *buf;
    drwav wav;
    drwav_data_format format;

    pulsar_t* oscs[VOICES];

    lpfloat_t freqs[VOICES] = {220., 330., 440., 550.};
    lpfloat_t morphs[VOICES] = {0.1, 0.2, 0.3, 0.4};
    lpfloat_t mods[VOICES] = {0.01, 0.02, 0.03, 0.04};

    buf = malloc(BUFSIZE * CHANNELS * sizeof(float));

    format.container = drwav_container_riff;
    format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
    format.channels = CHANNELS;
    format.sampleRate = SR;
    format.bitsPerSample = 32;

    drwav_init_file_write(&wav, "renders/pulsar-out.wav", &format, NULL);

    count = 0;

    for(i=0; i < VOICES; i++) {
        oscs[i] = Pulsar.create();
        oscs[i]->freq = freqs[i];
        oscs[i]->morphfreq = morphs[i];
        oscs[i]->modfreq = mods[i];
    }

    for(i=0; i < length; i++) {
        sample = 0;
        for(v=0; v < VOICES; v++) {
            sample += Pulsar.process(oscs[v]) * 0.2;
        }

        for(c=0; c < CHANNELS; c++) {
            buf[count * CHANNELS + c] = sample;
        }

        count++;

        if (count >= BUFSIZE) {
            drwav_write_pcm_frames(&wav, BUFSIZE, buf);
            count = 0;
        }
    }

    if (count != 0) {
        drwav_write_pcm_frames(&wav, count, buf);
    }

    for(v=0; v < VOICES; v++) {
        Pulsar.destroy(oscs[v]);
    }

    free(buf);
    drwav_uninit(&wav);
    return 0;
}



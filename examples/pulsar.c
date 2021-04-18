#include "pippi.h"

#define VOICES 4
#define CHANNELS 2
#define SR 44100

int main() {
    int length = SR * 60;
    int i, c, v;

    lpfloat_t sample = 0;

    pulsar_t* oscs[VOICES];
    lpfloat_t freqs[VOICES] = {220., 330., 440., 550.};
    lpfloat_t morphs[VOICES] = {0.1, 0.2, 0.3, 0.4};
    lpfloat_t mods[VOICES] = {0.01, 0.02, 0.03, 0.04};

    buffer_t* buf = Pippi.buffer(length, CHANNELS, SR);    

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
            buf->data[i * CHANNELS + c] = sample;
        }
    }

    write_soundfile("renders/pulsar-out.wav", buf);

    for(v=0; v < VOICES; v++) {
        Pulsar.destroy(oscs[v]);
    }

    Pippi.destroy_buffer(buf);

    return 0;
}



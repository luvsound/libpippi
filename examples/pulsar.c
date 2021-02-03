#include <pippi.h>

#define VOICES 4

int main() {
    int channels = 2;
    int length = 44100 * 60;

    pulsar_t* oscs[VOICES];
    lpfloat_t freqs[VOICES] = {220., 330., 440., 550.};
    lpfloat_t morphs[VOICES] = {0.1, 0.2, 0.3, 0.4};

    for(int i=0; i < VOICES; i++) {
        Pulsar.freq = freqs[i];
        Pulsar.morphfreq = morphs[i];
        oscs[i] = Pulsar.create(&Pulsar);
    }

    FILE *out;
    out = fopen("renders/pulsar-out.raw", "wb");

    lpfloat_t sample = 0;
    for(int i=0; i < length; i++) {
        for(int v=0; v < VOICES; v++) {
            sample = Pulsar.process(oscs[v]) * 0.2;
            for(int c=0; c < channels; c++) {
                fwrite(&sample, sizeof(lpfloat_t), 1, out);
            }
        }
    }

    for(int v=0; v < VOICES; v++) {
        Pulsar.destroy(oscs[v]);
    }

    fclose(out);
    return 0;
}



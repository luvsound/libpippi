#include "pulsar.h"

int main() {
    int channels = 2;
    int samplerate = 44100;
    int tablesize = 4096;
    int length = 44100 * 60;

    lpfloat_t modfreq = 0.03;
    lpfloat_t morphfreq = 0.3;
    lpfloat_t freq = 220.0;

    char wts[] = "sine,square,tri,sine";
    char wins[] = "sine,hann,sine";
    char burst[] = "1,1,0,1";

    Pulsar* p = init_pulsar(tablesize, freq, modfreq, morphfreq, wts, wins, burst, samplerate);

    FILE *out;
    out = fopen("out.raw", "wb");

    lpfloat_t sample = 0;
    for(int i=0; i < length; i++) {
        sample = process_pulsar_sample(p);
        for(int c=0; c < channels; c++) {
            fwrite(&sample, sizeof(lpfloat_t), 1, out);
        }
    }

    cleanup_pulsar(p);
    fclose(out);
    return 0;
}



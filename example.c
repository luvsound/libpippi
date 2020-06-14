#include "pulsar.h"

int main() {
    int channels = 2;
    int samplerate = 44100;
    int tablesize = 4096;
    int length = 44100 * 60;

    double modfreq = 0.1;
    double morphfreq = 0.3;
    double freq = 220.0;

    char wts[] = "sine,square,tri,sine";
    char wins[] = "sine";
    char burst[] = "1,1,0,1";

    Pulsar* p = init(tablesize, freq, modfreq, morphfreq, wts, wins, burst, samplerate);

    FILE *out;
    out = fopen("out.raw", "wb");

    double sample = 0;
    for(int i=0; i < length; i++) {
        sample = process(p);
        for(int c=0; c < channels; c++) {
            fwrite(&sample, sizeof(double), 1, out);
        }
    }

    cleanup(p);
    fclose(out);
    return 0;
}



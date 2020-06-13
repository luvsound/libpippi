#include "pulsar.h"

int main() {
    int channels = 2;
    int samplerate = 44100;
    int tablesize = 4096;
    int length = 44100 * 60;

    double modfreq = 0.02;
    double morphfreq = 0.1;
    double freq = 220.0;

    generator wts[4] = {
        make_sine,
        make_square, 
        make_tri, 
        make_sine
    };
    int numwts = sizeof(wts) / sizeof(wts[0]);

    generator wins[2] = {
        make_sine_win,
        make_tri_win
    };
    int numwins = sizeof(wts) / sizeof(wts[0]);

    FILE *out;

    Pulsar* p = init(tablesize, numwts, numwins, freq, modfreq, morphfreq, wts, wins, samplerate);
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



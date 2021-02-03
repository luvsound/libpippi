#include <pippi.h>

int main() {
    int channels = 2;
    int length = 44100 * 60;

    pulsar_t* p = Pulsar.create();

    FILE *out;
    out = fopen("renders/pulsar-out.raw", "wb");

    lpfloat_t sample = 0;
    for(int i=0; i < length; i++) {
        sample = Pulsar.process(p);
        for(int c=0; c < channels; c++) {
            fwrite(&sample, sizeof(lpfloat_t), 1, out);
        }
    }

    Pulsar.destroy(p);
    fclose(out);
    return 0;
}



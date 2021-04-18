#ifndef LP_SOUNDFILES_H
#define LP_SOUNDFILES_H

#include "pippi.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_libs/dr_wav.h"

#define BUFSIZE 1024

void write_soundfile(char* path, buffer_t* buf) {
    int count;
    float* tmpbuf;
    drwav wav;
    drwav_data_format format;
    int i, c;

    int channels = buf->channels;
    tmpbuf = (float*)calloc(BUFSIZE * buf->channels, sizeof(float));

    format.container = drwav_container_riff;
    format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
    format.channels = buf->channels;
    format.sampleRate = buf->samplerate;
    format.bitsPerSample = 32;

    drwav_init_file_write(&wav, path, &format, NULL);

    count = 0;

    for(i=0; i < buf->length; i++) {
        for(c=0; c < channels; c++) {
            tmpbuf[count * channels + c] = buf->data[i * channels + c];
        }

        count++;

        if (count >= BUFSIZE) {
            drwav_write_pcm_frames(&wav, BUFSIZE, tmpbuf);
            count = 0;
        }
    }

    if (count != 0) {
        drwav_write_pcm_frames(&wav, count, tmpbuf);
    }

    drwav_uninit(&wav);
    free(tmpbuf);
}

#endif

#include "soundfile.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_libs/dr_wav.h"

#define LP_SOUNDFILE_BUFSIZE 1024

lpbuffer_t * read_soundfile(const char * path) {
    unsigned int channels;
    unsigned int samplerate;
    lpbuffer_t * out;
    drwav_uint64 frames;
    float * data;
    int i, c;

    data = (float *)drwav_open_file_and_read_pcm_frames_f32(path, &channels, &samplerate, &frames, NULL);
    if (data == NULL) {
        printf("Error: could not open file: %s", path);
        exit(EXIT_FAILURE);
    }

    out = LPBuffer.create(frames, channels, samplerate);
    for(i=0; i < frames; i++) {
        for(c=0; c < channels; c++) {
            out->data[i * channels + c] = data[i * channels + c];
        }
    }
    drwav_free(data, NULL);
    return out;
}

void write_soundfile(const char * path, lpbuffer_t * buf) {
    int count;
    float * tmpbuf;
    drwav wav;
    drwav_data_format format;
    int i, c;

    int channels = buf->channels;
    tmpbuf = (float *)LPMemoryPool.alloc(LP_SOUNDFILE_BUFSIZE * buf->channels, sizeof(float));

    format.container = drwav_container_riff;
    format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
    format.channels = buf->channels;
    format.sampleRate = buf->samplerate;
    format.bitsPerSample = 32;

    drwav_init_file_write(&wav, path, &format, NULL);

    count = 0;

    for(i=0; i < buf->length; i++) {
        for(c=0; c < channels; c++) {
            tmpbuf[count * channels + c] = (float)buf->data[i * channels + c];
        }

        count++;

        if (count >= LP_SOUNDFILE_BUFSIZE) {
            drwav_write_pcm_frames(&wav, LP_SOUNDFILE_BUFSIZE, tmpbuf);
            count = 0;
        }
    }

    if (count != 0) {
        drwav_write_pcm_frames(&wav, count, tmpbuf);
    }

    drwav_uninit(&wav);
    LPMemoryPool.free(tmpbuf);
}


const lpsoundfile_factory_t LPSoundFile = { read_soundfile, write_soundfile };

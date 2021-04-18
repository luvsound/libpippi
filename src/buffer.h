#ifndef LP_BUFFER_H
#define LP_BUFFER_H

typedef struct buffer_t {
    lpfloat_t* data;
    size_t length;
    int samplerate;
    int channels;
} buffer_t;

buffer_t* init_buffer(size_t length, int channels, int samplerate) {
    buffer_t* buf = (buffer_t*)calloc(1, sizeof(buffer_t));
    buf->data = (lpfloat_t*)calloc(length * channels, sizeof(lpfloat_t));
    buf->channels = channels;
    buf->length = length;
    buf->samplerate = samplerate;
    return buf;
}

void destroy_buffer(buffer_t* buf) {
    free(buf->data);
    free(buf);
}

#endif

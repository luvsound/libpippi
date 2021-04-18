#ifndef LP_BUFFER_H
#define LP_BUFFER_H

typedef struct buffer_t {
    lpfloat_t* data;
    size_t length;
    float samplerate;
    int channels;
} buffer_t;

void destroy_buffer(buffer_t* buf) {
    free(buf->data);
    free(buf);
}

#endif

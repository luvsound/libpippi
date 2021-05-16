#include "ringbuffer.h"

ringbuffer_t * ringbuffer_create(size_t length, int channels, int samplerate) {
    ringbuffer_t * ringbuf;
    ringbuf = (ringbuffer_t*)MemoryPool.alloc(1, sizeof(ringbuffer_t));
    ringbuf->buf = Buffer.create(length, channels, samplerate);
    ringbuf->pos = 0;
    return ringbuf;
}

buffer_t * ringbuffer_read(ringbuffer_t * ringbuf, size_t length) {
    int i, c;
    size_t pos = ringbuf->pos - length;
    buffer_t * out;

    pos = pos % ringbuf->buf->length;
    out = Buffer.create(length, ringbuf->buf->channels, ringbuf->buf->samplerate);
    for(i=0; i < length; i++) {
        for(c=0; c < ringbuf->buf->channels; c++) {
            out->data[i * out->channels + c] = ringbuf->buf->data[pos * ringbuf->buf->channels + c];
        }

        pos += 1;
        pos = pos % ringbuf->buf->length;
    }

    return out;
}

void ringbuffer_write(ringbuffer_t * ringbuf, buffer_t * buf) {
    int i, c, j;
    for(i=0; i < buf->length; i++) {
        for(c=0; c < ringbuf->buf->channels; c++) {
            j = c % buf->channels;
            ringbuf->buf->data[ringbuf->pos * ringbuf->buf->channels + c] = buf->data[i * buf->channels + j];
        }

        ringbuf->pos += 1;
        ringbuf->pos = ringbuf->pos % ringbuf->buf->length;
    }
}

void ringbuffer_dub(ringbuffer_t * ringbuf, buffer_t * buf) {
    int i, c, j;
    for(i=0; i < buf->length; i++) {
        for(c=0; c < ringbuf->buf->channels; c++) {
            j = c % buf->channels;
            ringbuf->buf->data[ringbuf->pos * ringbuf->buf->channels + c] += buf->data[i * buf->channels + j];
        }

        ringbuf->pos += 1;
        ringbuf->pos = ringbuf->pos % ringbuf->buf->length;
    }
}

void ringbuffer_destroy(ringbuffer_t * ringbuf) {
    Buffer.destroy(ringbuf->buf);
    MemoryPool.free(ringbuf);
}

const ringbuffer_factory_t RingBuffer = { ringbuffer_create, ringbuffer_read, ringbuffer_write, ringbuffer_dub, ringbuffer_destroy };

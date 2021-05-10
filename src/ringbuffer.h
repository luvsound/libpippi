#ifndef LP_RINGBUFFER_H
#define LP_RINGBUFFER_H

#include "pippicore.h"

typedef struct ringbuffer_t {
    buffer_t * buf;
    size_t pos;
} ringbuffer_t;

typedef struct ringbuffer_factory_t {
    buffer_t * (*create)(size_t, int, int);
    buffer_t * (*read)(ringbuffer_t *, size_t);
    buffer_t * (*write)(ringbuffer_t *, buffer_t *);
    buffer_t * (*destroy)(ringbuffer_t *);
} ringbuffer_factory_t;

extern const ringbuffer_factory_t RingBuffer;

#endif

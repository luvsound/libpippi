#ifndef LP_RINGBUFFER_H
#define LP_RINGBUFFER_H

#include "pippicore.h"

typedef struct ringbuffer_t {
    buffer_t * buf;
    size_t pos;
} ringbuffer_t;

typedef struct ringbuffer_factory_t {
    ringbuffer_t * (*create)(size_t, int, int);
    buffer_t * (*read)(ringbuffer_t *, size_t);
    void (*write)(ringbuffer_t *, buffer_t *);
    lpfloat_t (*readone)(ringbuffer_t *);
    void (*writeone)(ringbuffer_t *, lpfloat_t);
    void (*dub)(ringbuffer_t *, buffer_t *);
    void (*destroy)(ringbuffer_t *);
} ringbuffer_factory_t;

extern const ringbuffer_factory_t RingBuffer;

#endif

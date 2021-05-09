#ifndef LP_MEMORYPOOL_H
#define LP_MEMORYPOOL_H

#include "pippicore.h"

typedef struct memorypool_factory_t {
    unsigned char * pool;
    size_t poolsize;
    size_t pos;

    void * (*alloc)(size_t, size_t);
    void (*init)(unsigned char *, size_t);
} memorypool_factory_t;

extern memorypool_factory_t MemoryPool;

#endif

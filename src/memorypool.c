#include "memorypool.h"

void * memorypool_alloc(size_t itemcount, size_t itemsize);
void memorypool_init(unsigned char * pool, size_t poolsize);
memorypool_factory_t MemoryPool = { 0, 0, 0, memorypool_alloc, memorypool_init };

void * memorypool_alloc(size_t itemcount, size_t itemsize) {
    void * p;
    size_t length;

    assert(MemoryPool.pool != 0); 
    length = itemcount * itemsize;

    if(MemoryPool.poolsize >= MemoryPool.pos + length) {
        p = (void *)(&MemoryPool.pool[MemoryPool.pos]);
        MemoryPool.pos += length;
        return p;
    }
    exit(EXIT_FAILURE);
}

void memorypool_init(unsigned char * pool, size_t poolsize) {
    assert(poolsize >= 1);
    MemoryPool.pool = pool;
    MemoryPool.poolsize = poolsize;
    MemoryPool.pos = 0;
}


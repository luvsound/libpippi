#ifndef LP_WINDOW_H
#define LP_WINDOW_W

#include "pippicore.h"

typedef struct window_factory_t {
    buffer_t* (*create)(char* name, size_t length);
    void (*destroy)(buffer_t*);
} window_factory_t;

extern const window_factory_t Window;

#endif

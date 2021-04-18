#ifndef LP_SOUNDFILE_H
#define LP_SOUNDFILE_H

#include "pippicore.h"

typedef struct soundfile_factory_t {
    void (*write)(char*, buffer_t*);
} soundfile_factory_t;

extern const soundfile_factory_t SoundFile;

#endif

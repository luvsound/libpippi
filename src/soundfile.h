#ifndef LP_SOUNDFILE_H
#define LP_SOUNDFILE_H

#include "pippicore.h"

typedef struct lpsoundfile_factory_t {
    lpbuffer_t * (*read)(const char *);
    void (*write)(const char *, lpbuffer_t *);
} lpsoundfile_factory_t;

extern const lpsoundfile_factory_t LPSoundFile;

#endif

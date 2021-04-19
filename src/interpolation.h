#ifndef LP_INTERPOLATION_H
#define LP_INTERPOLATION_H

#include "pippicore.h"

typedef struct interpolation_factory_t {
    lpfloat_t (*linear)(lpfloat_t*, int, lpfloat_t);
} interpolation_factory_t;

extern const interpolation_factory_t Interpolation;

#endif

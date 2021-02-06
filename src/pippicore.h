#ifndef PIPPICORE_H
#define PIPPICORE_H

#include <string.h>
#include <math.h>

#ifdef LPFLOAT
typedef float lpfloat_t;
#else
typedef double lpfloat_t;
#endif

#ifndef PI
#define PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062
#endif

/* Utilities
 *
 * Small collection of utility functions
 */
int imax(int a, int b) {
    if(a > b) {
        return a;
    } else {
        return b;
    }
}

int paramcount(char* str) {
    int count = 1;
    int i = 0;

    while(str[i] != '\0') {
        char c = str[i];
        i += 1;
        if(c == ',') {
            count += 1;
        }
    }

    return count;
}

#endif

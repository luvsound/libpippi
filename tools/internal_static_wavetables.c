#include "pippicore.h"

#include <stdio.h>
#include <ctype.h>

#define DEFAULT_WAVETABLE_SIZE 4096

void make_wtstring(const char * wtname, size_t wtname_length, size_t wtsize) {
    int i;
    buffer_t * wt;

    printf("lpfloat_t LP_");

    for(i=0; i < wtname_length; i++) {
        printf("%c", toupper(wtname[i]));
    }

    printf("_STATIC = {");

    wt = Wavetable.create((char *)wtname, wtsize);

    for(i=0; i < wtsize; i++) {
        if(i == wtsize-1) {
            printf("%.16ff", wt->data[i]);
        } else {
            printf("%.16ff, ", wt->data[i]);
        }
    }

    printf("};\n");
}

int main() {
    size_t wtsize = DEFAULT_WAVETABLE_SIZE;
    make_wtstring("sine", 4, wtsize); 
    make_wtstring("tri", 3, wtsize); 

    return 0;
}

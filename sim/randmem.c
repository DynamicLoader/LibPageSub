#include "sim.h"

#ifndef SIM_WITH_LIBC
#error "This suit requires libc to be enabled."
#endif

#include <stdlib.h>

int a[4096];
int b[4096];

int main(int argc, const char** argv)
{
    if (argc < 2) {
        return 1;
    }

    int r = atoi(argv[1]);


    if(argc == 3){
        int sr = atoi(argv[2]);
        srand(sr);
    }

    for (int i = 0; i < r; i++) {
        int k =  rand();
        a[i] = k;
        for (int j = 0; j < (k & 0xF); j++)
            b[rand() & 0xFFF] = a[rand() & 0xFFF];
    }
    return 0;
}

SIM_EXPORT("RANDMEM", main,3)

/**
 * @brief A simple test C file for simulating, only writes to an array.
 * @details Should be compiled with -nostdlib -nostartfiles -O1. 
   Basic setup is done in simulator.
 */


int a[4096];
int b[4096];
int c[4096];

int start(){
    for(int i =0;i<4096;i++){
        a[i] = i;
        b[i] = i*2;
        c[i] = a[i] + b[i];
    }
    return 0;
}
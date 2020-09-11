/*
 * calloc_and_free.c - Tests special calloc() case in multi-thread mode 
 * compiling with -lpthread since dlsym() uses calloc() at the first time,
 * and they could call each other recursively.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main() {
    char *ptr = (char *)calloc(4, sizeof(char));
    sleep(6);
    free(ptr);
    return 0;
}

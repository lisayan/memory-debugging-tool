/*
 * realloc_and_free.c - Tests all 4 behaviors of realloc.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main() {
    // Case 1: Behaves like malloc.
    void *ptr = realloc(NULL, 4);
    // Case 2: Same pointer, smaller size.
    sleep(5);
    void *new_ptr = realloc(ptr, 2);
    //assert(new_ptr == ptr);
    // Case 3: New pointer, bigger size.
    sleep(5);
    void *new_new_ptr = realloc(new_ptr, 4);
    // Case 4: Behaves like free.
    sleep(5);
    void *null_ptr = realloc(new_new_ptr, 0);
    //assert(null_ptr == NULL);
    return 0;
}

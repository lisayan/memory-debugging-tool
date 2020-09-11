#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main() {
    void *ptr = malloc(4);
    sleep(5);
    free(ptr);
    return 0;
}

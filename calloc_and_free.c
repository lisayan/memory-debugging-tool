#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main() {
    void *ptr = calloc(4, sizeof(char));
    sleep(6);
    free(ptr);
    return 0;
}

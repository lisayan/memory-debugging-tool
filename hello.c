#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main() {
    void *ptr = malloc(4);
    free(ptr);
    sleep(5);
    ptr = malloc(4);
    free(ptr);
    printf("Hello World \n");
    return 0;
}

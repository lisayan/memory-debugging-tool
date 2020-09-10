#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    void *ptr = malloc(4);
    free(ptr);
    time_t curtime;
    time(&curtime);
    printf("Hello World %s\n", ctime(&curtime));
    return 0;
}

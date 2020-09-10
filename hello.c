#include <stdio.h>
#include <stdlib.h>

int main() {
    void *ptr = malloc(4);
    free(ptr);
    printf("Hello World\n");
    return 0;
}

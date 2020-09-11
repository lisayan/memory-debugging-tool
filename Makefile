CC=gcc

malloc_and_free: malloc_and_free.c
	gcc -o malloc_and_free malloc_and_free.c

calloc_and_free: calloc_and_free.c
	gcc -o calloc_and_free calloc_and_free.c

malloc_wrappers.so: malloc_wrappers.c 
	gcc -shared -fPIC -o malloc_wrappers.so malloc_wrappers.c -ldl

clean:
	rm malloc_and_free calloc_and_free malloc_wrappers.so

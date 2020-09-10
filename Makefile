CC=gcc

hello: hello.c
	gcc -o hello hello.c
	
malloc_wrappers.so: malloc_wrappers.c 
	gcc -shared -fPIC -o malloc_wrappers.so malloc_wrappers.c -ldl

clean:
	rm hello malloc_wrappers.so

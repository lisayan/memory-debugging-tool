/*
 * malloc_wrappers.c - Run-time library interposition of malloc functions.
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t lock;

void *malloc(size_t size) {
    printf("HIIIII\n");
    pthread_mutex_lock(&lock);
    static void *(*mallocp)(size_t s);
    char *error;
    void *ptr;

    if (!mallocp) {
        mallocp = dlsym(RTLD_NEXT, "malloc");
        if ((error = dlerror()) != NULL) {
            fputs(error, stderr);
            exit(1);
        }
    }
    ptr = mallocp(size);
    printf("My malloc called\n");
    pthread_mutex_unlock(&lock);
    return ptr;
}

void free(void *ptr) {
    printf("HELLOOOOO\n");
    pthread_mutex_lock(&lock);
    static void (*freep)(void *);
    char *error;

    if (!freep) {
        freep = dlsym(RTLD_NEXT, "free");
        if ((error = dlerror()) != NULL) {
            fputs(error, stderr);
            exit(1);
        }
    }
    pthread_mutex_unlock(&lock);
    return;
}

void *calloc(size_t nmemb, size_t size) {
    pthread_mutex_lock(&lock);

    static void *(*callocp)(size_t nelt, size_t eltsize);
    char *error;
    void *ptr;

    if (!callocp) {
        callocp = dlsym(RTLD_NEXT, "calloc");
        if ((error = dlerror()) != NULL) {
            fputs(error, stderr);
            exit(1);
        }
    }
    ptr = callocp(nmemb, size);
    pthread_mutex_unlock(&lock);
    return ptr;
}

void *realloc(void *buf, size_t size) {
    pthread_mutex_lock(&lock);
    static void *(*reallocp)(void *b, size_t s);
    char *error;
    void *ptr;

    if (!reallocp) {
        reallocp = dlsym(RTLD_NEXT, "realloc");
        if ((error = dlerror()) != NULL) {
            fputs(error, stderr);
            exit(1);
        }
    }
    ptr = reallocp(buf, size);
    pthread_mutex_unlock(&lock);
    return ptr;
}

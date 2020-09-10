/* 
 * malloc_wrappers.c - Run-time library interposition of malloc functions.
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static __thread int no_hook;
pthread_mutex_t lock;
static void *(*real_malloc)(size_t s) = NULL;
static void (*real_free)(void *) = NULL;
static void *(*real_calloc)(size_t nelt, size_t eltsize) = NULL;
static void *(*real_realloc)(void *b, size_t s) = NULL;
char *message = "%s";

static void init() {
  real_malloc = dlsym(RTLD_NEXT, "malloc");
  real_free = dlsym(RTLD_NEXT, "free");
  real_calloc = dlsym(RTLD_NEXT, "calloc");
  real_realloc = dlsym(RTLD_NEXT, "realloc");

  if (!real_malloc || !real_free || !real_calloc || !real_realloc) {
    fputs(dlerror(), stderr);
    exit(1);
  }
}

static void printout() {
  //char* message;
  time_t current_time;
  time(&current_time);
  //fprintf(stderr, "%s", ctime(&current_time));
  //fputs(message, stderr);
  fputs(ctime(&current_time), stderr);
}

void *malloc(size_t size) {
    pthread_mutex_lock(&lock);
    void *caller;
    if (!real_malloc) {
      init();
    }
    if (no_hook) {
      return real_malloc(size);
    }
    no_hook = 1;
    caller = __builtin_return_address(0);
    void *ptr = real_malloc(size);
    fputs("My malloc called\n", stderr);
    printout();
    no_hook = 0;
    pthread_mutex_unlock(&lock);
    return ptr;
}

void free(void *ptr) {
    pthread_mutex_lock(&lock);
    if (!real_free) {
        init();
    }
    real_free(ptr);
    fputs("My free called\n", stderr);
    pthread_mutex_unlock(&lock);
    return;
}

void *calloc(size_t nmemb, size_t size) {
    pthread_mutex_lock(&lock);
    if (!real_calloc) {
      init();
    }
    void *ptr = real_calloc(nmemb, size);
    pthread_mutex_unlock(&lock);
    return ptr;
}

void *realloc(void *buf, size_t size) {
    pthread_mutex_lock(&lock);
    if (!real_realloc) {
    }
    void *ptr = real_realloc(buf, size);
    pthread_mutex_unlock(&lock);
    return ptr;
}

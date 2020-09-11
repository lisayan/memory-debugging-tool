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

/* Malloc wrapper definitions. */
static __thread int no_hook;
pthread_mutex_t lock;
static void *(*real_malloc)(size_t s) = NULL;
static void (*real_free)(void *) = NULL;
static void *(*real_calloc)(size_t nelt, size_t eltsize) = NULL;
static void *(*real_realloc)(void *b, size_t s) = NULL;

/* Memory debugging tool printout variables. */
int first_printout = 0;
char message[10000];
time_t last_print_time;

/* Memory debugging tool stats. */
int current_allocations = 0;
int overall_allocations = 0;
// NOTE: This is an approximation since realloc doesn't know the pointer size
// without keeping track of pointer memory block sizes manually.
int total_allocated_size_bytes = 0;

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
  if (!first_printout) {
    char *header = ">>>>>>>>>>>>> %s <<<<<<<<<<<<<\n";
    char *overall_stats = "Overall stats:\n%d Current allocations\n%d Overall "
      "allocations since start\n%d%s Current total allocated size\n\n";
    char *by_size = "Current allocations by size: (# = 8,123 current "
      "allocations)\n0 - 4 bytes: %s\n4 - 8 bytes: %s\n8 - 16 bytes: %s\n"
      "16 - 32 bytes: %s\n32 - 64 bytes: %s\n64 - 128 bytes: %s\n128 - 256 "
      "bytes: %s\n256 - 512 bytes: %s\n512 - 1024 bytes: %s\n1024 - 2048 bytes: "
      "%s\n2048 - 4096 bytes: %s\n4096 + bytes: %s\n\n";
    char *by_age = "Current allocations by age: (# = 8,123 current allocations)\n"
      "< 1 sec: %s\n< 10 sec: %s\n< 100 sec: %s\n< 1000 sec: %s\n> 1000 sec: %s\n\n";
    strcat(message, header);
    strcat(message, overall_stats);
    first_printout = 1;
  }
  time_t current_time;
  time(&current_time);
  char* byte_unit = "B";
  if (difftime(current_time, last_print_time) >= 5) {
    fprintf(stderr, message, ctime(&current_time), current_allocations,
          overall_allocations, total_allocated_size_bytes, byte_unit);
    time(&last_print_time);
  }
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
    current_allocations++;
    overall_allocations++;
    total_allocated_size_bytes += size;
    printout();
    no_hook = 0;
    pthread_mutex_unlock(&lock);
    return ptr;
}

void free(void *ptr) {
    pthread_mutex_lock(&lock);
    void *caller;
    if (!real_free) {
        init();
    }
    if (no_hook) {
      return real_free(ptr);
    }
    no_hook = 1;
    caller = __builtin_return_address(0);
    real_free(ptr);
    current_allocations--;
    printout();
    no_hook = 0;
    pthread_mutex_unlock(&lock);
    return;
}

void *calloc(size_t nmemb, size_t size) {
    pthread_mutex_lock(&lock);
    void *caller;
    if (!real_calloc) {
      init();
    }
    if (no_hook) {
      return real_calloc(nmemb, size);
    }
    no_hook = 1;
    caller = __builtin_return_address(0);
    void *ptr = real_calloc(nmemb, size);
    current_allocations++;
    overall_allocations++;
    total_allocated_size_bytes += size;
    printout();
    no_hook = 0;
    pthread_mutex_unlock(&lock);
    return ptr;
}

void *realloc(void *buf, size_t size) {
    pthread_mutex_lock(&lock);
    void *caller;
    if (!real_realloc) {
      init();
    }
    if (no_hook) {
      return real_realloc(buf, size);
    }
    no_hook = 1;
    caller = __builtin_return_address(0);
    if (!buf) {
      // If buf is NULL, equivalent to malloc(size).
      current_allocations++;
      overall_allocations++;
      total_allocated_size_bytes += size;
    }
    if (buf && size == 0) {
      // If buf is not NULL and size is 0, equivalent to free(buf).
      current_allocations--;
    }
    void *ptr = real_realloc(buf, size);
    printout();
    no_hook = 0;
    pthread_mutex_unlock(&lock);
    return ptr;
}

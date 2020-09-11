/* 
 * malloc_wrappers.c - Run-time library interposition of malloc functions.
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <math.h>
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

struct AllocationInfo {
  // Pointer to memory allocation.
  void *ptr;
  // Unix time in seconds when memory was allocated.
  time_t init_time;
  // Size of memory allocation in bytes.
  size_t size;
};

/* Memory debugging tool printout variables. */
int first_printout = 0;
char message[10000];
time_t last_print_time;

/* Memory debugging tool stats. */
int current_allocations = 0;
int overall_allocations = 0;
int total_allocated_size_bytes = 0;
#define BYTE_POWER 12
static int allocations_by_size[12];
// The number of current allocations 1 '#' represents.
#define SYMBOL_COUNT 1

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

static char* calculate_symbols(int num_allocations) {
  // Allocate space for null terminating character.
  int num_symbols = num_allocations / SYMBOL_COUNT + 1;
  if (num_symbols > 1) {
    char *symbols = (char *)real_malloc(num_symbols*sizeof(char));
    for (int i = 0; i < num_symbols-1; i++) {
      strcat(symbols, "#");
    }
    symbols[num_symbols-1] = '\0';
    return symbols;
  }
  return NULL;
}

static void printout() {
  pthread_mutex_lock(&lock);
  if (!first_printout) {
    char *header = ">>>>>>>>>>>>> %s <<<<<<<<<<<<<\n";
    char *overall_stats = "Overall stats:\n%d Current allocations\n%d Overall "
      "allocations since start\n%d%s Current total allocated size\n\n";
    char *by_size = "Current allocations by size: (# = %d current "
      "allocations)\n0 - 4 bytes: %s\n4 - 8 bytes: %s\n8 - 16 bytes: %s\n"
      "16 - 32 bytes: %s\n32 - 64 bytes: %s\n64 - 128 bytes: %s\n128 - 256 "
      "bytes: %s\n256 - 512 bytes: %s\n512 - 1024 bytes: %s\n1024 - 2048 bytes: "
      "%s\n2048 - 4096 bytes: %s\n4096 + bytes: %s\n\n";
    char *by_age = "Current allocations by age: (# = 1 current allocation)\n"
      "< 1 sec: %s\n< 10 sec: %s\n< 100 sec: %s\n< 1000 sec: %s\n> 1000 sec: %s\n\n";
    strcat(message, header);
    strcat(message, overall_stats);
    strcat(message, by_size);
    first_printout = 1;
  }
  // Overall stats
  time_t current_time;
  time(&current_time);
  char* byte_unit = "B";

  // Current allocations by size
  char *one = calculate_symbols(allocations_by_size[0]);
  char *two = calculate_symbols(allocations_by_size[1]);
  char *three = calculate_symbols(allocations_by_size[2]);
  char *four = calculate_symbols(allocations_by_size[3]);
  char *five = calculate_symbols(allocations_by_size[4]);
  char *six = calculate_symbols(allocations_by_size[5]);
  char *seven = calculate_symbols(allocations_by_size[6]);
  char *eight = calculate_symbols(allocations_by_size[7]);
  char *nine = calculate_symbols(allocations_by_size[8]);
  char *ten = calculate_symbols(allocations_by_size[9]);
  char *eleven = calculate_symbols(allocations_by_size[10]);
  char *twelve = calculate_symbols(allocations_by_size[11]);
  
  // Print formatted string.
  if (difftime(current_time, last_print_time) >= 5) {
    fprintf(stderr, message, ctime(&current_time), current_allocations,
          overall_allocations, total_allocated_size_bytes, byte_unit,
          SYMBOL_COUNT, one, two, three, four, five, six, seven, eight,
          nine, ten, eleven, twelve);
    time(&last_print_time);
  }
  real_free(one);
  real_free(two);
  real_free(three);
  real_free(four);
  real_free(five);
  real_free(six);
  real_free(seven);
  real_free(eight);
  real_free(nine);
  real_free(ten);
  real_free(eleven);
  real_free(twelve);
  pthread_mutex_unlock(&lock);
 }

static int calculate_size_index(size_t size) {
  if (size >= 0 && size < 4) {
    return 0;
  } else if (size >= 4 && size < 8) {
    return 1;
  } else if (size >= 8 && size < 16) {
    return 2;
  } else if (size >= 16 && size < 32) {
    return 3;
  } else if (size >= 32 && size < 64) {
    return 4;
  } else if (size >= 64 && size < 128) {
    return 5;
  } else if (size >= 128 && size < 256) {
    return 6;
  } else if (size >= 256 && size < 512) {
    return 7;
  } else if (size >= 512 && size < 1024) {
    return 8;
  } else if (size >= 1024 && size < 2048) {
    return 9;
  } else if (size >= 2048 && size < 4096) {
    return 10;
  } else if (size >= 4096) {
    return 11;
  }
  // Shouldn't return here.
  printf("Oh no!\n");
  return -1;
}

void *malloc(size_t size) {
    void *caller;
    if (!real_malloc) {
      init();
    }
    if (no_hook) {
      return real_malloc(size);
    }
    no_hook = 1;
    caller = __builtin_return_address(0);
    void *ptr = real_malloc(sizeof(struct AllocationInfo) + size);
    
    // Save memory stats.
    pthread_mutex_lock(&lock);
    current_allocations++;
    overall_allocations++;
    total_allocated_size_bytes += size;
    allocations_by_size[calculate_size_index(size)] += 1;
    time_t init_time;
    time(&init_time);
    struct AllocationInfo info = {ptr, init_time, size};
    // Save info before the allocated block.
    *(struct AllocationInfo *)ptr = info;
    pthread_mutex_unlock(&lock);

    printout();
    no_hook = 0;
    return (void *)ptr + sizeof(struct AllocationInfo);
}

void free(void *ptr) {
    void *caller;
    if (!real_free) {
        init();
    }
    if (no_hook) {
      return real_free(ptr);
    }
    no_hook = 1;
    caller = __builtin_return_address(0);
    
    // Read memory stats.
    pthread_mutex_lock(&lock);
    void *info_ptr = ptr - sizeof(struct AllocationInfo);
    struct AllocationInfo info = *(struct AllocationInfo *)info_ptr;
    real_free(info_ptr);
    current_allocations--;
    total_allocated_size_bytes -= info.size;
    allocations_by_size[calculate_size_index(info.size)] -= 1;
    pthread_mutex_unlock(&lock);

    printout();
    no_hook = 0;
    return;
}

void *calloc(size_t nmemb, size_t size) {
    // Use custom malloc for easier memory stats management.
    void *ptr = malloc(sizeof(nmemb*size));
    memset(ptr, 0, nmemb*size);
    return ptr;
}

void *realloc(void *buf, size_t size) {
    void *ptr = NULL;
    if (!buf) {
      // If buf is NULL, equivalent to malloc(size).
      ptr = malloc(size); 
      return ptr;
    } else if (buf != NULL && size == 0) {
      // If buf is not NULL and size is 0, equivalent to free(buf).
      free(buf);
      return NULL;
    } 
    
    void *info_ptr = buf - sizeof(struct AllocationInfo);
    struct AllocationInfo info = *(struct AllocationInfo *)info_ptr;
    if (buf != NULL && size <= info.size) {
      // New size is smaller, allocate new memory and copy data up until new
      // size.
      ptr = malloc(size);
      memcpy(ptr, buf, size);
      free(buf);
      return ptr;
    } else if (buf != NULL && size > info.size) {
      // New size is larger, allocate new memory and copy data up until old
      // size.
      ptr = malloc(size);
      memcpy(ptr, buf, info.size);
      free(buf);
      return ptr;
    }
    return ptr;
}

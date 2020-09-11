# Memory Debugging Tool in C

This page describes how to write a glibc-based shared library object (.so) that
wraps libc's malloc(), realloc(), calloc(), and free() and instruments them to
provide number of allocations, size, and age statistics printed to stderr.
Details are in the PDF.

## How to Build and Run

There are three tests: malloc_and_free, calloc_and_free, realloc_and_free.
This was tested on Debian GNU/Linux.
To build and run malloc_and_free:

```
make malloc_and_free
make malloc_wrappers.so
LD_BIND_NOW=1 LD_PRELOAD=./malloc_wrappers.so ./malloc_and_free
```

To build and run calloc_and_free:

```
make calloc_and_free
make malloc_wrappers.so
LD_BIND_NOW=1 LD_PRELOAD=./malloc_wrappers.so ./calloc_and_free
```

To build and run realloc_and_free:

```
make realloc_and_free
make malloc_wrappers.so
LD_BIND_NOW=1 LD_PRELOAD=./malloc_wrappers.so ./realloc_and_free
```

## Implementation Details

### Tracking size of an allocation

The tricky part about tracking size statistics of current
allocations is that traditionally, free() has no knowledge of the pointer's
size when it deallocates. My idea was to save this info in a struct
and write it *before* the allocated block so that free() can access it later.

I defined an AllocationInfo struct like so:

```
struct AllocationInfo {
  void *ptr;
  time_t init_time;
  size_t size;
};
```

When `malloc(2)` is called, under the hood I actually call
`real_malloc(sizeof(struct AllocationInfo) + 2)`. I write the struct to the
beginning of the true allocated block, and then return an incremented pointer
that points after where I wrote the struct.

However, [the Linux manual for free()](https://man7.org/linux/man-pages/man3/free.3.html)
states that the `void *ptr` in `free(void *ptr)` must have been returned by a
previous call to `malloc()`, `calloc()`, or `realloc()`. In other words, I
cannot simply call `free()` on the pointer I returned from `malloc()` since I
know that is not the beginning of the allocated block. Thus, under the hood I
call `real_free(ptr-sizeof(struct AllocationInfo))`.

### Why I call malloc() in calloc() instead of real_calloc()
There is a special reason why I decided to call malloc() and memset() the
allocated memory manually instead of simply calling real_calloc(). According to
[slide 34 and 35 of this lecture](https://elinux.org/images/b/b5/Elc2013_Kobayashi.pdf)
If you compile the program in multi-threaded mode with `-lpthread`, `dlsym()` and
`calloc()` call each other recursively until there is a segmentation fault. The
workaround for this is defining a static global buffer to return a pointer to 
pre-allocated memory the first time `dlsym()` calls `calloc()`, and then later
calling `real_calloc()` when it is initialized.

This would work fine in a normal scenario, except since we are tracking
statistics, `free()` is expecting a struct in front of the pointer. So for this
purpose, it is better to call `malloc()` and manually clear the memory.

### Tracking age of an allocation

Unfortunately, I ran out of time before I was able to implement this, but here
I will outline how I would have done it.

If I did this in C++, I could use `std::unordered_map` and map
pointer -> struct AllocationInfo. Each time `malloc()` is called, I would insert
the info into the hashmap, and I would delete it when `free()` is called. When
the message is printing to stderr, the hashmap represents the current
allocations and I would sort the ages into buckets.

In C, there are no hashmaps, so more manual effort is required. I would
implement helper functions to dynamically add or remove elements from an array,
taking care of allocating new memory if necessary. Similar to above, I would
keep an array of struct AllocationInfo and add one in `malloc()` and remove
one in `free()`. When the message is printing to stderr, the array represents
the current allocations and it would be O(n) to sort the ages into buckets.

## Resources
[Function Interposition in C](https://www.geeksforgeeks.org/function-interposition-in-c-with-an-example-of-user-defined-malloc/)

[Tips of Malloc and Free: Making your own malloc library for troubleshooting](https://elinux.org/images/b/b5/Elc2013_Kobayashi.pdf)

[Stack Overflow: Cannot reproduce segfault in gdb](https://stackoverflow.com/questions/7057651/cannot-reproduce-segfault-in-gdb)

[Using make and writing Makefiles](https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html)


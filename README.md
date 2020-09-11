# Memory Debugging Tool in C

This page describes how to write a glibc-based shared library object (.so) that
wraps libc's malloc(), realloc(), calloc(), and free() and instruments them to
provide number of allocations, size, and age statistics printed to stderr.
Details are in the uploaded PDF.

[TOC]

## How to Build and Run

There are three tests: malloc_and_free, calloc_and_free, realloc_and_free.
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

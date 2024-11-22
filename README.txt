Authors:
Grayson & Esdras

Files:
1) test.c: A test enviroment for writting and running specific tests for any of the functions in my-malloc.c
2) my-malloc.c Contains implementations of a number of heap functions. Our implementation of malloc relies on a singly linked list. The file also contains implementations of calloc, realloc, free, and malloc_usable_size.
3) Makefile Commands:
    "make": builds my-malloc.so file from my-malloc.c
    "make test": builds test (exe) file from test.c
    "make debug": builds test file (if needed), then runs gdb with env using preload to allow debuging of functions in my-malloc
    "make clean": Cleans up workspace. Removes test (exe) and my-malloc.so if present. 
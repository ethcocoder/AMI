#include "core.h"
#include <stdlib.h>
#include <stdio.h>

void* ami_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Fatal Error: Memory allocation failed for %zu bytes\n", size);
        exit(1);
    }
    return ptr;
}

void ami_free(void* ptr) {
    if (ptr) {
        free(ptr);
    }
}

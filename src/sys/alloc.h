#ifndef ALLOC_H
#define ALLOC_H

#include <stdlib.h>

#define danpa_alloc(size) malloc(size)
#define danpa_realloc(ptr, size) realloc(ptr, size)
#define danpa_calloc(size) calloc(1, size)

#endif // ALLOC_H

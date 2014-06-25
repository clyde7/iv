#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>
#include <alloca.h>
#include "string.h"

#define alloc_size(Type) (Type *) alloca(sizeof(Type))
#define alloc_array(Type, count) (Type *) alloca((count) * sizeof(Type))

#define malloc_size(Type) (Type *) malloc(sizeof(Type))
#define malloc_array(Type, count) (Type *) malloc((count) * sizeof(Type))

#define calloc_size(Type) (Type *) calloc(1, sizeof(Type))
#define calloc_array(Type, count) (Type *) calloc((count), sizeof(Type))

#define realloc_array(Type, array, count) (Type *) realloc(array, (count) * sizeof(Type))

#define clear(Type, pointer) memset((pointer), 0, sizeof(Type))
#define clear_array(Type, array, count) memset((array), 0, (count) * sizeof(Type))

#define copy_array(Type, source, size) (Type *) copy_array_((source), (size) * sizeof(Type));

#define cuda_malloc(address, size) \
        cudaMalloc((void **) (&(address)), size)
#define cuda_malloc_size(address, type) \
        cudaMalloc((void **) (&(address)), sizeof(type))
#define cuda_malloc_array(address, type, count) \
        cudaMalloc((void **) (&(address)), sizeof(type) * (count))
#define cuda_push(target, source, size) \
        cudaMemcpy((target), (source), (size), cudaMemcpyHostToDevice)
#define cuda_pull(target, source, size) \
        cudaMemcpy((target), (source), (size), cudaMemcpyDeviceToHost)
#define cuda_push_const(symbol, source) \
        cudaMemcpyToSymbol((symbol), (source), (size))

void memory_randomize(void *, int size);
void * copy_array_(void const * source, int size);

#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>





typedef struct  gc_circular_buffer
{
    size_t size;
    size_t enqP;
    size_t deqP;
    int *ptr;
} gc_circular_buffer_t;

gc_circular_buffer_t *gc_circular_buffer_create(size_t size)
{
    void *void_ptr =  malloc(sizeof(gc_circular_buffer_t) + size * sizeof(int));
    gc_circular_buffer_t *circular_buffer = (gc_circular_buffer_t*) void_ptr;
    circular_buffer->size = size;
    circular_buffer->enqP = 0;
    circular_buffer->deqP = 0;
    circular_buffer->ptr = (int*) (void_ptr - sizeof(gc_circular_buffer_t));
    return circular_buffer;
}

bool gc_circular_buffer_size(gc_circular_buffer_t *ptr)
{
    return ptr->enqP - ptr->deqP;
}

void gc_circular_buffer_push(gc_circular_buffer_t *circular_buffer , int val)
{
    circular_buffer->ptr[circular_buffer->enqP % circular_buffer->size] = val;
    circular_buffer ->enqP ++;
}

int gc_circular_buffer_front(gc_circular_buffer_t *circular_buffer)
{
    return circular_buffer->ptr[circular_buffer->deqP % circular_buffer->size];
}

void gc_circular_buffer_pop(gc_circular_buffer_t *circular_buffer)
{
    circular_buffer->deqP ++;
}

void gc_circular_buffer_destroy(gc_circular_buffer_t *ptr)
{
    free(ptr);
}


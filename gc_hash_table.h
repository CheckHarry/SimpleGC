#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>



#define MODNUM 37

typedef struct gc_hash_node
{
    void *ptr;
    struct gc_hash_node *next;
} gc_hash_node_t;


typedef struct gc_hash_bin
{
    gc_hash_node_t base;
} gc_hash_bin_t;

typedef struct gc_hash_table
{
    gc_hash_bin_t bins[MODNUM];
} gc_hash_table_t;

extern gc_hash_table_t* gc_hash_table_create();

extern void gc_hash_table_destroy(gc_hash_table_t* hash_table);

extern int gc_hash_table_contain(gc_hash_table_t* hash_table , void *ptr);

extern void gc_hash_table_insert(gc_hash_table_t* hash_table , void *ptr);

extern void gc_hash_table_erase(gc_hash_table_t* hash_table , void *ptr);

extern void verify_hash_table();

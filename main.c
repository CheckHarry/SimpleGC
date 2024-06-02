#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "gc_hash_table.h"



#define MAX_PTR_NUM 1024

typedef enum status
{
    STATUS_MARK = 1,
    STATUS_ALLOCATED = 2
} status_t;


typedef struct gc_ptr
{
    void *ptr; // pointer to the ptr returned to user
    size_t ptr_size;
    //u_int64_t hash;
    int64_t index;
    int flag;
} gc_ptr_t;


typedef struct gc
{
    void* stack_bottom;
    int64_t cur_num;
    gc_ptr_t ptrs[MAX_PTR_NUM];
    gc_hash_table_t *hash_table;
} gc_t;


gc_t *gc_create(void *bottom)
{
    gc_t * ptr = (gc_t *) malloc(sizeof(gc_t));
    if (!ptr) return NULL;
    ptr->cur_num = 0;
    ptr->stack_bottom = bottom;
    memset((void*)ptr -> ptrs , 0 , sizeof(ptr -> ptrs));
    ptr->hash_table = gc_hash_table_create();
    return ptr;
}

void gc_destroy(gc_t * ptr)
{
    gc_hash_table_destroy(ptr->hash_table);
    free(ptr);
}

void* gc_malloc(gc_t *gc , size_t size)
{
    if (gc->cur_num >= MAX_PTR_NUM) return NULL;

    void *ptr = malloc(sizeof(gc_ptr_t) + size);
    if (!ptr) return NULL;

    gc_ptr_t *header_ptr = (gc_ptr_t *) ptr;
    gc->ptrs[gc->cur_num].ptr = ptr + sizeof(gc_ptr_t);
    gc->ptrs[gc->cur_num].ptr_size = size;
    gc->ptrs[gc->cur_num].index = gc->cur_num;
    gc->ptrs[gc->cur_num].flag |= STATUS_ALLOCATED;

    *header_ptr = gc->ptrs[gc->cur_num];
    gc_hash_table_insert(gc->hash_table , gc->ptrs[gc->cur_num].ptr);

    gc->cur_num ++;
    return ptr + sizeof(gc_ptr_t);
}

void gc_print_ptr(const gc_ptr_t *ptr)
{
    printf("ptr address : %p | ptr size : %lu | ptr index : %lu | flag : %d\n" , ptr->ptr , ptr->ptr_size , ptr->index , ptr->flag);
}

void gc_print_ptr_with_ident(const gc_ptr_t *ptr , int ident)
{
    for (int i = 0 ; i < ident ; i ++)
        printf("\t");
    printf("ptr address : %p | ptr size : %lu | ptr index : %lu | flag : %d\n" , ptr->ptr , ptr->ptr_size , ptr->index , ptr->flag);
}

void gc_print(const gc_t *gc)
{
    printf("==================\n");
    for (int i = 0 ; i < MAX_PTR_NUM ; i ++)
    {
        if (gc->ptrs[i].flag & STATUS_ALLOCATED)
        {
            gc_print_ptr(&gc->ptrs[i]);
        }
    }
    printf("==================\n");
}

gc_ptr_t* get_index(gc_t *gc , void *ptr)
{
    if (!gc_hash_table_contain(gc->hash_table , ptr)) return NULL;
    //if (ptr < gc->ptr_min || ptr > gc->ptr_max) return NULL;
    gc_ptr_t *header_ptr = (gc_ptr_t*) (ptr - sizeof(gc_ptr_t));

    if (header_ptr->ptr != ptr) return NULL;
    return &(gc->ptrs[header_ptr->index]);
}

void gc_dfs(gc_t *gc , void *ptr , int ident)
{
    const gc_ptr_t* header_ptr = get_index(gc , ptr);
    if (!header_ptr) return;
    //gc_print_ptr(header_ptr);
    gc_print_ptr_with_ident(header_ptr , ident);

    for (size_t i = 0; i < header_ptr->ptr_size ; i ++)
    {
        gc_dfs(gc , *(void**)(header_ptr->ptr + i) , ident + 1);
    }
}

void gc_mark_start_from_ptr(gc_t *gc , void *ptr)
{
    gc_ptr_t* header_ptr = get_index(gc , ptr);
    if (!header_ptr) return;
    if ((header_ptr->flag & STATUS_MARK)) return;

    header_ptr->flag |= STATUS_MARK;
    for (size_t i = 0; i < header_ptr->ptr_size ; i ++)
    {
        gc_mark_start_from_ptr(gc , *(void**)(header_ptr->ptr + i));
    }
}

void gc_mark(gc_t *gc)
{
    void *cur = gc->stack_bottom;
    void *top = __builtin_frame_address(0);
    
    if (top < cur)
    {
        //printf("stack going downward\n");
        for (;cur >= top;)
        {
            gc_mark_start_from_ptr(gc , *(void**)cur);
            cur = cur - 1;
        }
    }
    else if (top > cur)
    {
        //printf("stack going upward\n");
        for (;cur <= top;)
        {
            gc_mark_start_from_ptr(gc , *(void**)cur);
            cur = cur + 1;
        }
    }
}

void gc_sweep(gc_t *gc)
{
    for (size_t i = 0 ; i < gc->cur_num ; i ++)
    {
        if (!(gc->ptrs[i].flag & STATUS_ALLOCATED)) continue;
        if (!(gc->ptrs[i].flag & STATUS_MARK))
        {
            void *ptr_to_erase = gc->ptrs[i].ptr - sizeof(gc_ptr_t);
            free(ptr_to_erase);
            gc->ptrs[i].flag = 0;
            continue;
        } 
        
        gc->ptrs[i].flag ^= STATUS_MARK;
        
    }
}


typedef struct tree_node {
    int val;
    struct tree_node *left;
    struct tree_node *right;
} tree_node_t;

tree_node_t* create_node(gc_t *gc ,int val)
{
    tree_node_t* node = (tree_node_t*) gc_malloc(gc , sizeof(tree_node_t));
    node -> val = val;
    node -> left = NULL;
    node -> right = NULL;
    return node;
}

tree_node_t* create_tree(gc_t *gc)
{
    tree_node_t *root =  create_node(gc,0);
    root -> left = create_node(gc,1);
    root -> right = create_node(gc,2);
    root -> left -> left = create_node(gc,3);
    return root;
}

int gc_main(void* (*func)(gc_t*))
{
    gc_t* gc = gc_create(__builtin_frame_address(0));
    if (!gc) {
        // error handling
        return -1;
    }

    gc_malloc(gc , 64);
    void *ptr = (void*) func(gc);
    gc_malloc(gc , 64);
    // void **vector_of_ptr = (void**) gc_malloc(gc , 8 * sizeof(void*));
    // for (int i = 0 ; i < 8 ; i ++)
    // {
    //     vector_of_ptr[i] = gc_malloc(gc , sizeof(void*));
    // }
    // gc_malloc(gc , sizeof(void*));
        
    
    gc_print(gc);
    gc_mark(gc);
    gc_print(gc);
    gc_sweep(gc);
    gc_print(gc);

    gc_destroy(gc);
    return 0;
}


int gc_main2(void* (*func)(gc_t*))
{
    gc_t* gc = gc_create(__builtin_frame_address(0));
    if (!gc) {
        // error handling
        return -1;
    }

    gc_malloc(gc , 64);
    tree_node_t *ptr = (tree_node_t*) func(gc);
    gc_malloc(gc , 64);
    
    gc_print(gc);
    gc_mark(gc);
    gc_print(gc);
    gc_sweep(gc);
    gc_print(gc);

    ptr->left = NULL;
    gc_print(gc);
    gc_mark(gc);
    gc_print(gc);
    gc_sweep(gc);
    gc_print(gc);

    gc_destroy(gc);
    return 0;
}


void func()
{
    //printf("%p\n" , __builtin_frame_address(0) );
    printf("func frame : %p\n" , __builtin_frame_address(0) );
    int y;
    printf("func y : %p\n" , &y);
}


void *create_tree_void(gc_t* gc)
{
    return (void*) create_tree(gc);
}

int main(int argc , char **argv)
{
    //verify_hash_table();
    gc_main2(&create_tree_void);
    // printf("main frame : %p\n" , __builtin_frame_address(0) );
    // printf("main x : %p\n" , &x);
    // func();
};




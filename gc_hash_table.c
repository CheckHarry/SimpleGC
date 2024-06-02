#include "gc_hash_table.h"

gc_hash_table_t* gc_hash_table_create()
{
    void *ptr = malloc(sizeof(gc_hash_table_t));
    if (!ptr) return NULL;
    gc_hash_table_t* to_return = (gc_hash_table_t*) ptr;
    for (size_t i = 0 ; i < MODNUM ; i ++)
    {
        to_return->bins[i].base.next = NULL;
    }

    return to_return;
}

void gc_hash_table_destroy(gc_hash_table_t* hash_table)
{
    if (!hash_table) return;
    for (size_t i = 0 ; i < MODNUM ; i ++)
    {
        gc_hash_node_t *cur = hash_table->bins[i].base.next;
        while (cur)
        {
            gc_hash_node_t *tmp = cur;
            cur = cur->next;
            free(tmp);
        }
    }

    free(hash_table);
}

int gc_hash_table_contain(gc_hash_table_t* hash_table , void *ptr)
{
    u_int64_t hash = (u_int64_t)(ptr) % MODNUM;

    gc_hash_node_t *cur = hash_table->bins[hash].base.next;

    while (cur)
    {
        if (cur->ptr == ptr) return 1;
        cur = cur->next;
    }

    return 0;
}

void gc_hash_table_insert(gc_hash_table_t* hash_table , void *ptr)
{
    if (gc_hash_table_contain(hash_table , ptr)) return;

    u_int64_t hash = (u_int64_t)(ptr) % MODNUM;

    gc_hash_node_t *new_node = (gc_hash_node_t *) malloc(sizeof(gc_hash_node_t));
    new_node -> next = hash_table->bins[hash].base.next;
    new_node -> ptr = ptr;
    hash_table->bins[hash].base.next = new_node;
}

void gc_hash_table_erase(gc_hash_table_t* hash_table , void *ptr)
{
    if (!gc_hash_table_contain(hash_table , ptr)) return;

    u_int64_t hash = (u_int64_t)(ptr) % MODNUM;

    gc_hash_node_t *prev = &hash_table->bins[hash].base;
    gc_hash_node_t *cur = hash_table->bins[hash].base.next;

    while (cur)
    {
        if (cur->ptr == ptr)
        {
            prev->next = cur->next;
            free(cur);
            break;
        }
        cur = cur->next;
    }
}

void verify_hash_table()
{
    gc_hash_table_t *hash_table = gc_hash_table_create();

    assert(!gc_hash_table_contain(hash_table , (void*) 12));
    assert(!gc_hash_table_contain(hash_table , (void*) 13));

    gc_hash_table_insert(hash_table , (void*) 12);
    gc_hash_table_insert(hash_table , (void*) 13);

    assert(gc_hash_table_contain(hash_table , (void*) 12));
    assert(gc_hash_table_contain(hash_table , (void*) 13));

    gc_hash_table_erase(hash_table , (void*) 12);
    assert(!gc_hash_table_contain(hash_table , (void*) 12));
    assert(gc_hash_table_contain(hash_table , (void*) 13));

    gc_hash_table_destroy(hash_table);
}

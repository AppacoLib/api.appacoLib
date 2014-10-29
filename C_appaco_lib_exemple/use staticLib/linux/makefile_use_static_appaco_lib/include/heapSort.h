#ifndef HEAPSORT_H
#define HEAPSORT_H




#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


typedef struct
{
    int val;
    int data;
    char *str;
} Heap_Item;

typedef struct
{
    int size;
    int capacity;
    Heap_Item ** buffer;
} Heap_Array;


Heap_Array * Heap_initialize (int init_cap);
void Heap_free_space(Heap_Array * my_heap);
void Heap_add_item(Heap_Array * my_heap,Heap_Item *newItem, int (*compare)(Heap_Item *item_1,Heap_Item *item_2));
void Heap_delete_top(Heap_Array * my_heap, int (*compare)(Heap_Item *, Heap_Item *));

Heap_Item * Heap_get_top(Heap_Array * my_heap);

unsigned int Heap_get_size(Heap_Array * my_heap);

Heap_Item* Heap_newItem(int val, int data, char *str);

#endif // HEAPSORT_H

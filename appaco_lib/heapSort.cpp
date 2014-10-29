#include "heapSort.h"


// allocate memory for our heap
Heap_Array * Heap_initialize(int init_cap)
{
    Heap_Array * my_heap=(Heap_Array*)malloc(sizeof(Heap_Array));

    my_heap->size=0;
    my_heap->capacity=1;

    // increase the capacity of our table
    while(my_heap->capacity < init_cap)
        my_heap->capacity*=2;

    my_heap->buffer=(Heap_Item**) malloc(my_heap->capacity*sizeof(Heap_Item*));

    return my_heap;
}

// free the space memory occupied by our heap
void Heap_free_space(Heap_Array * my_heap)
{
    if(my_heap)
    {
        my_heap->size=0;
        my_heap->capacity=0;

        if(my_heap->buffer)
        {
            int i=0;
            int size_buffer = Heap_get_size(my_heap);

            for(;i<size_buffer;i++)
            {
                free(my_heap->buffer[i]);
            }

            free(my_heap->buffer);
        }

        free(my_heap);
    }
}


static inline void Heap_swap(Heap_Array * my_heap,int idx1,int idx2)
{
    Heap_Item *tmp= my_heap->buffer[idx1];
    my_heap->buffer[idx1]  = my_heap->buffer[idx2];
    my_heap->buffer[idx2]  = tmp;
}

static inline int Heap_left_child(int i)
{
    return (i+1)*2-1;
}

static inline int Heap_right_child(int i )
{
    return (i+1)*2;
}

static inline int Heap_parent(int i )
{
    return (i+1)/2-1;
}

// insert a new element in the heap
void Heap_add_item(Heap_Array * my_heap,Heap_Item *newItem, int (*compare)(Heap_Item *item_1,Heap_Item *item_2))
{
    int next_pos;
    int pos;

    // the table is full, so we must allocate a new space
    if(my_heap->size==my_heap->capacity)
    {
        if(my_heap->capacity==0)
            my_heap->capacity=1;

        while(my_heap->capacity <= my_heap->size)
            my_heap->capacity*=2;

        my_heap->buffer=(Heap_Item **)realloc(my_heap->buffer,my_heap->capacity*sizeof(Heap_Item*));
    }

    // insert the new element
    pos=my_heap->size;
    my_heap->size++;
    my_heap->buffer[pos]=newItem;


    // re-equilibrate the tree
    while(pos!=0)
    {
        next_pos=Heap_parent(pos);

         if(compare(my_heap->buffer[next_pos],my_heap->buffer[pos])>0)
             break;

        Heap_swap(my_heap,pos,next_pos);
        pos=next_pos;
    }
}

// remove the element that is at the top
void Heap_delete_top(Heap_Array * my_heap,int (*compare)(Heap_Item *item_1,Heap_Item *item_2))
{
    int pos;
    int next_pos;
    int nnext_pos;

    //
    if(my_heap->size<=1)
    {
        my_heap->size=0;
        return ;
    }


    my_heap->size--;

    my_heap->buffer[0]= my_heap->buffer[my_heap->size];

    // re-equilibrate the tree
    pos=0;
    for(;;)
    {
        next_pos=Heap_left_child(pos);

        if( (next_pos < my_heap->size) && ( compare( my_heap->buffer[next_pos], my_heap->buffer[pos]) > 0) )
        {
            nnext_pos=Heap_right_child(pos);

            if(nnext_pos>=my_heap->size)
            {
                Heap_swap(my_heap,pos,next_pos);
                break;
            }

            if(compare(my_heap->buffer[nnext_pos] , my_heap->buffer[next_pos])>0)
            {
                Heap_swap(my_heap,pos,nnext_pos);
                next_pos=nnext_pos;
            }
            else
                Heap_swap(my_heap,pos,next_pos);
        }
        else
        {
            next_pos=Heap_right_child(pos);

            if((next_pos < my_heap->size) && ( compare( my_heap->buffer[next_pos] , my_heap->buffer[pos])>0)  )
            {
                Heap_swap(my_heap,pos,next_pos);
            }
            else
                break;
        }

        pos=next_pos;
    }
}

// return the first element (that is at the top)
Heap_Item * Heap_get_top(Heap_Array * my_heap)
{
    if(my_heap->buffer)
        return my_heap->buffer[0];
    return 0;
}

// return the size of the heap
unsigned int Heap_get_size(Heap_Array * my_heap)
{

    if(my_heap->buffer)
        return my_heap->size;
    return 0;
}

Heap_Item* Heap_newItem(int val, int data, char *str)
{
    Heap_Item* item=(Heap_Item*)malloc(sizeof(Heap_Item));

    item->val=val;
    item->data=data;
    item->str=str; // we need just a pointer to our string

    return item;
}

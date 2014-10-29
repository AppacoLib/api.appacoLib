
#include "my_List_c.h"


static inline ListString* augmentCapacity(ListString* list);
static inline ListInt* augmentCapacity_List_Int(ListInt* list);


ListString* create_ListString()
{
    ListString* list  = (ListString *) malloc(sizeof(ListString));

    list->buffer = (char**)malloc(CAPACITY_INIT*sizeof(char*));
    list->capacity = CAPACITY_INIT;
    list->size = 0;

    return list;
}

ListString* delete_ListString(ListString* list)
{
    list = clear_ListString(list);

    free(list);

    return NULL;
}

ListString* clear_ListString(ListString* list)
{
    if(list == NULL) return NULL;

    char** tmp = list->buffer;

    free(tmp);

    list->buffer = NULL;
    list->capacity = 0;
    list->size = 0;

    return list;
}

static inline ListString* augmentCapacity(ListString* list)
{
    list->capacity += CAPACITY_INIT;

    ///  it doesn't work !!!!????
    /// list->buffer = (char**)realloc(list->buffer,list->capacity);

    //  this to replace realloc
    char** tmp= list->buffer;
    list->buffer = (char**)malloc((list->capacity)*sizeof(char*));
    int i= 0;
    for(i = 0; i<list->size; i++)
    {
        list->buffer[i]=tmp[i];
    }
    free(tmp);

    return list;
}

ListString* appendIn_ListString(ListString* list, char *string)
{
    /**
    if(list == NULL)
    {
        list = create_ListString();
    }
    */

    if(list->size == list->capacity)
    {
        list = augmentCapacity(list);
    }

    list->buffer[list->size] = (char *) malloc( (strlen(string)+1)*sizeof(char));
    strcpy(list->buffer[list->size],string);
    list->size++;


    return list;
}

ListString* appendListIn_ListString(ListString* list, ListString* listIN)
{
    int i =0;
    int size = Length_ListString(listIN);

    for(i=0; i<size;i++)
    {
        list = appendIn_ListString(list,getString_i_ListString(listIN,i));
    }

    return list;
}

void Print_ListString(ListString* list)
{
    if(list == NULL) return;

    int i = 0;

    int listSize = Length_ListString(list);

    printf("\n\n results : \n");

    for(i =0; i<listSize; i++)
    {
        printf("\n  %d ) %s",i,list->buffer[i]);
    }

    printf("\n\n");
}

void json_Print_ListString(ListString* list)
{
    if(list == NULL) return;

    int i = 0;

    int listSize = Length_ListString(list) -1;

    /// printf("Content-Type: text/html;\n\n");
    printf("Content-Type: application/json; charset=utf-8\n\n");

    printf("[");

    for(i =0; i<listSize; i++)
    {
        printf("\"%s\",",list->buffer[i]);
    }
    printf("\"%s\"]",list->buffer[i]); // the last one
}

int isEmpty_ListString(ListString* list)
{
    if (list == NULL ) return 1;

    if(list->size == 0)
    {
        return 1;
    }

    return 0;
}

char* getString_First_ListString(ListString* list)
{
    if(list == NULL || list->size == 0) return NULL;

    return list->buffer[0];
}

char* getString_End_ListString(ListString* list)
{
    if(list == NULL || list->size == 0) return NULL;

    return list->buffer[list->size - 1];
}

char* getString_i_ListString(ListString* list, int index)
{
    if(list == NULL || list->size == 0)return NULL;

    return list->buffer[index];
}

int Length_ListString(ListString* list)
{
    if(list == NULL) return 0;
    return list->size;
}


void swap_withPosition_ListString(ListString *list, int element_1, int element_2)
{
    if(list == NULL || list->size == 0) return;

    char* tmp = list->buffer[element_1];

    list->buffer[element_1] = list->buffer[element_2];

    list->buffer[element_2] = tmp;

}


/// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
/// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


ListInt* create_List_Int()
{
    ListInt* list  = (ListInt *) malloc(sizeof(ListInt));

    list->buffer = (int*)malloc(CAPACITY_INIT*sizeof(int));
    list->capacity = CAPACITY_INIT;
    list->size = 0;

    return list;
}

ListInt* delete_List_Int(ListInt* list)
{
    list = clear_List_Int(list);

    free(list);

    return NULL;
}

ListInt* clear_List_Int(ListInt* list)
{
    int* tmp = list->buffer;

    free(tmp);

    list->buffer = NULL;
    list->capacity = 0;
    list->size = 0;

    return list;
}

inline ListInt* augmentCapacity_List_Int(ListInt* list)
{
    list->capacity += CAPACITY_INIT;


    /// list->buffer = (char**)realloc(list->buffer,list->capacity);

    //  this to replace realloc
    int* tmp= list->buffer;
    list->buffer = (int*)malloc((list->capacity)*sizeof(int));
    int i= 0;
    for(i = 0; i<list->size; i++)
    {
        list->buffer[i]=tmp[i];
    }
    free(tmp);

    return list;
}

ListInt* appendIn_List_Int(ListInt *list, int val)
{
    /**
    if(list == NULL)
    {
        list = creat_List_Int();
    }
    */

    if(list->size == list->capacity)
    {
        list = augmentCapacity_List_Int(list);
    }

    list->buffer[list->size] = val;
    list->size++;


    return list;
}

void Print_List_Int(ListInt* list)
{
    if(list == NULL) return;

    int i = 0;

    for(i =0; i<list->size; i++)
    {
        printf("\n  str = %d",list->buffer[i]);
    }
}

int isEmpty_List_Int(ListInt* list)
{
    if (list == NULL ) return 1;

    if(list->size == 0)
    {
        return 1;
    }

    return 0;
}

int getVal_First_List_Int(ListInt *list)
{
    if(list == NULL || list->size == 0) return -1;

    return list->buffer[0];
}

int getVal_End_List_Int(ListInt *list)
{
    if(list == NULL || list->size == 0) return -1;

    return list->buffer[list->size - 1];
}

int getVal_i_List_Int(ListInt *list, int indice)
{
    if(list == NULL || list->size == 0)return -1;


    return list->buffer[indice];
}

int Length_List_Int(ListInt *list)
{
    if(list==NULL) return 0;
    return list->size;
}


void swap_withPosition_List_Int(ListInt *list, int element_1, int element_2)
{
    if(list == NULL || list->size == 0) return;

    int tmp = list->buffer[element_1];

    list->buffer[element_1] = list->buffer[element_2];

    list->buffer[element_2] = tmp;

}

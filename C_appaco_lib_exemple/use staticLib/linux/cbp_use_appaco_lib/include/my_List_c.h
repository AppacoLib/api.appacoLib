#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "define.h"


#define CAPACITY_INIT 1000

typedef struct ListString ListString;
struct ListString
{
    char** buffer;
    int capacity;
    int size;
};

ListString *create_ListString();
ListString *delete_ListString(ListString* list);

ListString *appendIn_ListString(ListString *list, char* string);
ListString *appendListIn_ListString(ListString *list, ListString *listIN);
void Print_ListString(ListString *list);
void json_Print_ListString(ListString* list);
ListString *clear_ListString(ListString *list);
int isEmpty_ListString(ListString *list);
char* getString_First_ListString(ListString* list);
char* getString_End_ListString(ListString* list);
char* getString_i_ListString(ListString* list, int index);
int Length_ListString(ListString *list);
void swap_withPosition_ListString(ListString *list, int element_1, int element_2);


/// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
/// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// if I'm in c++, I do this code once with the template method

typedef struct ListInt ListInt;
struct ListInt
{
    int* buffer;
    int capacity;
    int size;

};

ListInt *create_List_Int();
ListInt* delete_List_Int(ListInt* list);

ListInt *appendIn_List_Int(ListInt *list, int val);
void Print_List_Int(ListInt *list);
ListInt *clear_List_Int(ListInt *list);
int isEmpty_List_Int(ListInt *list);
int getVal_First_List_Int(ListInt* list);
int getVal_End_List_Int(ListInt* list);
int getVal_i_List_Int(ListInt* list, int indice);
int Length_List_Int(ListInt *list);
void swap_withPosition_List_Int(ListInt *list, int element_1, int element_2);


#endif // LIST_H

#ifndef HASH_FONCTION_H_INCLUDED
#define HASH_FONCTION_H_INCLUDED

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct HashTable
{
    char** buffer;
    int size_hashTable;
    int nb_element;
    int percentage_loadFator;
}HashTable;

HashTable* initialisation_HF(int nbElement,int max_size_word);

inline unsigned long hash(char *string, int sizeString);

void hash_addItem(HashTable *my_hashTable,char *string);
int hash_findItem(HashTable* my_hashTable,char *string);

void hash_free_space(HashTable *my_hashTable);

// --------------------------------------------------------------------------
typedef struct HashTable_int
{
    int* buffer;
    int size_hashTable;
    int nb_element;
    int percentage_loadFator;
}HashTable_int;

HashTable_int* initialisation_HF_int(int nbElement);

void hash_addItem_int(HashTable_int *my_hashTable,int val);
int hash_findItem_int(HashTable_int* my_hashTable,int val);

void hash_free_space_int(HashTable_int *my_hashTable);


/// /////////////////////////////////////////////////////////////////////

#define mod_P_multiply(x,y) ((unsigned long)((((unsigned long long)x)*(y))%P))
#define mod_P_multiply_add(x,y,z) ((unsigned long)((((unsigned long long)x)*(y)+(z))%P))
#define mod_P_multiply_add_add(x,y,z,w) ((unsigned long)((((unsigned long long)x)*(y)+(z)+(w))%P))
#define mod_P_add_add_add(x,y,z) ((unsigned long)((((unsigned long long)x)+(y)+(z))%P))

/// ///////////////////////////////////////////////////////////////////////




#endif // HASH_FONCTION_H_INCLUDED

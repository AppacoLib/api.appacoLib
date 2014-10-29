#ifndef DEFINE_H
#define DEFINE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "my_List_c.h"


#ifndef __cplusplus
#define bool int
#define true 1
#define false 0
#endif


#define fatal_error(msg) \
   { printf (msg); \
     exit(1); \
   }

#define NUM_PAR  8                     // for internal use only
#define NUM_OCTETS_RESERVED_BY_NODE 4  // free space added in the trie for each node (top K)

/// #define CHAR_SEPARATOR '#'

#define MAX_SIZE_WORD 1000

// search types, how to search query word in the trie
#define SEARCH_WHOLE_WORD_EACH_TIME 0 // search query word each time from root
#define SEARCH_SUFFIX_BY_PREFIXE    1 // search the second query word from the node of the first query word, if the first is total prefix of the second, So it comes to search just the suffix added
#define SEARCH_DELET_SUFFIX_ALL     2 // we start from the node where the two query word share a prefix, in the 2nd query we delete or add character

// display types, how to display the result
#define PRINT_SIMPLE_DIRECTLY           0 // direct display from the tri (from nodes).
#define PRINT_WORD_LIST                 1 // simple display, words are in the list
#define PRINT_WORD_LIST_TOP_K           2 // words are ordered according to the top k, and they are displayed all at once
#define PRINT_WORD_LIST_TOP_K_byGroup   3 // words are ordered according to the top k, and they are displayed group by group (withe "next" option)


typedef unsigned char uchar;
typedef uint32_t u_int32_t;
typedef unsigned long ulong;


typedef struct TrieInformation
{
    uchar *text;
    int longest_line;
    int limit_leaf;
    uchar * trie;
    int first_node;

    long long Number_Bytes_Occupied_by_Trie;

} TrieInformation;


/*
typedef struct StringInfo
{
    uchar* stringAddress;
    uchar* stringTopK_Address;//a variable that will contain the memory area that contains top_K information  of our string.
    uint32_t stringTopK_Value;

}StringInfo;

*/


typedef struct Options
{
    int typeSearch;
    int typePrintResults;
    bool isExactSearchMethod; // exact or approximate search with one error.    

    char topK_CHAR_SEPARATOR;
    // words_SEPARATOR=/[\n\r\t\0]+/g, // sa peut etre un char, string, ou regexp
    char words_SEPARATOR; // you must be vry carful, in windows we use \r\n tu retourn to new line

    bool doSort; // sort the dictionary or not, on disactive cette options si et seulement si le dic est ordonnie, sinon en risque de ne pas avoire des boone resultat, bon la construction sera dans un desourdre totale

    // _________________________________________________________________________

    int TOPK_SIZE_GROUP;  // the number of words that will be displayed at the same time according to (top-k) in the same group

    int delay; // The delay in milliseconds between when a keystroke occurs and when a search is performed.

    int minLength; // The minimum number of characters a user must type before a search is performed.


} Options;

/*
typedef struct exact_and_1err_Solution_nodes exact_and_1err_Solution_nodes;
struct exact_and_1err_Solution_nodes
{
    ListInt* list_solution_1error;
    int exact_solution;      // -1 : mean we have no exact solution

};
*/

typedef struct exact_and_1err_Solution_nodes
{
    struct ListInt* list_solution_1error;
    int exact_solution;      // -1 : mean we have no exact solution

}exact_and_1err_Solution_nodes;


#endif // DEFINE_H

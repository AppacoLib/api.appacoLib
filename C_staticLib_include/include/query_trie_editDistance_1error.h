#ifndef QUERY_TRIE_EDITDISTANCE_1ERROR_H
#define QUERY_TRIE_EDITDISTANCE_1ERROR_H


#include "query_trie.h"
#include "my_List_c.h"
#include "hash_fonction.h"

void printResult_queryWord_1error(TrieInformation *trie_info, ListInt* listSolution);

void searchQueryWord_1error_PrintResult(uchar *pattern,TrieInformation *trie_info);

exact_and_1err_Solution_nodes *searchQueryWord_1error(uchar *pattern, TrieInformation *trie_info);

exact_and_1err_Solution_nodes* searchSuffixbyPrefix_1error(uchar *pattern, int lengPrefixCommon, TrieInformation *trie_info);

exact_and_1err_Solution_nodes* search_Prefix_by_Suffix_Delete_1error(uchar *pattern, int lengPrefixCommon, TrieInformation *trie_info);

#endif // QUERY_TRIE_EDITDISTANCE_1ERROR_H

#ifndef GET_LISTWORD_H
#define GET_LISTWORD_H


#include "my_List_c.h"
#include "integer_coding.h"
#include "utf8.h"
#include "heapSort.h"


ListString *wordList(TrieInformation *trie_info, int Last_pointed_node);

ListString *wordList_TopK(TrieInformation *trie_info, int Last_pointed_node);

ListString *wordList_TopK_byGroup(TrieInformation *trie_info, int Last_pointed_node, int topK_Element,bool isFirstCall);

/// ____________________________________________________________________________________________________


ListString *wordListe_exact_1err(TrieInformation *trie_info, exact_and_1err_Solution_nodes* listSolutionNodes);

ListString* wordList_exact_1err_TopK(TrieInformation *trie_info, exact_and_1err_Solution_nodes* listSolutionNodes);

ListString* wordList_exact_1err_TopK_byGroup(TrieInformation *trie_info, exact_and_1err_Solution_nodes* listSolutionNodes, int topK_Element,bool isFirstCall);

/// ____________________________________________________________________________________________________

void topK_updating_AllWayNodes_bySearching(uchar *pattern, TrieInformation *trie_info);

#endif // GET_LISTWORD_H

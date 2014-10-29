#ifndef QUERY_TRIE_H
#define QUERY_TRIE_H



#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdint.h>

#include "define.h"
#include "integer_coding.h"
#include "utf8.h"

void searchQueryWord_PrintResult(uchar *pattern,TrieInformation *trie_info);

int searchQueryWord(uchar *pattern,TrieInformation *trie_info);

int searchSuffixbyPrefix(uchar *pattern,int lengPrefixCommon,TrieInformation *trie_info);

int search_Prefix_by_Suffix_Delete(uchar *pattern,int lengPrefixCommon,TrieInformation *trie_info);

void printWordsHavePrefix_allInformation(TrieInformation *trie_info,int Last_pointed_node);

void printWordsHavePrefix(TrieInformation *trie_info,int Last_pointed_node);

#endif // QUERY_TRIE_H

#ifndef TRIE_CODING_H
#define TRIE_CODING_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdint.h>

#include "integer_coding.h"
#include "define.h"
#include "utf8.h"


TrieInformation *main_trie_coding(const char *nameFile_dic,Options *my_option);

void printingWholeTrie(uchar * text,int longest_line,uchar * trie,int first_node,int limit_leaf, int num_octet_by_node);

void save_trie(const char * nameFile_dic,uchar * trie,int size,int nb_trans_max,int limit_leaf,int pos_begin, int longest_line);


uint32_t utf8_char_len_pgpc(uchar * str1, uchar* str2);


#endif // TRIE_CODING_H

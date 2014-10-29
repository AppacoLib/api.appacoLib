#ifndef APPROXIMATE_AUTOCOMPLETE_H
#define APPROXIMATE_AUTOCOMPLETE_H

#include "define.h"
#include "trie_coding.h"
#include "query_trie.h"
#include "query_trie_editDistance_1error.h"
#include "get_listWord.h"

Options* new_options();
void print_options(Options *my_option);

void initialize_AutoComplete(const char *source,Options *my_option_in);

ListString* get_results(char* queryWord);

ListString *get_next_result();


#endif // APPROXIMATE_AUTOCOMPLETE_H

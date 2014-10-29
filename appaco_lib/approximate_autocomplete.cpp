#include "approximate_autocomplete.h"


static bool isFirstCall;
static bool isExistNextResult;
static TrieInformation* trie_info;
static Options *my_option;

static int lengPrefixCommon=0; // the length of the common prefix between the current and previous word
static int pos_node_stop_searsh=0; // solution node (in case of exact search)

static ListString* choices = NULL; // suggestion list will be displayed (solution)
static exact_and_1err_Solution_nodes* list_solution_1error=NULL; // contains nodes solution for approximate search with one error

static char lastString[MAX_SIZE_WORD]={0}; // the previous string, to calculate the common prefix with the current one



// default configuration for research (exact or approximate, how to search, how to display the result)
Options* new_options()
{
    Options *my_option = (Options*)malloc(sizeof(Options));

    my_option->isExactSearchMethod = false; // exact search

    my_option->typeSearch = SEARCH_WHOLE_WORD_EACH_TIME; // search the whole query Word each times. ");

    my_option->typePrintResults = PRINT_WORD_LIST; // 1) words are in an unordered list ");

    my_option->topK_CHAR_SEPARATOR ='#';

    my_option->doSort = true;

    my_option->TOPK_SIZE_GROUP = 8;

    my_option->delay = 300;

    my_option->minLength = 1;

    return my_option;
}


// display the configuration
void print_options(Options *my_option)
{
    if(my_option->isExactSearchMethod) printf("\n\n the type of search : exact search ");
    else printf("\n\n type of search : approximate search (1 error )");


    if(my_option->typeSearch == SEARCH_WHOLE_WORD_EACH_TIME)
    {
        printf("\n\n how to search : the whole query Word each times");
    }
    else
        if(my_option->typeSearch == SEARCH_SUFFIX_BY_PREFIXE)
        {
            printf("\n\n how to search : suffix of queryWord when previous one is a TOTAL PREFIX.");
        }
        else
        {
            printf("\n\n how to search : suffix of queryWord when we have the SAME PREFIX with the previous one.");
        }


    if(my_option->typePrintResults == PRINT_SIMPLE_DIRECTLY)
    {
        printf("\n\n output method : print results directly from the Trie");
    }
    else
        if(my_option->typePrintResults == PRINT_WORD_LIST)
        {
            printf("\n\n output method : words are in an unordered list");
        }
        else
            if(my_option->typePrintResults == PRINT_WORD_LIST_TOP_K_byGroup)
            {
                printf("\n\n output method : words by group, ordered according to the Top k (their score).");
            }
            else // PRINT_WORD_LIST_TOP_K
            {
                printf("\n\n output method : ALL words are ordered according to the Top k (their score).");
            }

    printf("\n\n ===========================================================================\n ");


    return;
}

// build the Trie from an input file that contains the dictionary
void initialize_AutoComplete(const char *source,Options *my_option_in)
{
    my_option = my_option_in;

    trie_info = main_trie_coding (source,my_option);
}



// look up the  query word and display the result
ListString* get_results(char* queryWord)
{
    // Empty the list of "choices", to use it in the next time
    choices=clear_ListString(choices);


    if(queryWord==NULL)
    {
        choices = create_ListString(); // tableau vide, pas de solution
        return choices;
    }

    /// printf(" \n\n query word :  %s",queryWord);

    lengPrefixCommon = utf8_char_len_pgpc((uchar*)lastString,(uchar*)queryWord);

    // save the current queryWord
    strcpy(lastString,queryWord);


    // 1 : research, look up for the queryWord

    if(my_option->isExactSearchMethod) // exact search
    {
        if(my_option->typeSearch==SEARCH_WHOLE_WORD_EACH_TIME) // search the whole word each times
        {
            pos_node_stop_searsh = searchQueryWord((uchar*)queryWord,trie_info);
        }
        else
            if(my_option->typeSearch==SEARCH_DELET_SUFFIX_ALL) // search, supprme suffixe, add suffixe, (all combinison), there's prefixe commun beteween the previeus queryWord and the present queryWord
            {
                pos_node_stop_searsh = search_Prefix_by_Suffix_Delete((uchar*)queryWord,lengPrefixCommon,trie_info);
            }
            else  // search the word, but we begin in the end of the befor word if it's a suffixe, we search the just the suffixe
            {
                pos_node_stop_searsh = searchSuffixbyPrefix((uchar*)queryWord,lengPrefixCommon,trie_info);
            }
    }
    else // approximate search with one error
    {
        if(my_option->typeSearch==SEARCH_WHOLE_WORD_EACH_TIME)
        {
            list_solution_1error = searchQueryWord_1error((uchar*)queryWord,trie_info);
        }
        else
            if(my_option->typeSearch==SEARCH_DELET_SUFFIX_ALL)
            {
                list_solution_1error = search_Prefix_by_Suffix_Delete_1error((uchar*)queryWord, lengPrefixCommon,trie_info);
            }
            else
            {
                list_solution_1error = searchSuffixbyPrefix_1error((uchar*)queryWord, lengPrefixCommon,trie_info);
            }
    }

    /// printf("\n\n.. end search .. pos_node_stop_searsh = %d\n now display the result : ",pos_node_stop_searsh); // ça si on est dans la recherche exact, mais si on est dans la recherche approuchée donc on a toute une liste
    /// getchar();

    // get a list of words to display
    if(my_option->isExactSearchMethod) //results for exact search
    {
        if(pos_node_stop_searsh!=-1)
        {
            if(my_option->typePrintResults==PRINT_WORD_LIST)
            {
                choices = wordList(trie_info,pos_node_stop_searsh); // get a list of words to display
            }
            else if(my_option->typePrintResults==PRINT_WORD_LIST_TOP_K_byGroup)
            {
                // re-initialize those var to se theme in wordList and get_next_result functions
                isFirstCall = true;
                isExistNextResult = true;

                choices =  wordList_TopK_byGroup(trie_info, pos_node_stop_searsh, my_option->TOPK_SIZE_GROUP, isFirstCall);

                if(choices->size<my_option->TOPK_SIZE_GROUP) // this means that the words number in the list is unless of the number topk_element, so wholes nodes were treated
                    isExistNextResult = false;

                isFirstCall=false;
            }
            else // "if(typePrintResults==PRINT_WORD_LIST_TOP_K)", print all results at once, words are ordered by topK
            {
                choices = wordList_TopK(trie_info,pos_node_stop_searsh);
            }
        }
        else
        {
            choices= create_ListString(); // an empty list for no solution
        }
    }
    else // results for approximate search
    {
        if(list_solution_1error->exact_solution!=-1 || list_solution_1error->list_solution_1error->size!=0)
        {
            if(my_option->typePrintResults==PRINT_WORD_LIST)
            {
                choices = wordListe_exact_1err(trie_info, list_solution_1error);
            }
            else if(my_option->typePrintResults==PRINT_WORD_LIST_TOP_K_byGroup)
            {
                isFirstCall = true;
                isExistNextResult = true;


                choices = wordList_exact_1err_TopK_byGroup(trie_info,list_solution_1error,my_option->TOPK_SIZE_GROUP, isFirstCall);


                if(choices->size<my_option->TOPK_SIZE_GROUP)
                    isExistNextResult = false;

                isFirstCall=false;
            }
            else
            {
                choices = wordList_exact_1err_TopK(trie_info,list_solution_1error);
            }
        }
        else
        {
            choices= create_ListString(); // no solution
        }
    }

    return choices;
}


ListString*  get_next_result()
{

    if(isExistNextResult && my_option->typePrintResults==PRINT_WORD_LIST_TOP_K_byGroup)
    {
        // Empty the list of "choices", to use it in the next time
        choices=clear_ListString(choices);

        // display the result
        if(my_option->isExactSearchMethod) // display results for exact search
        {
            choices =  wordList_TopK_byGroup(trie_info, pos_node_stop_searsh, my_option->TOPK_SIZE_GROUP, isFirstCall);

            return choices;

            if(choices->size<my_option->TOPK_SIZE_GROUP) // this means that the words number in the list is unless of the number topk_element, so wholes nodes were treated
                isExistNextResult = false;
        }
        else //  display results for approximate search
        {
            choices = wordList_exact_1err_TopK_byGroup(trie_info,list_solution_1error,my_option->TOPK_SIZE_GROUP, isFirstCall);

            return choices;

            if(choices->size<my_option->TOPK_SIZE_GROUP) // this means that the words number in the list is unless of the number topk_element, so wholes nodes were treated
                isExistNextResult = false;
        }
    }

    return create_ListString(); // return an empty list :)

}

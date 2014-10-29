
#include "get_listWord.h"
#include "query_trie.h"
#include "hash_fonction.h"

#define INTITIAL_CAPACITY 1000
#define MAX_SIZE_WORD 1000

static Heap_Array *my_heap = NULL; // to use it in wordList_TopK_byGroup, cause we call this function sevreal time

// use those var in wordList_exact_1err_TopK_byGroup
static bool still_exact_solution = true; // extract the exat solution and then we go to the approximate solution
static Heap_Array *my_heap_err=NULL;

HashTable *list_words_hash_err=NULL; // on fait sa, pour que meme es resultat de next ne se deplique pas ;) cela vaut dir que il ne sont pas déja sorti dans les groupe precedent ;)


// compare 2 node by there score
// if there scor is equal so we compare theme by there str
int compare(Heap_Item *a, Heap_Item *b)
{
    if( a->val > b->val) return 1; // decroissant
    else
        if( a->val < b->val) return -1; // decroissant
        else
            return strcmp(b->str,a->str); // on change le b par a, car on vaudri un ordre alphabitique donc croissant
}

// _____________________________________________________________________________//
// ------------------------------ exact solution -------------------------------//


// get solution words (going through the subtree at the end of the transition)
//  it's exactly like search function but we don't make comparison
ListString *wordList(TrieInformation *trie_info, int Last_pointed_node)
{
    uchar *text = trie_info->text;
    int longest_line = trie_info->longest_line;
    uchar *trie = trie_info->trie;
    int limit_leaf = trie_info->limit_leaf;

    ListString* list_words = create_ListString();

    int * file_last_nodes;
    int file_last_nodes_counter =0;

    int * file_nodes_temp;

    int k2;

    int nb_sequence;
    int nb_trans;
    int pointer_sequence;
    int length_transition;

    int pos_code;


    file_last_nodes = (int  *) malloc(longest_line*sizeof(int));
    if (file_last_nodes == NULL)
        fatal_error("Cannot allocate memory for file_last_nodes\n");

    file_nodes_temp = (int  *) malloc(longest_line*sizeof(int));


    file_last_nodes[0] = Last_pointed_node;
    file_last_nodes_counter = 0;

    while (file_last_nodes_counter>=0)
    {
        pos_code=file_last_nodes[file_last_nodes_counter];

        if (pos_code<limit_leaf)
        {
            nb_sequence=decode_int(trie+pos_code+NUM_OCTETS_RESERVED_BY_NODE);

            list_words=appendIn_ListString(list_words, (char*)(nb_sequence+text) ); // save the word of this node

            file_last_nodes_counter--;
        }
        else
        {
            pos_code += NUM_OCTETS_RESERVED_BY_NODE;

            nb_sequence = length_decode_int(&pos_code,trie+pos_code);

            nb_trans = length_decode_int(&pos_code,trie+pos_code);


            for (k2=0; k2<nb_trans; k2++)
            {
                pos_code += u8_seqlen((char*)(trie+pos_code));
            }

            file_last_nodes_counter--;

            // we add the nodes in the inverse way, for that we put them 'file_nodes_temp' and then we extract them from the end aand we add them to 'file_last_nodes', we do this to re-order the result in alphabitic order
            for (k2=0;k2<nb_trans;k2++)
            {
                pointer_sequence = length_decode_int(&pos_code,trie+pos_code);
                length_transition = length_decode_int(&pos_code,trie+pos_code);
                Last_pointed_node = length_decode_int(&pos_code,trie+pos_code);

                file_nodes_temp[k2] = Last_pointed_node;
            }

            for (k2=nb_trans-1;k2>=0;k2--)
            {
                file_last_nodes[++file_last_nodes_counter] = file_nodes_temp[k2];
            }

        }
    }

    return list_words;
}

// get solution words (with their information), words are ordered by their score topK
ListString *wordList_TopK(TrieInformation *trie_info, int Last_pointed_node)
{
    uchar *text = trie_info->text;
    int longest_line = trie_info->longest_line;
    uchar *trie = trie_info->trie;
    int limit_leaf = trie_info->limit_leaf;

    ListString* list_words = create_ListString();


    int k2;

    int nb_sequence;
    int nb_trans;
    int pointer_sequence;
    int length_transition;

    int pos_code;

    Heap_Array *my_heap = Heap_initialize(longest_line); // create the heap

    // add the first node to the heap
    char *str_node = (char*)((decode_int(trie+Last_pointed_node+NUM_OCTETS_RESERVED_BY_NODE) )+text);
    Heap_add_item( my_heap,Heap_newItem(decode_int(trie+Last_pointed_node),Last_pointed_node,str_node),compare);

    while (Heap_get_size(my_heap)>0)
    {
        pos_code = Heap_get_top(my_heap)->data;

        if (pos_code<limit_leaf)
        {
            nb_sequence=decode_int(trie+pos_code+NUM_OCTETS_RESERVED_BY_NODE);
            list_words=appendIn_ListString(list_words, (char*)(nb_sequence+text) ); // save the word of this node
            Heap_delete_top(my_heap,compare);
        }
        else
        {
            pos_code += NUM_OCTETS_RESERVED_BY_NODE;

            nb_sequence = length_decode_int(&pos_code,trie+pos_code);

            nb_trans = length_decode_int(&pos_code,trie+pos_code);

            for (k2=0; k2<nb_trans; k2++)
            {
                pos_code += u8_seqlen((char*)(trie+pos_code));
            }

            Heap_delete_top(my_heap,compare);

            for (k2=0;k2<nb_trans;k2++)
            {
                pointer_sequence = length_decode_int(&pos_code,trie+pos_code);
                length_transition = length_decode_int(&pos_code,trie+pos_code);
                Last_pointed_node = length_decode_int(&pos_code,trie+pos_code);

                str_node = (char*)((decode_int(trie+Last_pointed_node+NUM_OCTETS_RESERVED_BY_NODE) )+text);
                Heap_add_item( my_heap,Heap_newItem(decode_int(trie+Last_pointed_node),Last_pointed_node,str_node),compare);
            }
        }
    }

    return list_words;
}

// get solution words (with their information), words are ordered by their score topK
// each time we return a group of words (size == topK_Element)
ListString *wordList_TopK_byGroup(TrieInformation *trie_info, int Last_pointed_node, int topK_Element,bool isFirstCall)
{
    uchar *text = trie_info->text;
    int longest_line = trie_info->longest_line;
    uchar *trie = trie_info->trie;
    int limit_leaf = trie_info->limit_leaf;

    char *str_node;

    if(isFirstCall) //initialize, and add the first node to the "prio_queue".
    {

        Heap_free_space(my_heap);
        my_heap = Heap_initialize(longest_line); // create the heap

        // add the first node to the heap
        str_node = (char*)((decode_int(trie+Last_pointed_node+NUM_OCTETS_RESERVED_BY_NODE) )+text);
        Heap_add_item( my_heap,Heap_newItem(decode_int(trie+Last_pointed_node),Last_pointed_node,str_node),compare);
    }

    ListString* list_words = create_ListString();

    int k2;

    int nb_sequence;
    int nb_trans;
    int pointer_sequence;
    int length_transition;

    int pos_code;


    int idx_TopK = 0; //to calculate the number of words to return in a single group

    while (Heap_get_size(my_heap)>0 && idx_TopK < topK_Element)
    {
        pos_code = Heap_get_top(my_heap)->data;

        if (pos_code<limit_leaf)
        {
            nb_sequence=decode_int(trie+pos_code+NUM_OCTETS_RESERVED_BY_NODE);
            list_words=appendIn_ListString(list_words, (char*)(nb_sequence+text) ); // save the word of this node

            Heap_delete_top(my_heap,compare);

            idx_TopK++;
        }
        else
        {
            pos_code += NUM_OCTETS_RESERVED_BY_NODE;

            nb_sequence = length_decode_int(&pos_code,trie+pos_code);

            nb_trans = length_decode_int(&pos_code,trie+pos_code);

            for (k2=0; k2<nb_trans; k2++)
            {
                pos_code += u8_seqlen((char*)(trie+pos_code));
            }

            Heap_delete_top(my_heap,compare);

            for (k2=0;k2<nb_trans;k2++)
            {
                pointer_sequence = length_decode_int(&pos_code,trie+pos_code);
                length_transition = length_decode_int(&pos_code,trie+pos_code);
                Last_pointed_node = length_decode_int(&pos_code,trie+pos_code);

                str_node = (char*)((decode_int(trie+Last_pointed_node+NUM_OCTETS_RESERVED_BY_NODE) )+text);
                Heap_add_item( my_heap,Heap_newItem(decode_int(trie+Last_pointed_node),Last_pointed_node,str_node),compare);
            }
        }
    }

    return list_words;
}


// _____________________________________________________________________________//
// ----------------------- exact && 1err solution ------------------------------//

// get all solution words  from a list of solution node
ListString *wordListe_exact_1err(TrieInformation *trie_info, exact_and_1err_Solution_nodes* listSolutionNodes)
{
    uchar *text = trie_info->text;
    int longest_line = trie_info->longest_line;
    uchar *trie = trie_info->trie;
    int limit_leaf = trie_info->limit_leaf;

    int * file_last_nodes;
    int file_last_nodes_counter =0;
    int *file_nodes_temp;
    int k2;

    int nb_sequence;
    int nb_trans;
    int pointer_sequence;
    int length_transition;
    int Last_pointed_node;

    int pos_code;
    int i=0;

    ListString* list_words = create_ListString();
    int sizeListSolutionNodes = Length_List_Int(listSolutionNodes->list_solution_1error);


    HashTable *list_words_hash= initialisation_HF(INTITIAL_CAPACITY,MAX_SIZE_WORD);


    file_last_nodes = (int  *) malloc(longest_line*sizeof(int));
    if (file_last_nodes == NULL)
        fatal_error("Cannot allocate memory for file_last_nodes\n");

    file_nodes_temp = (int  *) malloc(longest_line*sizeof(int));

    if(listSolutionNodes->exact_solution>=0) // dans la solution exact on a pas besoin de verifier si les mot exist ou non (bon je pense, mais je ne sais pas si après on peut avoir des solution approuchée qui sont déjà dans l'exact et donc on doit aussi inclure sa la table de hashage, bon il suffi just d'ajouter le node exact en primier dans la liste des err ensuit faire la boucle for)
    {
        list_words = wordList(trie_info, listSolutionNodes->exact_solution);
    }

    //  extraire les resulta de la recherrche approuchée
    for(i=0; i<sizeListSolutionNodes; i++)
    {
        file_last_nodes[0] = listSolutionNodes->list_solution_1error->buffer[i];
        file_last_nodes_counter = 0;

        while (file_last_nodes_counter>=0)
        {
            pos_code=file_last_nodes[file_last_nodes_counter];

            if (pos_code<limit_leaf)
            {
                nb_sequence=decode_int(trie+pos_code+NUM_OCTETS_RESERVED_BY_NODE);

                if(hash_findItem(list_words_hash, (char*)(nb_sequence+text) )==-1)
                {
                    list_words=appendIn_ListString(list_words, (char*)(nb_sequence+text) ); // save the word of this node
                    hash_addItem(list_words_hash, (char*)(nb_sequence+text) );
                }

                file_last_nodes_counter--;
            }
            else
            {
                pos_code += NUM_OCTETS_RESERVED_BY_NODE;

                nb_sequence = length_decode_int(&pos_code,trie+pos_code);

                nb_trans = length_decode_int(&pos_code,trie+pos_code);


                for (k2=0; k2<nb_trans; k2++)
                {
                    pos_code += u8_seqlen((char*)(trie+pos_code));
                }

                file_last_nodes_counter--;

                for (k2=0;k2<nb_trans;k2++)
                {
                    pointer_sequence = length_decode_int(&pos_code,trie+pos_code);
                    length_transition = length_decode_int(&pos_code,trie+pos_code);
                    Last_pointed_node = length_decode_int(&pos_code,trie+pos_code);

                    file_nodes_temp[k2] = Last_pointed_node;
                }

                for (k2=nb_trans-1;k2>=0;k2--)
                {
                    file_last_nodes[++file_last_nodes_counter] = file_nodes_temp[k2];
                }
            }
        }

    }

    hash_free_space(list_words_hash);
    return list_words;
}


// get all solution words (with their information),  from a list of solution node. words are ordered by their score topK
ListString* wordList_exact_1err_TopK(TrieInformation *trie_info, exact_and_1err_Solution_nodes* listSolutionNodes)
{
    uchar *text = trie_info->text;
    int longest_line = trie_info->longest_line;
    uchar *trie = trie_info->trie;
    int limit_leaf = trie_info->limit_leaf;
    int k2;

    int nb_sequence;
    int nb_trans;
    int pointer_sequence;
    int length_transition;

    int pos_code;

    ListString* list_words = create_ListString();

    Heap_Array *my_heap = Heap_initialize(longest_line); // create the heap

    char *str_node;

    int sizeListSolutionNodes = Length_List_Int(listSolutionNodes->list_solution_1error);

    HashTable *list_words_hash= initialisation_HF(INTITIAL_CAPACITY,MAX_SIZE_WORD);

    if(listSolutionNodes->exact_solution>=0) // dans la solution exact on a pas besoin de verifier si les mot exist ou non (bon je pense, mais je ne sais pas si après on peut avoir des solution approuchée qui sont déjà dans l'exact et donc on doit aussi inclure sa la table de hashage, bon il suffi just d'ajouter le node exact en primier dans la liste des err ensuit faire la boucle for)
    {
        list_words = wordList(trie_info, listSolutionNodes->exact_solution);
    }


    // add the first nodes in the stack
    int indice_listSolutionNodes = 0;
    int Last_pointed_node=0;
    for(indice_listSolutionNodes=0; indice_listSolutionNodes < sizeListSolutionNodes ; indice_listSolutionNodes++)
    {
        Last_pointed_node = getVal_i_List_Int( listSolutionNodes->list_solution_1error , indice_listSolutionNodes);

        str_node = (char*)((decode_int(trie+Last_pointed_node+NUM_OCTETS_RESERVED_BY_NODE) )+text);
        Heap_add_item( my_heap,Heap_newItem(decode_int(trie+Last_pointed_node),Last_pointed_node,str_node),compare);

    }

    while (Heap_get_size(my_heap)>0)
    {
        pos_code = Heap_get_top(my_heap)->data;

        if (pos_code<limit_leaf)
        {
            nb_sequence=decode_int(trie+pos_code+NUM_OCTETS_RESERVED_BY_NODE);

            if(hash_findItem(list_words_hash, (char*)(nb_sequence+text) )==-1)
            {
                list_words=appendIn_ListString(list_words, (char*)(nb_sequence+text) ); // save the word of this node
                hash_addItem(list_words_hash, (char*)(nb_sequence+text) );
            }

            Heap_delete_top(my_heap,compare);
        }
        else
        {
            pos_code += NUM_OCTETS_RESERVED_BY_NODE;

            nb_sequence = length_decode_int(&pos_code,trie+pos_code);

            nb_trans = length_decode_int(&pos_code,trie+pos_code);

            for (k2=0; k2<nb_trans; k2++)
            {
                pos_code += u8_seqlen((char*)(trie+pos_code));
            }

            Heap_delete_top(my_heap,compare);

            for (k2=0;k2<nb_trans;k2++)
            {
                pointer_sequence = length_decode_int(&pos_code,trie+pos_code);
                length_transition = length_decode_int(&pos_code,trie+pos_code);
                Last_pointed_node = length_decode_int(&pos_code,trie+pos_code);

                str_node = (char*)((decode_int(trie+Last_pointed_node+NUM_OCTETS_RESERVED_BY_NODE) )+text);
                Heap_add_item( my_heap,Heap_newItem(decode_int(trie+Last_pointed_node),Last_pointed_node,str_node),compare);
            }
        }
    }

    hash_free_space(list_words_hash);
    return list_words;
}


// get all solution words (with their information),  from a list of solution node. words are ordered by their score topK
// each time we return a group of words (size == topK_Element)
ListString* wordList_exact_1err_TopK_byGroup(TrieInformation *trie_info, exact_and_1err_Solution_nodes* listSolutionNodes, int topK_Element,bool isFirstCall)
{
    uchar *text = trie_info->text;
    int longest_line = trie_info->longest_line;
    uchar *trie = trie_info->trie;
    int limit_leaf = trie_info->limit_leaf;

    int k2;

    int nb_sequence;
    int nb_trans;
    int pointer_sequence;
    int length_transition;

    int pos_code;

    int idx_TopK = 0;

    int sizeListSolutionNodes = Length_List_Int(listSolutionNodes->list_solution_1error);
    int indice_listSolutionNodes = 0;
    int Last_pointed_node=0;

    ListString* list_words = create_ListString();

    char* str_node;


    if(isFirstCall)
    {
        still_exact_solution = true;

        Heap_free_space(my_heap_err);
        my_heap_err = Heap_initialize(longest_line);

        hash_free_space(list_words_hash_err);
        list_words_hash_err = initialisation_HF(INTITIAL_CAPACITY,MAX_SIZE_WORD);


        for(indice_listSolutionNodes=0; indice_listSolutionNodes < sizeListSolutionNodes ; indice_listSolutionNodes++)
        {
            Last_pointed_node = getVal_i_List_Int( listSolutionNodes->list_solution_1error , indice_listSolutionNodes);

            str_node = (char*)((decode_int(trie+Last_pointed_node+NUM_OCTETS_RESERVED_BY_NODE) )+text);
            Heap_add_item( my_heap_err,Heap_newItem(decode_int(trie+Last_pointed_node),Last_pointed_node,str_node),compare);

        }

    }


    if(still_exact_solution)
    {
        if(listSolutionNodes->exact_solution>=0)
        {
            list_words  =wordList_TopK_byGroup(trie_info, listSolutionNodes->exact_solution, topK_Element, isFirstCall);

            if(list_words->size<topK_Element) // this means that the words number in the list is unless of the number topk_element, so chod go to treat the node of 1err result
            {
                still_exact_solution = false;
            }
            else
                return list_words;
        }
        else
            still_exact_solution = false;
    }


    // si on est ariver là cela vaut dire que :
    // 1) soit on a pas une soultion exact, donc on a que des solution aprouchée
    // 2) le nombre des mots de la solution exact sont terminer



    while (Heap_get_size(my_heap_err)>0 && idx_TopK < topK_Element)
    {
        pos_code = Heap_get_top(my_heap_err)->data;

        if (pos_code<limit_leaf)
        {
            nb_sequence=decode_int(trie+pos_code+NUM_OCTETS_RESERVED_BY_NODE);

            if(hash_findItem(list_words_hash_err, (char*)(nb_sequence+text) )==-1)
            {
                list_words=appendIn_ListString(list_words, (char*)(nb_sequence+text) ); // save the word of this node
                hash_addItem(list_words_hash_err, (char*)(nb_sequence+text) );
            }

            Heap_delete_top(my_heap_err,compare);

            idx_TopK++;
        }
        else
        {
            pos_code += NUM_OCTETS_RESERVED_BY_NODE;

            nb_sequence = length_decode_int(&pos_code,trie+pos_code);

            nb_trans = length_decode_int(&pos_code,trie+pos_code);

            for (k2=0; k2<nb_trans; k2++)
            {
                pos_code += u8_seqlen((char*)(trie+pos_code));
            }

            Heap_delete_top(my_heap_err,compare);

            for (k2=0;k2<nb_trans;k2++)
            {
                pointer_sequence = length_decode_int(&pos_code,trie+pos_code);
                length_transition = length_decode_int(&pos_code,trie+pos_code);
                Last_pointed_node = length_decode_int(&pos_code,trie+pos_code);

                str_node = (char*)((decode_int(trie+Last_pointed_node+NUM_OCTETS_RESERVED_BY_NODE) )+text);
                Heap_add_item( my_heap_err,Heap_newItem(decode_int(trie+Last_pointed_node),Last_pointed_node,str_node),compare);
            }
        }
    }

    return list_words;
}


// ----------------------------------------------------------------------------------
// dynamic topK : update All path Nodes of the chosen word (by Searching)
void topK_updating_AllWayNodes_bySearching(uchar *pattern, TrieInformation *trie_info)
{
    uchar *text = trie_info->text;
    uchar *trie = trie_info->trie;
    int firstNode = trie_info->first_node;

    ListInt* list_way_info= create_List_Int();

    int topK_value=0;

    // we search the word, and we save all the path node in stack, and then we make the update

    int i=0;
    int p=0;
    int nbchar_to_match=0;
    int lletter=0;
    int nb_letters=0;
    int k=0;
    int nb_sequence=0;
    int nb_trans=0;
    int pointer_sequence=0;
    int pointed_node=0;
    int length_transition=0;
    int pos_code=0;

    pos_code=firstNode;

    // add the firs node to the stack
    list_way_info = appendIn_List_Int(list_way_info,firstNode);

    nb_letters=strlen((char*)pattern);

    if(nb_letters==0) return;

    k=0;
    while (k<nb_letters)
    {
        nbchar_to_match = u8_seqlen((char*)(pattern+k));

        pos_code += NUM_OCTETS_RESERVED_BY_NODE;
        nb_sequence = length_decode_int(&pos_code,trie+pos_code);
        nb_trans = length_decode_int(&pos_code,trie+pos_code);

        for (i=0; i<nb_trans; i++)
        {
            lletter = u8_seqlen((char*)(trie+pos_code));
            if (lletter==nbchar_to_match)
            {
                if (memcmp(pattern+k , trie+pos_code , nbchar_to_match) ==0) break;
            }

            pos_code += lletter;
        }

        if (i==nb_trans)
        {
            break;
        }
        else
        {
            p =i;

            while (p<nb_trans)
            {
                pos_code += u8_seqlen((char*)(trie+pos_code));
                p++;
            }

            for (p=0;p<=i;p++)
            {
                pointer_sequence=length_decode_int(&pos_code,trie+pos_code);
                length_transition=length_decode_int(&pos_code,trie+pos_code);
                pointed_node=length_decode_int(&pos_code,trie+pos_code);
            }

            k += nbchar_to_match;
            pointer_sequence += nbchar_to_match;
            length_transition -= nbchar_to_match;

            p=0;
            while (p<length_transition && k<nb_letters)
            {
                lletter=u8_seqlen((char*)(text+pointer_sequence));
                nbchar_to_match=u8_seqlen((char*)(pattern+k));

                if (lletter==nbchar_to_match)
                {
                    if (memcmp(pattern+k,text+pointer_sequence,nbchar_to_match) !=0) break;
                }
                else
                {
                    break;
                }

                p += nbchar_to_match;
                k += nbchar_to_match;
                pointer_sequence += nbchar_to_match;
            }

            if (p==length_transition)
            {
                pos_code=pointed_node;

                list_way_info = appendIn_List_Int(list_way_info,pos_code);
            }
            else
            {
                if (k==nb_letters) break;

                /// fprintf(stdout,"Fail reading the transition on character %d\n",k);
                return ;
            }
        }
    }


    // the pattern ends in a final node, or it end in the middle of a transition and in this case we go to the pointed node (finale node)
    if (k==nb_letters)
    {
        pos_code = pointed_node; // the last node

        pos_code += NUM_OCTETS_RESERVED_BY_NODE;
        nb_sequence = length_decode_int(&pos_code,trie+pos_code);

        nb_trans = length_decode_int(&pos_code,trie+pos_code); // number of trans

        if(nb_trans!=0) // if we are in intermediary node, we go to the next node (the finale node)
        {
            // We must find the number of the right transition, we know that the transition have '\0' as char.
            nbchar_to_match = 1; // we have just '\0'
            for (i=0; i<nb_trans; i++)
            {
                lletter = u8_seqlen((char*)(trie+pos_code));
                if (lletter==nbchar_to_match)
                {
                    if (memcmp(pattern+k , trie+pos_code , nbchar_to_match) ==0) break;
                }

                pos_code += lletter;
            }


            if (i!=nb_trans) // there is a transition by the letter
            {
                p =i;

                while (p<nb_trans) // going to the transitions chunck
                {
                    pos_code += u8_seqlen((char*)(trie+pos_code));
                    p++;
                }

                for (p=0;p<=i;p++)
                {
                    pointer_sequence=length_decode_int(&pos_code,trie+pos_code);
                    length_transition=length_decode_int(&pos_code,trie+pos_code);
                    pointed_node=length_decode_int(&pos_code,trie+pos_code);
                }

                pos_code=pointed_node;

                // add the last node
                list_way_info = appendIn_List_Int(list_way_info,pos_code);
            }
        }

        // recupirer le topK de dernnier node de notre mot
        topK_value = decode_int(trie+pos_code);
        topK_value++;


        // updating
        int indice =0;
        int sizeList_way_info = Length_List_Int(list_way_info);

        for(indice=0; indice < sizeList_way_info; indice++)
        {
            if(topK_value > ( decode_int(trie+list_way_info->buffer[indice])) )
            {
                encode_int(topK_value,  trie+list_way_info->buffer[indice] );
            }
        }
    }

    return ;
}


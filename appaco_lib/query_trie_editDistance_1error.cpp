#include "query_trie_editDistance_1error.h"

#define INTITIAL_CAPACITY 1000

static inline int searchWildcare(uchar *pattern,TrieInformation *trie_info, int pointer_sequence, int k,int length_transition,int nb_letters,int pos_code,int pointed_node);

static inline void edit_Distance_SUBST(TrieInformation *trie_info,uchar* pattern,int nb_letters,int save_k, int save_pointer_sequence,int nbChar_rest_to_test_inTransition,int pos_code,int pointed_node,ListInt *listSolution);
static inline void edit_Distance_DELET(TrieInformation *trie_info,uchar* pattern,int nb_letters,int save_k,int pointer_sequence,int nbChar_rest_to_test_inTransition,int pos_code,int pointed_node,ListInt *listSolution);
static inline void edit_Distance_INSERT(TrieInformation *trie_info,uchar* pattern,int nb_letters,int save_k, int save_pointer_sequence,int nbChar_rest_to_test_inTransition,int pos_code,int pointed_node,ListInt *listSolution);

static inline void edit_Distance(TrieInformation *trie_info,uchar* pattern,int nb_letters,int k, int pointer_sequence,int nbChar_rest_to_test_inTransition,int pos_code,int pointed_node,ListInt *listSolution);

static inline void all_wayNodes(TrieInformation *trie_info,uchar* pattern,int nb_letters,ListInt *listSolution);

static inline void test_OneError(TrieInformation *trie_info,uchar* pattern,int nb_letters,int k, int pointer_sequence,int nbChar_rest_to_test_inTransition,int pos_code,int pointed_node,ListInt *listSolution,bool isExactSolutionFound);


static int lasteQueryWord_size=0;
static int pos_LastNode=-1;

static int * tab_nodes_way=NULL;

static int tab_nodes_way_counter=0;

static bool isSolutionExist=false;

static int pos_last_node_char_tested =0;


static int * branch_charLevels = NULL;//Stores the branching levels (the char that represents a branch)

static int * branch_trans_num = NULL;//Stores the transition num of the branching character

static int * branch_nodes = NULL;// Stores the branching nodes

static int * branch_node_trans_chunk = NULL;// Stores a pointer to the chunk representing the branching node in its parent

static int idx_path=0; // counter for the branch nodes and transition


// just for debugging
// approximate search with 1 error, and print the result in console
void searchQueryWord_1error_PrintResult(uchar *pattern, TrieInformation *trie_info)
{
    exact_and_1err_Solution_nodes * listSolution = searchQueryWord_1error(pattern,trie_info);

    printf("\n\n____________________________________________________________________________________________");
    printf("\n\n solution : listSolution.size() = %d ",Length_List_Int(listSolution->list_solution_1error));

    printResult_queryWord_1error(trie_info,listSolution->list_solution_1error);
}

// print result in console
void printResult_queryWord_1error(TrieInformation *trie_info, ListInt *listSolution)
{
    int size_listSolution = Length_List_Int(listSolution);

    int i;
    for(i=0; i< size_listSolution; i++)
    {
        printWordsHavePrefix(trie_info,getVal_i_List_Int(listSolution,i));
    }
}

static inline exact_and_1err_Solution_nodes* new_exact_and_1err_Solution_nodes()
{
    exact_and_1err_Solution_nodes* item=(exact_and_1err_Solution_nodes*)malloc(sizeof(exact_and_1err_Solution_nodes));

    item->exact_solution = -1;
    item->list_solution_1error = create_List_Int();

    return item;
}

static HashTable_int *my_hashTable_int=NULL;

// approximate search : look up for the query word in the Trie (we accept 1 error),and return the solution node (otherwise -1).
// the code is the same as searchQuery Word in query trie, just here we add the part that treat the error, so i will comment just this part
exact_and_1err_Solution_nodes *searchQueryWord_1error(uchar *pattern, TrieInformation *trie_info)
{
    uchar * text = trie_info->text;
    int longest_line = trie_info->longest_line;
    uchar * trie = trie_info->trie;
    int firstNode = trie_info->first_node;

    exact_and_1err_Solution_nodes* listSolution=  new_exact_and_1err_Solution_nodes();// a list containing all solutions nodes : exact (if we have) and with error

    my_hashTable_int=initialisation_HF_int(INTITIAL_CAPACITY);

    int i=0;
    int p=0;
    int nbchar_to_match=0;

    int lletter=0;

    int k=0;

    int nb_sequence=0;
    int nb_trans=0;
    int pointer_sequence=0;
    int length_transition=0;

    int pos_code=0;

    pos_code=firstNode;

    int pointed_node =firstNode;

    int nb_letters=strlen((char*)pattern);
    if(nb_letters==0) return listSolution; // return empty list


    // we can made this initialization just one time. outside of this function
    if(branch_charLevels==NULL)
    {
        branch_charLevels=(int *)malloc((longest_line+1)*sizeof(int));//Stores the branching levels

        branch_trans_num=(int *)malloc((longest_line+1)*sizeof(int));//Stores the transition num of the branching character

        branch_nodes=(int *)malloc((longest_line+1)*sizeof(int));// Stores the branching nodes

        branch_node_trans_chunk=(int *)malloc((longest_line+1)*sizeof(int));// Stores a pointer to the chunk representing the branching node in its parent
    }

    idx_path=0;

    bool isExactSolutionFound =false;

    int nbChar_rest_to_test_inTransition = 0; // size of all chars in bytes still to test in transition (length of substring)


    /// phase 1 : exact search
    // maybe you say : why don't we just call the function that do the exact search and then we make the approximate search
    // response : we have some new code to add in the middle of the exact search, example : save node, transition, chars (for using this information in the approximate search)


    k=0;
    while (k<nb_letters)
    {
        branch_nodes[idx_path] = pos_code; // save the node

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

        branch_charLevels[idx_path]=k; // Set the current branching level (the index of the branching character in the pattern)
        branch_trans_num[idx_path]=i; // Set the branching transition num

        if (i==nb_trans)
        {
            branch_node_trans_chunk[idx_path++]=pos_code; // we save the information of transition nodes where we arrived in the test
            break;
        }
        else // there is a transition by the letter
        {

            p =i;

            while (p<nb_trans)
            {
                pos_code += u8_seqlen((char*)(trie+pos_code));
                p++;
            }

            branch_node_trans_chunk[idx_path++]=pos_code; // we save the information of transition nodes where we arrived in the test

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
            }
            else
            {
                if (k==nb_letters) break;

                fprintf(stdout,"\n\n Fail reading the transition on character %d\n",k);

                break; // We have no solution, go out to do approximate search with 1 error.
            }
        }
    }


    if (k==nb_letters)
    {
        isExactSolutionFound = true;

        /// listSolution = appendIn_List_Int(listSolution,pointed_node); // add the solution node of the exact search
        listSolution->exact_solution = pointed_node;

    }
    /*
    else
    {
        printf("\n\t no solution exact found ");

    }
    */


    /// phase 2: approximate search
    // pos_mismatchCharPattern = k
    // pos_mismatchCharTrie = pointer_sequence
    // pos_code = pos_code
    // pointed_node = pointed_node
    // p = p
    // length_transition = length_transition
    //  nomber of char rest to test  = length_transition-p



    nbChar_rest_to_test_inTransition = length_transition-p;

    test_OneError(trie_info,pattern,nb_letters,k,pointer_sequence,nbChar_rest_to_test_inTransition,pos_code,pointed_node,listSolution->list_solution_1error,isExactSolutionFound);

    hash_free_space_int(my_hashTable_int);
    return listSolution;
}


// there are cases where the result come duplicate, for example find the same word with deletion and insertion

// call the three function for edit distance : deletion, insertion and substitution
static inline void edit_Distance(TrieInformation *trie_info,uchar* pattern,int nb_letters,int k, int pointer_sequence,int nbChar_rest_to_test_inTransition,int pos_code,int pointed_node,ListInt *listSolution)
{
   edit_Distance_DELET(trie_info,pattern,nb_letters,k,pointer_sequence,nbChar_rest_to_test_inTransition,pos_code,pointed_node,listSolution);

   edit_Distance_INSERT(trie_info,pattern,nb_letters,k,pointer_sequence,nbChar_rest_to_test_inTransition,pos_code,pointed_node,listSolution);

   edit_Distance_SUBST(trie_info,pattern,nb_letters,k,pointer_sequence,nbChar_rest_to_test_inTransition,pos_code,pointed_node,listSolution);

   return;
}

// 3 : substitution : skip in the same position, the char in pattern and the char in Trie, and then test others with exact search.
static inline void edit_Distance_SUBST(TrieInformation *trie_info,uchar* pattern,int nb_letters,int save_k, int save_pointer_sequence,int nbChar_rest_to_test_inTransition,int pos_code,int pointed_node,ListInt *listSolution)
{
    int k = save_k + u8_seqlen((char*)(pattern+save_k)); // get the position in the pattern (save_k), and then add the length of the next char to skip it.

    int nbChar_to_jump = u8_seqlen((char*)(trie_info->text+save_pointer_sequence)); // get the length of the char in trie, to skip it

    int pointer_sequence = save_pointer_sequence + nbChar_to_jump; // get the position in the trie (it my in the text) , and then add the length of the next char to skip it.

    int length_transition = nbChar_rest_to_test_inTransition - nbChar_to_jump; // decrease the size of transition, because we skip a char in the trie.

    // test others chars with exact search (we begin from a middle of transition or from node)
    int nodeSolution = searchWildcare(pattern,trie_info,pointer_sequence,k,length_transition,nb_letters,pos_code,pointed_node);


    if(nodeSolution != -1) // if we have solution with substitution method, we add it to the the list
    {
        if(hash_findItem_int(my_hashTable_int,nodeSolution)==-1)
        {
            hash_addItem_int(my_hashTable_int,nodeSolution);
            listSolution = appendIn_List_Int(listSolution, nodeSolution );
        }
    }

    return;
}


// 1 : deletion  : skip char in pattern , and then test others with exact search.
static inline void edit_Distance_DELET(TrieInformation *trie_info,uchar* pattern,int nb_letters,int save_k,int pointer_sequence,int nbChar_rest_to_test_inTransition,int pos_code,int pointed_node,ListInt *listSolution)
{
    int k = save_k + u8_seqlen((char*)(pattern+save_k));// get the position in the pattern (save_k), and then add the length of the next char to skip it.

    int nodeSolution = searchWildcare(pattern,trie_info,pointer_sequence,k,nbChar_rest_to_test_inTransition,nb_letters,pos_code,pointed_node);

    if(nodeSolution != -1)
    {
        if(hash_findItem_int(my_hashTable_int,nodeSolution)==-1)
        {
            hash_addItem_int(my_hashTable_int,nodeSolution);
            listSolution = appendIn_List_Int(listSolution, nodeSolution );
        }
    }

    return;
}


// 2 : insertion : skip char in Trie, and then test others with exact search.
static inline void edit_Distance_INSERT(TrieInformation *trie_info,uchar* pattern,int nb_letters,int save_k, int save_pointer_sequence,int nbChar_rest_to_test_inTransition,int pos_code,int pointed_node,ListInt *listSolution)
{
    int nbChar_to_jump = u8_seqlen((char*)(trie_info->text+save_pointer_sequence));

    int pointer_sequence = save_pointer_sequence + nbChar_to_jump;

    int length_transition = nbChar_rest_to_test_inTransition - nbChar_to_jump;

    int nodeSolution  = searchWildcare(pattern,trie_info,pointer_sequence,save_k,length_transition,nb_letters,pos_code,pointed_node);

    if(nodeSolution != -1)
    {
        if(hash_findItem_int(my_hashTable_int,nodeSolution)==-1)
        {
            hash_addItem_int(my_hashTable_int,nodeSolution);
            listSolution = appendIn_List_Int(listSolution, nodeSolution );
        }
    }

    return;
}


// check nodes path already traversed by the exact search (till the error position)
static inline void all_wayNodes(TrieInformation *trie_info,uchar* pattern,int nb_letters,ListInt *listSolution)
{
    int nbranches=idx_path;  // the number of node and transition chars traversed till the error position

    int pos_code=0;
    int nb_sequence = 0;
    int nb_trans=0;

    int j=0;
    int p=0;

    int pointer_sequence=0;
    int length_transition=0;
    int pointed_node=0;

    // for each node traversed we make edit distance verification just in the first char of each transition
    for(idx_path=0;idx_path<nbranches;idx_path++)
    {
        pos_code=branch_nodes[idx_path]; //  get the node

        // get the node information
        pos_code+=NUM_OCTETS_RESERVED_BY_NODE;
        nb_sequence=length_decode_int(&pos_code,trie_info->trie+pos_code); // number of sequnce
        nb_trans=length_decode_int(&pos_code,trie_info->trie+pos_code); // number of trans

        // for each transition we make edit distance verification (just the first char), we skip the transition traversed before in exact search
        for(j=0;j<nb_trans;j++)
        {
            // if this transition is in the table, this means that we traversed by it in exact search, so we skip it
            if(branch_trans_num[idx_path]==j)
            {
                continue;
            }

            pos_code=branch_node_trans_chunk[idx_path]; // the begin of all outgoing node (their informations)

            // go to the information of node j
            for (p=0;p<=j;p++)
            {
                pointer_sequence=length_decode_int(&pos_code,trie_info->trie+pos_code);
                length_transition=length_decode_int(&pos_code,trie_info->trie+pos_code);
                pointed_node=length_decode_int(&pos_code,trie_info->trie+pos_code);
            }

            // branch_charLevels[idx_path] == save_k, (char  position in pattern)
            // call the edit_Distance function to test error in the beginning of the node
            edit_Distance(trie_info,pattern,nb_letters,branch_charLevels[idx_path],pointer_sequence,length_transition,pos_code,pointed_node,listSolution);
        }
    }

    return;
}

// continue the search with one error from the position where we stopped in the exact search
static inline void test_OneError(TrieInformation *trie_info,uchar* pattern,int nb_letters,int k, int pointer_sequence,int nbChar_rest_to_test_inTransition,int pos_code,int pointed_node,ListInt *listSolution,bool isExactSolutionFound)
{
    // no exact solution
    // check the error where we stopped and also at each node traversed before (the nodes of the path)
    if(!isExactSolutionFound)
    {
        edit_Distance(trie_info,pattern,nb_letters,k,pointer_sequence,nbChar_rest_to_test_inTransition,pos_code,pointed_node,listSolution);

        // edit distance with all nodes of the traversed path
        all_wayNodes(trie_info,pattern,nb_letters,listSolution);
    }
    else // check just nodes of the traversed path
    {
        all_wayNodes(trie_info,pattern,nb_letters,listSolution);
    }

    return;
}


// approximate search with 1 error
// start the search from the solution node of the last query word . if it is total prefix of the current word query, otherwise start the search from the root
exact_and_1err_Solution_nodes* searchSuffixbyPrefix_1error(uchar *pattern, int lengPrefixCommon, TrieInformation *trie_info)
{

    uchar * text = trie_info->text;
    int longest_line = trie_info->longest_line;
    uchar * trie = trie_info->trie;
    int firstNode = trie_info->first_node;


    exact_and_1err_Solution_nodes* listSolution=  new_exact_and_1err_Solution_nodes();// a list containing all solutions nodes : exact (if we have) and with error

    my_hashTable_int=initialisation_HF_int(INTITIAL_CAPACITY);


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

    int last_pointed_node = pointed_node =firstNode;


    nb_letters=strlen((char*)pattern);


    if(branch_charLevels==NULL)
    {
        branch_charLevels=(int *)malloc((longest_line+1)*sizeof(int));

        branch_trans_num=(int *)malloc((longest_line+1)*sizeof(int));

        branch_nodes=(int *)malloc((longest_line+1)*sizeof(int));

        branch_node_trans_chunk=(int *)malloc((longest_line+1)*sizeof(int));
    }

    bool isExactSolutionFound =false;

    int nbChar_rest_to_test_inTransition = 0;

    if(nb_letters==0)
    {
        lasteQueryWord_size=0;
        pos_last_node_char_tested =0;
        pos_LastNode = firstNode;

        idx_path=0;

        return listSolution;
    }


    k=0;

    if(lengPrefixCommon>0 && lengPrefixCommon==lasteQueryWord_size)
    {
        if(isSolutionExist)
        {
            k=pos_last_node_char_tested;
            pos_code=pos_LastNode;

            last_pointed_node = pointed_node = pos_LastNode;
        }
        else
        {
            lasteQueryWord_size = nb_letters;
            isSolutionExist = false;

            // No Solution
        }
    }
    else
    {
        pos_last_node_char_tested =0;
        pos_LastNode = firstNode;

        idx_path=0;
    }


    while (k<nb_letters)
    {
        branch_nodes[idx_path] = pos_code;

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

        branch_charLevels[idx_path]=k;

        branch_trans_num[idx_path]=i;

        if (i==nb_trans)
        {
            branch_node_trans_chunk[idx_path++]=pos_code;
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

            branch_node_trans_chunk[idx_path++]=pos_code;

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

                last_pointed_node = pointed_node;

                pos_last_node_char_tested=k;
            }
            else
            {
                if (k==nb_letters) break;

                lasteQueryWord_size = nb_letters;
                isSolutionExist = false;
                pos_last_node_char_tested = 0;
                pos_LastNode = firstNode;

                break; // no solution
            }
        }
    }

    if (k==nb_letters)
    {
        lasteQueryWord_size=k;
        pos_LastNode=last_pointed_node;

        isSolutionExist = true;
        isExactSolutionFound = true;

        /// listSolution=appendIn_List_Int(listSolution,pointed_node);
        listSolution->exact_solution = pointed_node;
    }
    else
    {
        isSolutionExist = false;
        lasteQueryWord_size = nb_letters;

        pos_last_node_char_tested = 0;
        pos_LastNode = firstNode;

    }

    // phase : edit distance test

    nbChar_rest_to_test_inTransition = length_transition-p;

    test_OneError(trie_info,pattern,nb_letters,k,pointer_sequence,nbChar_rest_to_test_inTransition,pos_code,pointed_node,listSolution->list_solution_1error,isExactSolutionFound);


    hash_free_space_int(my_hashTable_int);
    return listSolution;

}


// approximate search with 1 error
// if there is a common prefix between the two query word, we start the search from node in this level (depth of common prefix)
exact_and_1err_Solution_nodes* search_Prefix_by_Suffix_Delete_1error(uchar *pattern, int lengPrefixCommon, TrieInformation *trie_info)
{
    uchar * text = trie_info->text;
    int longest_line = trie_info->longest_line;
    uchar * trie = trie_info->trie;
    int firstNode = trie_info->first_node;

    if(tab_nodes_way==NULL)
    {
        tab_nodes_way= (int *) malloc(longest_line * sizeof(int));
    }

    exact_and_1err_Solution_nodes* listSolution=  new_exact_and_1err_Solution_nodes();// a list containing all solutions nodes : exact (if we have) and with error

    my_hashTable_int=initialisation_HF_int(INTITIAL_CAPACITY);

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

    int last_pointed_node = pointed_node =firstNode;

    nb_letters=strlen((char*)pattern);

    if(branch_charLevels==NULL)
    {
        branch_charLevels=(int *)malloc((longest_line+1)*sizeof(int));

        branch_trans_num=(int *)malloc((longest_line+1)*sizeof(int));

        branch_nodes=(int *)malloc((longest_line+1)*sizeof(int));

        branch_node_trans_chunk=(int *)malloc((longest_line+1)*sizeof(int));
    }

    bool isExactSolutionFound =false;

    int nbChar_rest_to_test_inTransition = 0;


    if(nb_letters==0)
    {
        lasteQueryWord_size=0;
        pos_last_node_char_tested =0;
        pos_LastNode = firstNode;

        idx_path=0;

        return listSolution;

    }


    k=0;


    if(lengPrefixCommon == 0)
    {
        for(tab_nodes_way_counter=0;tab_nodes_way_counter<longest_line;tab_nodes_way_counter++)
        {
            tab_nodes_way[tab_nodes_way_counter] = -1;
        }
        tab_nodes_way_counter=0;

        tab_nodes_way[tab_nodes_way_counter]=firstNode;

        pos_last_node_char_tested =0;
        pos_LastNode = firstNode;

        idx_path = 0;
    }
    else
    {

        if(lengPrefixCommon == lasteQueryWord_size)
        {
            if(isSolutionExist)
            {
                k=pos_last_node_char_tested;
                pos_code=pos_LastNode;

                last_pointed_node = pointed_node = pos_LastNode;
            }
            else
            {
                lasteQueryWord_size = nb_letters;
                isSolutionExist = false;
            }
        }
        else
            if(lengPrefixCommon < lasteQueryWord_size)
            {
                tab_nodes_way_counter = lengPrefixCommon;

                while(tab_nodes_way[tab_nodes_way_counter]==-1)
                {
                    tab_nodes_way_counter --;
                }

                pos_code = tab_nodes_way[tab_nodes_way_counter];

                k= tab_nodes_way_counter;

                pos_last_node_char_tested = k;
                last_pointed_node = pointed_node = pos_code;

                while(tab_nodes_way_counter < longest_line)
                {
                    tab_nodes_way_counter ++;

                    tab_nodes_way[tab_nodes_way_counter] = -1;
                }

                idx_path=0;
                for(tab_nodes_way_counter=0;tab_nodes_way_counter <=pos_last_node_char_tested;tab_nodes_way_counter++)
                {
                    if(tab_nodes_way[tab_nodes_way_counter]!=-1)
                    {
                        idx_path++;
                    }
                }
            }
    }



    while (k<nb_letters)
    {
        branch_nodes[idx_path] = pos_code;

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

        branch_charLevels[idx_path]=k;

        branch_trans_num[idx_path]=i;

        if (i==nb_trans)
        {
            branch_node_trans_chunk[idx_path++]=pos_code;
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

            branch_node_trans_chunk[idx_path++]=pos_code;

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

                last_pointed_node = pointed_node;

                pos_last_node_char_tested=k;

                tab_nodes_way_counter=k;
                tab_nodes_way[tab_nodes_way_counter] =   pos_code;
            }
            else
            {
                if (k==nb_letters) break;

                lasteQueryWord_size=nb_letters;
                isSolutionExist = false;
                pos_last_node_char_tested = 0;
                pos_LastNode = firstNode;

                break;
            }
        }
    }

    if (k==nb_letters)
    {
        lasteQueryWord_size=k;
        pos_LastNode=last_pointed_node;

        isSolutionExist = true;
        isExactSolutionFound = true;

        /// listSolution=appendIn_List_Int(listSolution,pointed_node);
        listSolution->exact_solution = pointed_node;
    }
    else
    {
        isSolutionExist = false;
        lasteQueryWord_size = nb_letters;

        pos_last_node_char_tested = 0;
        pos_LastNode = firstNode;

    }


    nbChar_rest_to_test_inTransition = length_transition-p;

    test_OneError(trie_info,pattern,nb_letters,k,pointer_sequence,nbChar_rest_to_test_inTransition,pos_code,pointed_node,listSolution->list_solution_1error,isExactSolutionFound);

    hash_free_space_int(my_hashTable_int);
    return listSolution;
}



// check the rest of the chars (exact search), after the operation of edit distance jumps char in the pattern or in the trie or two at the same time
static inline int searchWildcare(uchar *pattern, TrieInformation *trie_info, int pointer_sequence, int k, int length_transition, int nb_letters, int pos_code, int pointed_node)
{
    uchar * text = trie_info->text;

    int p=0;
    int lletter=0;
    int nbchar_to_match=0;

    // if the error is in the middle of transition, so we start the verification from the middle
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

        int pos_node_stop_searsh = pointed_node;

        if(k!=nb_letters)
        {
            // Now the rest of verification made ??from the position of a node.
            // here we call directly the function of the exact search with the position of our node

            int save_first_node = trie_info->first_node;
            trie_info->first_node = pointed_node; // set the current node

            pos_node_stop_searsh = searchQueryWord((uchar*)(pattern+k),trie_info); // Call function exact search to continue the checking

            trie_info->first_node = save_first_node;

        }

        return pos_node_stop_searsh;
    }
    else
    {
        if (k==nb_letters)
        {
            return pointed_node;
        }

        return -1; // no solution
    }

    return -1;

}



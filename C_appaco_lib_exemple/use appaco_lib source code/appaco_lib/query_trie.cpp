#include "query_trie.h"


///  uchar *lasteQueryWord; // the last  pattern searched

static int lasteQueryWord_size=0; // the size in char (le nombre d'octet)

static int pos_LastNode=-1;// solution node for the lasteQueryWord (give us the prefix in the Trie)


static int * tab_nodes_way=NULL;  // contain all nodes in the path followed by the solution

static int tab_nodes_way_counter=0; // number of nodes added to the table

static bool isSolutionExist=false; // if we found solution with the previous query Word

static int pos_last_node_char_tested =0; // the number of the last char tested at the node


//the query word must be encoded in UTF-8

void searchQueryWord_PrintResult(uchar *pattern, TrieInformation *trie_info)
{
    int pos_node_stop_searsh = searchQueryWord(pattern,trie_info);

    if(pos_node_stop_searsh!=-1)
        printWordsHavePrefix(trie_info,pos_node_stop_searsh);
}

// look up for the query word in the Trie,and return the solution node (otherwise -1).
// query word search starts each time from the root
int searchQueryWord(uchar *pattern,TrieInformation *trie_info)
{
    uchar * text = trie_info->text;
    uchar * trie = trie_info->trie;
    int firstNode = trie_info->first_node;

    int i=0;
    int p=0;
    int k=0;

    // utf-8 managing
    //a char encode in utf 8 can take between 1 and 4 byte,so to compare a utf 8 char, we have to compare all his bytes byte by byte (1 byte == 1 type char in c (ASCII char))
    int nbchar_to_match=0; // size of one character from pattern
    int lletter=0; // lenght letter:size in byte ( char in c) of outgoing letter from the node

    int nb_letters=0; // number of all bytes (type char in c) of the pattern

    int nb_sequence=0; // the start position of the sequence of the node in the text
    int nb_trans=0;    // the number of outgoing transition from the node
    int pointer_sequence=0; // the starting position of the transition in the text  (pointer to the text)
    int length_transition=0; // the subsequence length between two nodes                  (length)
    int pointed_node=0; // the address of outgoing node from father node (pointed node (child) from the  father node)
    int pos_code=0;  // the position by the number of byte. to move through the trie

    pos_code=firstNode; // the first node (root)

    nb_letters=strlen((char*)pattern); // number of char  of the pattern
    if(nb_letters==0) return -1;


    while (k<nb_letters)
    {
        nbchar_to_match = u8_seqlen((char*)(pattern+k)); // get the size of character number k from pattern

        pos_code += NUM_OCTETS_RESERVED_BY_NODE; // skip the space of the score topK
        nb_sequence = length_decode_int(&pos_code,trie+pos_code); //get sequence number and advance in the trie, go to the next information
        nb_trans = length_decode_int(&pos_code,trie+pos_code); // get the number of outgoing transition and advance in the trie, go to the next information



        // finding the right transition that match the pattern character
        for (i=0; i<nb_trans; i++)
        {
            lletter = u8_seqlen((char*)(trie+pos_code));
            if (lletter==nbchar_to_match)
            {
                if (memcmp(pattern+k , trie+pos_code , nbchar_to_match) ==0) break;
            }

            pos_code += lletter; // go to the character of the next transition
        }


        if (i==nb_trans) //we didn't find a match
        {
            break;
        }
        else // there is a transition by the letter
        {
            p =i;

            while (p<nb_trans)// skip all characters of remaining transition, going to the transitions chunck
            {
                pos_code += u8_seqlen((char*)(trie+pos_code));
                p++;
            }

            // skip all the information for each transition from the first transition until we get the transition where we find a match.
            for (p=0;p<=i;p++)
            {
                pointer_sequence=length_decode_int(&pos_code,trie+pos_code); //  the starting position of the transition in the text  (pointer to the text)
                length_transition=length_decode_int(&pos_code,trie+pos_code); //  the subsequence length between two nodes
                pointed_node=length_decode_int(&pos_code,trie+pos_code);      // transition node position in the trie
            }

            k += nbchar_to_match; // go to the next character in the pattern
            pointer_sequence += nbchar_to_match; // go to the next position in the text
            length_transition -= nbchar_to_match; // the length of the sub-segment between two nodes decreases with the tested character length

            // check the characters that are between two nodes (from text) with the pattern
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

            // if we tested all character between the the Two node, so we go to the outgoing node (the son node)
            if (p==length_transition)
            {
                pos_code=pointed_node;

                // note : we can have this case : p==length_transition  && k==nb_letters , so here we have solution
            }
            else
            {
                // we tested all pattern character before reaching the next node (pointed_node), we have solution
                if (k==nb_letters) break; //

                // case : p!=length_transition && k!=nb_letters , so no solution
                /// fprintf(stdout,"\n\n Fail reading the transition on character %d\n",k);
                return -1;
            }
        }
    }


    if (k==nb_letters) // if we tested all the pattern chars, so we have a solution
    {
        return pointed_node;
    }

    return -1;
}



// start the search from the solution node of the last query word . if it is total prefix of the current word query, otherwise start the search from the root
// this function is the same as the president function, we have just a little change, so i will comment just the new code.
/// Here every possible case in detail :
///
///  1st queryWord = "ABCD"     |==> we have solution <==|
///
///  2nd queryWord = 1) "ABCD"      , Based on the previous information, return the same solution node without make search
///                = 2) "ABCDXYZ"   , just doing research for "XYZ"
///                = 3) "AB"        , start the search from the root
///                = 4) "XYZ"       , start the search from the root
///                = 5) "ABXY"      , start the search from the root
///                = 6) ""  empty   , no solution
//  ----------------------------------------------------------------------------
///  1st queryWord = "ABCD"     |==> no solution <==|
///
///  2nd queryWord = 1) "ABCD"      , Based on the previous information, directly we say that we haven't solution (without make search)
///                = 2) "ABCDXYZ"   , directly we say that we haven't solution  (without make search)
///                = 3) "AB"        , start the search from the root
///                = 4) "XYZ"       , start the search from the root
///                = 5) "ABXY"      , start the search from the root
///                = 6) ""  empty   , no solution

int searchSuffixbyPrefix(uchar *pattern,int lengPrefixCommon,TrieInformation *trie_info)
{
    uchar * text = trie_info->text;
    uchar * trie = trie_info->trie;
    int firstNode = trie_info->first_node;


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

    int last_pointed_node = pointed_node =firstNode; // position of the first node (root) of the trie


    nb_letters=strlen((char*)pattern);

    if(nb_letters==0) // empty query word, no solution, reset variables, which allows to search from the node of previous request
    {
        lasteQueryWord_size=0;
        pos_last_node_char_tested =0;
        pos_LastNode = firstNode; // root
        return -1; // return no solution
    }

    k=0;

    // lengPrefixCommon must be > 0, to avoid the conflict of the first execution
    if(lengPrefixCommon>0 && lengPrefixCommon==lasteQueryWord_size) // last query word is prefix total of this query word, (or both are the same)
    {
        if(isSolutionExist) // we have solution with the last query word, retrieves the information to continue the search from the last node solution
        {
            k=pos_last_node_char_tested;
            pos_code=pos_LastNode;
            last_pointed_node = pointed_node = pos_LastNode;

        }
        else // no solution in the last query word, so it's the same for the current query Word : no solution
        {
            lasteQueryWord_size = nb_letters;
            isSolutionExist = false;
            return -1; // return no solution for this query Word
        }
    }
    else // not the same prefix, or we are in the first execution
    {
        // reset variables, which allows to search from the node of previous request
        pos_last_node_char_tested =0;
        pos_LastNode = firstNode;
    }


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

                last_pointed_node = pointed_node; // save the node

                pos_last_node_char_tested=k; // save the number of chars which correspond to the node level
            }
            else
            {
                if (k==nb_letters) break;


                // no solution, reset variables, to use them in the next request
                lasteQueryWord_size=nb_letters;
                isSolutionExist = false;
                pos_last_node_char_tested = 0;
                pos_LastNode = firstNode;
                return -1;
            }
        }
    }

    if (k==nb_letters)
    {
        lasteQueryWord_size=k;
        pos_LastNode=last_pointed_node; // save the last node of our queryWord. (it can be the solution node (pointed_node==last_pointed_node) if the pattern chars finished  in level node, and it can be the father of solution node (last_pointed_node) if the pattern chars finished in the middl of the transition ), so in the next query Word we can continue the search (example if it will finish in the middle of transition also)
        isSolutionExist = true;

        return pointed_node;
    }
    else // no solution,
    {
        isSolutionExist = false;
        lasteQueryWord_size = nb_letters;

        pos_last_node_char_tested = 0;
        pos_LastNode = firstNode;
    }

    return -1;

}


// if there is a common prefix between the two query word, we start the search from node in this level (depth of common prefix)
// we have to save all nodes in the path followed by the solution
// this function is the same as the searchSuffixbyPrefix , we have just a little change, so i will comment just the new code.
/// Here every possible case in detail :
///
///  1st queryWord = "ABCD"     |==> we have solution <==|
///
///  2nd queryWord = 1) "ABCD"      , Based on the previous information, return the same solution node without make search
///                = 2) "ABCDXYZ"   , just doing research for "XYZ"
///                = 3) "AB"        , find the node corresponds to the "AB" from the table taht contain all nodes of the path solution (position of the node in table == lenght of common prefix), return the solution node without make search
///                = 4) "XYZ"       , start the search from the root
///                = 5) "ABXY"      , find the node corresponds to the "AB", and continue the search for "XY"
///                = 6) ""  empty   , no solution
//  ----------------------------------------------------------------------------
///  1st queryWord = "ABCD"     |==> no solution <==|
///
///  2nd queryWord = 1) "ABCD"      , Based on the previous information, directly we say that we haven't solution (without make search)
///                = 2) "ABCDXYZ"   , directly we say that we haven't solution  (without make search)
///                = 3) "AB"        , start the search from the root (we restart the search, cause we don't know in wich char the previeus queryword is faild)
///                = 4) "XYZ"       , start the search from the root
///                = 5) "ABXY"      , start the search from the root (we restart the search, cause we don't know in wich char the previeus queryword is faild)
///                = 6) ""  empty   , no solution
int search_Prefix_by_Suffix_Delete(uchar *pattern,int lengPrefixCommon,TrieInformation *trie_info)
{
    uchar * text = trie_info->text;
    int longest_line = trie_info->longest_line;
    uchar * trie = trie_info->trie;
    int firstNode = trie_info->first_node;


    if(tab_nodes_way==NULL)
        tab_nodes_way= (int *) malloc(longest_line * sizeof(int));

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
    if(nb_letters==0)
    {
        lasteQueryWord_size=0;
        pos_last_node_char_tested =0;
        pos_LastNode = firstNode;
        return -1;
    }


    k=0;


    if(lengPrefixCommon == 0) //no common prefix. reset the table that will contain the search path nodes
    {
        for(tab_nodes_way_counter=0;tab_nodes_way_counter<longest_line;tab_nodes_way_counter++)
        {
            tab_nodes_way[tab_nodes_way_counter] = -1;
        }
        tab_nodes_way_counter=0;

        tab_nodes_way[tab_nodes_way_counter]=firstNode; // stacking the root node  (the first node in the solution path)

        pos_last_node_char_tested =0;
        pos_LastNode = firstNode;

    }
    else // it's mean : lengPrefixCommon > 0
    {

        if(lengPrefixCommon == lasteQueryWord_size) // exactly like the previous function : the 1st query Word is a total prefix of the 2nd
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
                return -1;
            }
        }
        else
            if(lengPrefixCommon < lasteQueryWord_size) // we have just a common prefix
            {
                tab_nodes_way_counter = lengPrefixCommon;

                // back in the table if the common prefix ends between two nodes, So back to the previous node. (in this table node position == number of chars till this node (length))
                while(tab_nodes_way[tab_nodes_way_counter]==-1)
                {
                    tab_nodes_way_counter --;
                }

                pos_code = tab_nodes_way[tab_nodes_way_counter]; // get the node

                k= tab_nodes_way_counter; //number of chars tested, so we begin from here

                pos_last_node_char_tested = k;
                last_pointed_node = pointed_node = pos_code;

                // Reset the right side is after "tab_nodes_way counter"
                while(tab_nodes_way_counter < longest_line)
                {
                    tab_nodes_way_counter ++;

                    tab_nodes_way[tab_nodes_way_counter] = -1;
                }
            }
    }




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

                last_pointed_node = pointed_node;

                pos_last_node_char_tested=k;

                tab_nodes_way_counter=k; // get the position of the node in the table (the same as the nb of chars till this node)
                tab_nodes_way[tab_nodes_way_counter] =   pos_code; // save the current node

            }
            else
            {
                if (k==nb_letters) break;

                lasteQueryWord_size=nb_letters;
                isSolutionExist = false;
                pos_last_node_char_tested = 0;
                pos_LastNode = firstNode;
                return -1;
            }
        }
    }


    if (k==nb_letters)
    {
        lasteQueryWord_size=k;
        pos_LastNode=last_pointed_node;
        isSolutionExist = true;

        return pointed_node;
    }
    else
    {
        isSolutionExist = false;
        lasteQueryWord_size = nb_letters;

        pos_last_node_char_tested = 0;
        pos_LastNode = firstNode;
    }

    return -1;
}


// print all information words and chars transition, going through the subtree at the end of the transition, it's exactly like search function but we don't make comparison
void printWordsHavePrefix_allInformation(TrieInformation *trie_info, int Last_pointed_node)
{
    uchar * text = trie_info->text;
    int longest_line = trie_info->longest_line;
    uchar * trie = trie_info->trie;
    int limit_leaf = trie_info->limit_leaf;

    uchar stmp[10];

    int * file_last_nodes=NULL;
    int file_last_nodes_counter =0;
    int k2=0;

    int nb_sequence=0;
    int nb_trans=0;
    int pointer_sequence=0;
    int length_transition=0;

    int pos_code=0;


    file_last_nodes = (int  *) malloc(longest_line*sizeof(int));
    if (file_last_nodes == NULL)
        fatal_error("Cannot allocate memory for file_last_nodes\n");
    file_last_nodes_counter = 0;

    file_last_nodes[0] = Last_pointed_node;
    file_last_nodes_counter = 0;

    while (file_last_nodes_counter>=0)
    {
        pos_code=file_last_nodes[file_last_nodes_counter];

        if (pos_code<limit_leaf)
        {
            nb_sequence=decode_int(trie+pos_code+NUM_OCTETS_RESERVED_BY_NODE)-1;

            fprintf(stdout,"leaf corresponding to sequence ->");
            fprintf(stdout,"%s",nb_sequence+text);
            fprintf(stdout,"\n");

            file_last_nodes_counter--;
        }
        else
        {
            pos_code += NUM_OCTETS_RESERVED_BY_NODE;

            nb_sequence = length_decode_int(&pos_code,trie+pos_code);

            nb_trans = length_decode_int(&pos_code,trie+pos_code);

            for (k2=0; k2<nb_trans; k2++)
            {
                memcpy(stmp,trie+pos_code,u8_seqlen((char*)(trie+pos_code)));
                stmp[u8_seqlen((char*)(trie+pos_code))]=0;

                fprintf(stdout," ");
                fprintf(stdout,(char*)stmp);

                pos_code += u8_seqlen((char*)(trie+pos_code));
            }

            fprintf(stdout,"\n");

            file_last_nodes_counter--;

            for (k2=0;k2<nb_trans;k2++)
            {
                pointer_sequence = length_decode_int(&pos_code,trie+pos_code);
                length_transition = length_decode_int(&pos_code,trie+pos_code);
                Last_pointed_node = length_decode_int(&pos_code,trie+pos_code);

                fprintf(stdout,"(d %d,l %d,pn %d)",pointer_sequence, length_transition,Last_pointed_node);

                file_last_nodes[++file_last_nodes_counter] = Last_pointed_node;
            }

            fprintf(stdout,"\n");
        }
    }
}

// print solution words (going through the subtree at the end of the transition), it's exactly like search function but we don't make comparison
void printWordsHavePrefix(TrieInformation *trie_info, int Last_pointed_node)
{
    uchar * text = trie_info->text;
    int longest_line = trie_info->longest_line;
    uchar * trie = trie_info->trie;
    int limit_leaf = trie_info->limit_leaf;

    int wordCompter=0; // index (number) of word printed

    int * file_last_nodes=NULL;

    int file_last_nodes_counter =0;
    int k2=0;

    int nb_sequence=0;
    int nb_trans=0;
    int pointer_sequence=0;
    int length_transition=0;

    int pos_code=0;


    file_last_nodes = (int  *) malloc(longest_line*sizeof(int));
    if (file_last_nodes == NULL)
        fatal_error("Cannot allocate memory for file_last_nodes\n");
    file_last_nodes_counter = 0;

    file_last_nodes[0] = Last_pointed_node;
    file_last_nodes_counter = 0;

    while (file_last_nodes_counter>=0)
    {
        pos_code=file_last_nodes[file_last_nodes_counter];

        if (pos_code<limit_leaf)
        {
            nb_sequence=decode_int(trie+pos_code+NUM_OCTETS_RESERVED_BY_NODE)-1;


            fprintf(stdout,"\n  %d) %s",wordCompter,nb_sequence+text);
            wordCompter++;

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

            fprintf(stdout,"\n");

            file_last_nodes_counter--;

            for (k2=0;k2<nb_trans;k2++)
            {
                pointer_sequence = length_decode_int(&pos_code,trie+pos_code);
                length_transition = length_decode_int(&pos_code,trie+pos_code);
                Last_pointed_node = length_decode_int(&pos_code,trie+pos_code);

                file_last_nodes[++file_last_nodes_counter] = Last_pointed_node;
            }
        }
    }
}


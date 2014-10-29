#include "trie_coding.h"

typedef struct TextInformation
{
     uchar *text;//a memory area that will contain all our dictionary file.
     int size_text_inByte;
     int nbLigne;//the number of line

     int longest_line;//the longest line (the longest word in the dictionary)
     uchar **line_table;//a table will point to the start address of each line in the memory (the table contains the address of the beginning of each line in the text)

     int * pos_line_table; //a table will contain the start position of each line in the text, example the first line : 0

}TextInformation;


typedef struct Construct_Trie
{
    TextInformation *text_info;

    int *topK_tab; // a table that will contain the score value for each word from our dictionary
                        // Note: the score is at the end of each dictionary word with a separator '#'
                        // example  : abcde#05
                        //            abcguip#890

    int *long_pref_line_next;  //a table that will contain the length of common prefix between line (i) and (i+1)
    int *file_last_nodes; // a table which acts as a stack,
    int file_last_nodes_counter;
    int *linking_nodes; // file of linking each node of same depth in the trie
                     // Each box will contain the last node number of the same depth.

    int * nodes;     // a table that will contain the information of nodes,a node consists of a block of 8 successive memory box.
    int last_node;   // indicate the position of the last nodes inserted into the table

    int pos_code; // it just to advance in tree, byte by byte
    int limit_leaf;    // end of the zone devoted to the leaves
    int local_size; // the size of a single nodes

    uchar * trie;
    int size_trie;
    int first_node;

    int nb_trans_max;
    /// remarque : cette info n'est pas utiliser de tous

    long long Number_Bytes_Occupied_by_Trie;

}Construct_Trie;


static inline void initialize(Construct_Trie *trie_build);
static inline void readFile(const char * nameFile, TextInformation *text_info);
static inline void cuttingTextIntoLines(TextInformation *text_info);
static inline void get_TopK_fromLines(Construct_Trie *trie_build,const char CHAR_SEPARATOR);
static inline void findOffsetLineInText(TextInformation *text_info);
static inline void calculLongestPrefixLines(Construct_Trie *trie_build);
static inline void modifies_node(int * linking, int * ntable,int * lnode, int nb_trans, int tot_8, int depth,int seq_num, int tot_trans, int p_father);
static inline void add_node(int * linking, int * ntable,int * lnode, int nb_trans, int tot_8, int depth,int seq_num, int tot_trans, int p_father);
static inline void nodesIdentify(Construct_Trie *trie_build);
static inline void spaceDeterminingForTrie(Construct_Trie *trie_build);
static inline void fillingTrie(Construct_Trie *trie_build);
static inline int maxTopK_update(int nodeToGetTopK , Construct_Trie *trie_build);

static inline uint32_t utf8_nb_line_text(uchar * text, int length);

static inline void print_TabString(uchar **tabString,int size_tabString);

static int cmpstringp(const void *p1, const void *p2)
{
/*
The arguments of this function are
"pointers to pointers to char",
but the arguments of strcmp (3) are
"pointers to char"
hence forcing the type and use of the asterisk
*/

    /// return strcmp(* (uchar * const *) p1, * (uchar * const *) p2);
    return strcmp(* (char * const *) p1, * (char * const *) p2);
}



// build the Trie from an input file that contains the dictionary
// return a data structure that contains all Trie information
TrieInformation *main_trie_coding(const char *nameFile_dic,Options *my_option)
{
    Construct_Trie *trie_build = (Construct_Trie *)malloc(sizeof(Construct_Trie));
    initialize(trie_build);

    readFile(nameFile_dic,trie_build->text_info);

    cuttingTextIntoLines(trie_build->text_info);


    if(my_option->doSort) // sorting the dictionary
    {
        /// very very important point: if you're sure at 100% that the file that contains the word is definately order, So you can disable this instruction, otherwise we must put it.
        qsort(trie_build->text_info->line_table,trie_build->text_info->nbLigne, sizeof(uchar *), cmpstringp);
    }

    get_TopK_fromLines(trie_build,my_option->topK_CHAR_SEPARATOR);

    findOffsetLineInText(trie_build->text_info);

    calculLongestPrefixLines(trie_build);

    nodesIdentify(trie_build);

    spaceDeterminingForTrie(trie_build);


    fillingTrie(trie_build);


    maxTopK_update(trie_build->first_node,trie_build); // we start from the root node, then the update is done in a recursive way

    /// printingWholeTrie(trie_build->text_info->text,trie_build->text_info->longest_line,trie_build->trie,trie_build->first_node,trie_build->limit_leaf,NUM_OCTETS_RESERVED_BY_NODE);
    /// getchar();

    /// save_trie(nameFile_dic,trie_build->trie,trie_build->size_trie,trie_build->nb_trans_max,trie_build->limit_leaf,trie_build->first_node,trie_build->text_info->longest_line);

    TrieInformation *trieInfo= (TrieInformation*)malloc(sizeof(TrieInformation));

    trieInfo->first_node=trie_build->first_node;
    trieInfo->limit_leaf=trie_build->limit_leaf;
    trieInfo->longest_line=trie_build->text_info->longest_line;
    trieInfo->text=trie_build->text_info->text;
    trieInfo->trie=trie_build->trie;

    trieInfo->Number_Bytes_Occupied_by_Trie = trie_build->Number_Bytes_Occupied_by_Trie;

    return trieInfo;
}

// initialization of all variables to construct the tree
static void initialize(Construct_Trie *trie_build)
{
    trie_build->text_info = (TextInformation *)malloc( sizeof(TextInformation));
    trie_build->text_info->text = NULL;
    trie_build->text_info->longest_line=0;
    trie_build->text_info->size_text_inByte=0;
    trie_build->text_info->nbLigne=0;
    trie_build->text_info->line_table=NULL;
    trie_build->text_info->longest_line=0;
    trie_build->text_info->pos_line_table=NULL;

    trie_build->topK_tab=NULL;

    trie_build->long_pref_line_next=NULL;

    trie_build->file_last_nodes=NULL;
    trie_build->file_last_nodes_counter = 0;
    trie_build->linking_nodes=NULL;
    trie_build->last_node = 0;

    trie_build->limit_leaf=0;
    trie_build->pos_code = 0;
    trie_build->local_size=0;

    trie_build->trie=0;
    trie_build->size_trie=0;
    trie_build->first_node=0;

    trie_build->Number_Bytes_Occupied_by_Trie=0;

    return;
}

// extract information from our dictionary file
// read the information that is in the "nameFile" file and put them in a storage area "text"
// put the file size in "size_text_inByte",
// calculate the number of line in this file and put the result in "nb Ligne"
static void readFile(const char * nameFile,TextInformation *text_info)
{
    struct stat sdata;
    FILE *file_in=NULL;

    if (stat(nameFile,&sdata) != 0)
        fatal_error("\n Could not open stat file\n");

    text_info->size_text_inByte = sdata.st_size;

    text_info->text=(uchar *)malloc((text_info->size_text_inByte+1)*sizeof(uchar));

    if(text_info->text==NULL)
        fatal_error("\n cannot allocate  memory");

    file_in=fopen(nameFile,"r");

    if(fread(text_info->text,sizeof(uchar),text_info->size_text_inByte,file_in)!=(unsigned)text_info->size_text_inByte)  //retrieve all data and put in the memory area "text".
        fatal_error("\n could  not read    file!!!!!!!");

    text_info->text[text_info->size_text_inByte]='\0'; // to mark the end of the memory area (end of string)

    fclose(file_in);

    text_info->nbLigne=utf8_nb_line_text(text_info->text,text_info->size_text_inByte+1);//    calculer    le  nombre  de  ligne   in  the file

    return;
}


// count the number of lines, including a fake char O as newline
// Note: if you use the strlen function, we'll miss the 0 as a line, because for it 0 considered as an end of string
static inline uint32_t utf8_nb_line_text(uchar * text, int length)
{
    int i=0;
    int nl=0;

    while (i<length)
    {
        if (
                ((uint32_t)text[i]==10) || // '\n'
                ((uint32_t)text[i]==13) || // '\r'
                ((uint32_t)text[i]==0)     // '\0'
           )
        {
            nl++;
        }

        i++;
    }

    return nl;
}

// just for debugging
static inline void print_TabString(uchar** tabString,int size_tabString)
{
    int i=0;

    printf("\n\n tabString ...............................\n");

    for(i=0;i<size_tabString;i++)
    {
        printf("%s\n",tabString[i]);
    }
    printf("\n\n .........................................\n");

    return;
}


// cutting the overall text to lines, by setting '0 'at the end of each line
// each line is pointed by a memory box in "line_table"
// find the longest line.
// also recalculate the number of lines "nbline", because we cut a single line by 2, when we have a (tab), '\ t'
static void cuttingTextIntoLines(TextInformation *text_info)
{
    int k=0;
    int tomp_longest_line=0;

    text_info->line_table=(uchar**)malloc(text_info->nbLigne*sizeof(char*));

    if(text_info->line_table==NULL)
        fatal_error("\n could  not allocate    memory  for line_table");

    // skip invalid characters that are before the first line.
    k=0;
    while(  (k<text_info->size_text_inByte)   &&
            (
                ((uint32_t)text_info->text[k]==10) ||  //'\n'
                ((uint32_t)text_info->text[k]==13) ||  //'\r'
                ((uint32_t)text_info->text[k]==0)  ||  //'\0'
                ((uint32_t)text_info->text[k]==9)  ||  //'\t'
                ((uint32_t)text_info->text[k]==32)     //Space
             )
           )
    {
        text_info->text++;// go to the next char
    }

    text_info->line_table[0]=text_info->text;// the address of the first line


    k=0;
    text_info->nbLigne=1;//We will re-calculate the number of line, because a blank line will be ignored


    while (k<text_info->size_text_inByte)
    {
        if(
                ((uint32_t)text_info->text[k]==10) ||  //'\n'
                ((uint32_t)text_info->text[k]==13) ||  //'\r'
                ((uint32_t)text_info->text[k]==0)  ||  //'\0'
                ((uint32_t)text_info->text[k]==9)      //'\t'
                )
        {
            if(tomp_longest_line>(text_info->longest_line))
            {
                text_info->longest_line=tomp_longest_line;
                tomp_longest_line=0;
            }

            text_info->text[k++]=0;//  mark the end of the line

            // skip all the char format and replace them with 0, so go to the next line (valid)
            while(  (k<text_info->size_text_inByte)   &&
                    (
                        ((uint32_t)text_info->text[k]==10) ||  //'\n'
                        ((uint32_t)text_info->text[k]==13) ||  //'\r'
                        ((uint32_t)text_info->text[k]==0)  ||  //'\0'
                        ((uint32_t)text_info->text[k]==9)      //'\t'
                        )
                    )
            {
                text_info->text[k++]=0;
            }

            // we arrived to a new line
            if(k<text_info->size_text_inByte)
            {
                text_info->line_table[text_info->nbLigne++]=&(text_info->text[k]);// save the address of the new line
                tomp_longest_line=0;
            }
        }

        k++;//  Progress in the text till we find a format character
        tomp_longest_line++;//  count the number of characters of the new line
    }

    return;
}



// retrieve TopK score from each line, and put the result in "topK_tab".
// values ??are at the end of words with a separator, example: abcd#34
// when we recover the number, we replace it with 0 at the memory zone(to get a new end of line)
// Consequently, we must recalculate the longest line
static inline void get_TopK_fromLines(Construct_Trie *trie_build,const char CHAR_SEPARATOR)
{
    int k=0;

    trie_build->text_info->longest_line =0;

    int index_char_separator =0;

    char buffer[100];

    int str_lenght=0;

    trie_build->topK_tab = (int *) malloc(trie_build->text_info->nbLigne*sizeof(int));
    if(trie_build->topK_tab==NULL)
        fatal_error("\n could  not allocate    memory  for topK_tab");


    for(k=0; k<trie_build->text_info->nbLigne; k++)
    {
        str_lenght = strlen((char*)(trie_build->text_info->line_table[k]));
        index_char_separator = str_lenght -1;

        // go to the separateur position
        while(index_char_separator>0 && trie_build->text_info->line_table[k][index_char_separator]!= CHAR_SEPARATOR)
        {
            index_char_separator--;
        }

        if(index_char_separator >0)
        {
            int i=0; //retrieve the digits from the line
            for(i=index_char_separator+1; i<= str_lenght; i++)
            {
                buffer[i -(index_char_separator+1)]= trie_build->text_info->line_table[k][i];
            }

            trie_build->topK_tab[k]=strtol(buffer,NULL,10);

            // at the place of number and the separator in memory zone we put 0
            while(index_char_separator < str_lenght)
            {
                trie_build->text_info->line_table[k][index_char_separator]=0;
                index_char_separator++;
            }
        }
        else
        {
            trie_build->topK_tab[k]=0; // in the case where no separator exist, so we put a default  Value '0 '
        }

        // check the longest line
        str_lenght = strlen((char*)(trie_build->text_info->line_table[k]));
        if(str_lenght>trie_build->text_info->longest_line)
        {
            trie_build->text_info->longest_line = str_lenght;
        }
    }

    return;
}



//  find the position of the beginning of the lines in the text, the first line begin at 0
static void findOffsetLineInText(TextInformation *text_info)
{
    int k=0;

    text_info->pos_line_table = (int *) malloc(text_info->nbLigne*sizeof(int));
    if (text_info->pos_line_table == NULL)
        fatal_error("\n Cannot allocate memory for text_pos_line_table_VG\n");

    for (k=0;k<text_info->nbLigne;k++)
    {
        text_info->pos_line_table[k] = text_info->line_table[k] - text_info->text;  // to find the position : the address of the line (K) - the address of text
    }

    return;
}



// calculate the length of common prefix between the line i and i +1
static inline void calculLongestPrefixLines(Construct_Trie *trie_build)
{
    int k=0;

    trie_build->long_pref_line_next = (int *) malloc(trie_build->text_info->nbLigne*sizeof(int));
    if (trie_build->long_pref_line_next == NULL)
        fatal_error("\n Cannot allocate memory for long_pref_line_next\n");

    for (k=0;k<trie_build->text_info->nbLigne-1;k++)
    {
        trie_build->long_pref_line_next[k] = utf8_char_len_pgpc(trie_build->text_info->line_table[k],trie_build->text_info->line_table[k+1]);
    }

    return;
}


// calculate the length of greatest common prefix (the chars are in utf-8)
uint32_t utf8_char_len_pgpc(uchar * str1, uchar* str2)
{
    uint32_t len=0;

    uchar mask1 = 1 << 7;
    uchar mask2 = 1 << 6;

    while(str1[len]  && (str1[len]==str2[len]) ) len++;

    while(len>0 && (str1[len]&mask1) && !(str1[len]&mask2) )len--;

    return len;
}



// first steps: identifying necessary nodes to build our compact trie
static void nodesIdentify(Construct_Trie *trie_build)
{
    int k=0;

    int node_tmp=0;

    int node_pointed=0; // in the middle of a transition
    int node_father=0;


    // file of the last nodes in the trie
    trie_build->file_last_nodes = (int  *) malloc(trie_build->text_info->longest_line*sizeof(int));
    if (trie_build->file_last_nodes == NULL)
        fatal_error("\n Cannot allocate memory for file_last_nodes\n");

    trie_build->file_last_nodes_counter = 0;


    // file of linking each node of same depth in the trie
    // Each box will contain the last node number of the same depth.
    trie_build->linking_nodes = (int  *) malloc((1+trie_build->text_info->longest_line)*sizeof(int));
    if (trie_build->linking_nodes == NULL)
        fatal_error("\n Cannot allocate memory for linking_nodes\n");

    for (k=0;k<=trie_build->text_info->longest_line;k++)
    {
        trie_build->linking_nodes[k]=-1;// initialization
    }


    // a table that will contain the information of nodes,a node consists of a block of 8 successive memory box.
    trie_build->nodes = (int  *) malloc(trie_build->text_info->nbLigne*NUM_PAR*2*sizeof(int)+2*NUM_PAR);
    if (trie_build->nodes == NULL)
        fatal_error("\n Cannot allocate memory for nodes\n");


    // [O] -> number of outgoing transitions
    // [1] -> size in char of all utf_8 first symbol of all outgoing transition
    // [2] -> depth of the node in the trie
    // [3] -> (sequence number (line), number of the shortest sequences on which the node relies
    // [4] -> upper bound in char of the coding of all transitions (pointer to the text, length, pointed node)
    //  : the number of bytes required to encode all outgoing transition, for each transition we have :
    //                                         the starting position of the transition in the text (pointer to the text)
    //                                         the length of the sub-sequence (number of chars that are between two nodes, Father & Son) (length)
    //                                         start position to encode this node (leaf) in the Trie. (node pointed)
    // [5] -> pointer to the father (-1 for the first node)
    // [6] -> pointer to the previous node of same deph, -1 if none.
    // [7] -> pointer to its place in the (future) reserved memory.


    // indicate the position of the last nodes inserted into the table
    trie_build->last_node = -NUM_PAR;


    // insert the 2 first nodes,(2 first nodes of the trie, root and first transition for the first line).

    // 1st nodes is the root
    add_node(trie_build->linking_nodes, trie_build->nodes,&(trie_build->last_node),1,0,0,0,0,-1);

    // node for the 1st line
    add_node(trie_build->linking_nodes,trie_build->nodes,&(trie_build->last_node),0,0,strlen((const char*)trie_build->text_info->text),0,0,0);


    trie_build->file_last_nodes[0] =0;       // the first position of nodes (which is the root) is "0"
    trie_build->file_last_nodes[1] =NUM_PAR; // the position of 2nd node is "NUM_PAR == 8", because each node occupy 8 byte
    trie_build->file_last_nodes_counter=1;


    // treat others lines
    // for each line we have a leaf node
    // if we have common prefix between two line, we create intermediary node
    for (k=1;k<trie_build->text_info->nbLigne;k++)
    {
        node_tmp= trie_build->file_last_nodes[trie_build->file_last_nodes_counter]; // retrieve the position of the last node inserted

        // check if we have the same depth (common prefix) between the line k-1 and K
        // '=='    that's means we are in this case    {1)abc,    2)abc }
        // '>'     that's means we are in this case    {1)abc,    2)abcd }
        if (trie_build->long_pref_line_next[k-1]>= trie_build->nodes[node_tmp+2])
        {
            // same depth or longer

            if (*(trie_build->text_info->line_table[k]+trie_build->long_pref_line_next[k-1])==0) // in this case 1)abc, 2)abc, we do nothing
            {
                fprintf(stdout,"Lines %d and %d are identical !\n", trie_build->nodes[node_tmp+3],k);
            }
            else // in this case    1)abc,    2)abcd
            {
                // example
                // 1) abc   :  N0--->N1
                //
                // 2) abcd  :         --->N1
                //           N0--->N2
                //                    --->N3

                // get the father (N0)
                node_tmp= trie_build->file_last_nodes[--trie_build->file_last_nodes_counter];

                // N1 : pointed by N0
                node_pointed = trie_build->file_last_nodes[trie_build->file_last_nodes_counter+1];

                // adding a new node (an intermediate node N2), the current line correspond to a node
                add_node(trie_build->linking_nodes, trie_build->nodes,&(trie_build->last_node),2,0,trie_build->long_pref_line_next[k-1],k,0,node_tmp);

                node_father =trie_build->last_node;// N2

                trie_build->nodes[node_pointed+5]=trie_build->last_node;   // new father for N1,(update)

                trie_build->file_last_nodes[++trie_build->file_last_nodes_counter]=trie_build->last_node; // added in the stack

                // adding a new leaf for the line (N3)
                add_node(trie_build->linking_nodes,trie_build->nodes,&(trie_build->last_node),0,0,strlen((const char*)trie_build->text_info->line_table[k]),k,0,node_father);

                trie_build->file_last_nodes[++trie_build->file_last_nodes_counter]=trie_build->last_node; // added in the stack

            }
        }
        else // we are in this case : example a: 1)abcde  2)abcxyz  or example b:  1)abcde  2)abcxyz 3)abzz
        {
            // return backwards to find the node of common prefix (example b, insert the 3rd word)
            while (trie_build->long_pref_line_next[k-1]< trie_build->nodes[node_tmp+2] )
            {
                node_tmp= trie_build->file_last_nodes[--trie_build->file_last_nodes_counter];
            }


            // Here we divide the transition, create an intermediary node and then insert the leaf node
            if (trie_build->long_pref_line_next[k-1]> trie_build->nodes[node_tmp+2])
            {
                // in the middle of a transition
                node_pointed = trie_build->file_last_nodes[trie_build->file_last_nodes_counter+1];  // node pointed by the old transition

                add_node(trie_build->linking_nodes, trie_build->nodes,&(trie_build->last_node),2,0,trie_build->long_pref_line_next[k-1],k,0,node_tmp);

                node_father =trie_build->last_node;

                trie_build->nodes[node_pointed+5]=trie_build->last_node;

                trie_build->file_last_nodes[++trie_build->file_last_nodes_counter]=trie_build->last_node;

            }
            else    // here we have a prefix of the same depth as that of father (or grand father), we insert directly the leaf node
            {
                trie_build->nodes[node_tmp]++;   // one more transition
                node_father =node_tmp;

            }

            // adding a new leaf for the line
            add_node(trie_build->linking_nodes,trie_build->nodes,&(trie_build->last_node),0,0,strlen((const char*)trie_build->text_info->line_table[k]),k,0,node_father);

            trie_build->file_last_nodes[++trie_build->file_last_nodes_counter]=trie_build->last_node; // added in the stack
        }
    }

    return;
}


// modifies a node in the table of nodes "ntable"
//
// int * linking : file of linking each node of same depth in the trie (Each box will contain the last node number of the same depth)
// int * ntable  : nodes table (nodes);
// int * lnode   : last_node ,indicate the position of the last nodes inserted into the table
// int nb_trans  : number of outbound transactions from a node
// int tot_8     : size in char of all utf_8 first symbol of all outgoing transition.
// int depth     : depth of the node in the trie
// int seq_num   : sequence number on which the node is based on
// int tot_trans : upper bound in char of the coding of all transitions (pointer to the text, length, pointed node)
// int p_father  : pointer to the father (-1 for the first node)
static void modifies_node(int * linking, int * ntable,int * lnode, int nb_trans, int tot_8, int depth,int seq_num, int tot_trans, int p_father)
{
    ntable[*lnode]=nb_trans; // it's the same ntable[*lnode+0]=nb_trans, we are in the first memory box
    ntable[*lnode+1]=tot_8;
    ntable[*lnode+2]=depth;
    ntable[*lnode+3]=seq_num;
    ntable[*lnode+4]=tot_trans;
    ntable[*lnode+5]=p_father;
    ntable[*lnode+6]=linking[depth];
    ntable[*lnode+7]=-1;             //  pointer to its place in the (future) reserved memory.

    linking[depth]=*lnode;  // The last node number of the same depth
}


// adds a node in the table of nodes "ntable"
static void add_node(int * linking, int * ntable,int * lnode, int nb_trans, int tot_8, int depth,int seq_num, int tot_trans, int p_father)
{
    *lnode+=NUM_PAR; // (last_node) must proceed with "NUM_PAR==8" position.

    modifies_node(linking,ntable,lnode,nb_trans,tot_8,depth,seq_num,tot_trans,p_father);
}


// SECOND PHASE: DETERMINING THE SPACE REQUIRED BY THE TRIE

// calculer l'espace memoire nécessaire pour chaque noeud.
// set the address of each node in the memory according to the size of previous nodes.
static void spaceDeterminingForTrie(Construct_Trie *trie_build)
{
    int father;
    int k=0;
    int node_tmp;


    // coding the leaves in a reserved space, (and make update to the father and leaf node)

    for (k=0; k<=trie_build->last_node; k+=NUM_PAR)
    {
        if (trie_build->nodes[k]==0) // a leaf
        {
            trie_build->local_size=NUM_OCTETS_RESERVED_BY_NODE; // free space added for TopK

            trie_build->local_size+=length_encode_int(trie_build->text_info->pos_line_table[trie_build->nodes[k+3]]); // on how much byte is coded (the position in the text) of the line k.



            if (trie_build->nodes[k+5]>-1) // if the node have a father
            {
                // adding the size of coding the (transition to the node) to (the size required) to code this node

                father=trie_build->nodes[k+5];

                trie_build->nodes[father+4] +=  length_encode_int(trie_build->text_info->pos_line_table[trie_build->nodes[k+3]]+trie_build->nodes[father+2]) // the position (in the text) of the beginning of the transition of node "k"
                                            + length_encode_int(trie_build->nodes[k+2]-trie_build->nodes[father+2]) // the transition length between the two node, father and son, so the number of chars between them.
                                            + length_encode_int(trie_build->pos_code); //start position (to store node "k" information ) in the Trie

                trie_build->nodes[father+1]+=u8_seqlen((char*)(trie_build->text_info->line_table[trie_build->nodes[k+3]]+trie_build->nodes[father+2])); // size in char utf_8 first symbol of outgoing transition
            }

            trie_build->nodes[k+7]=trie_build->pos_code;  // future place of the node in the memory chunck
            // the node [0] is at 0, the node [1] will be at the address (space node [0]), ... etc

            trie_build->pos_code+=trie_build->local_size;
        }
    }


    trie_build->limit_leaf = trie_build->pos_code; // end of the zone devoted to the leaves !
    // all leaf nodes are encoded in a contiguous memory area

    /// fprintf(stdout,"\n Number of Bytes to code the leaves %d o, %d Ko, %d Mo",trie_build->limit_leaf,trie_build->limit_leaf/1024,(trie_build->limit_leaf/1024)/1024);


    /// coding the other nodes, starting from limit_leaf !
    // do the same with the remaining nodes (so intermediary nodes)

    for (k=trie_build->text_info->longest_line;k>=0;k--) // for all possible depth in decreasing order
    {
        node_tmp = trie_build->linking_nodes[k];

        //processing the nodes which are at the same depth
        while (node_tmp>-1) // there are nodes at this depth, follow the guide
        {
            if (trie_build->nodes[node_tmp]!=0) // not a leaf, otherise already coded
            {
                trie_build->local_size=NUM_OCTETS_RESERVED_BY_NODE; //free space added for the topK

                // we retrieves the position of the beginning of this sequence
                trie_build->local_size+=length_encode_int(trie_build->text_info->pos_line_table[trie_build->nodes[node_tmp+3]]);

                trie_build->local_size+=length_encode_int(trie_build->nodes[node_tmp]);  // number of transitions

                trie_build->local_size+=trie_build->nodes[node_tmp+1]; // plus the size of utf8

                trie_build->local_size+=trie_build->nodes[node_tmp+4]; // plus the size of coding the transitions,


                // we do the same thing as the leaf nodes
                if (trie_build->nodes[node_tmp+5]>-1) // to the father
                {
                    // adding the size of coding the transition to the node to the size required to code the node
                    father=trie_build->nodes[node_tmp+5];

                    trie_build->nodes[father+4] += length_encode_int(trie_build->text_info->pos_line_table[trie_build->nodes[node_tmp+3]]+trie_build->nodes[father+2])
                            +length_encode_int(trie_build->nodes[node_tmp+2]-trie_build->nodes[father+2])
                            +length_encode_int(trie_build->pos_code);

                    trie_build->nodes[father+1]+=u8_seqlen((char*)(trie_build->text_info->line_table[trie_build->nodes[node_tmp+3]]+trie_build->nodes[father+2]));
                }

                trie_build->nodes[node_tmp+7]=trie_build->pos_code;  // future place of the node in the memory chunck

                trie_build->pos_code+=trie_build->local_size;
            }

            node_tmp = trie_build->nodes[node_tmp+6];  // next node at the same depth
        }
    }

    trie_build->Number_Bytes_Occupied_by_Trie = trie_build->pos_code;

    /// fprintf(stdout,"\n Number of Bytes to code all nodes %d o, %d Ko, %d Mo",trie_build->pos_code,trie_build->pos_code/1024,(trie_build->pos_code/1024)/1024);


    return;
}



// filling the informations nodes in the tree.
//      Initializes the positions where to place the transitions,
//      and filling them in the tree
static void fillingTrie(Construct_Trie *trie_build)
{
    int k=0;
    int beg_utf8;
    int father;

// THIRD PHASE: INITIALIZES THE POSITION WHERE TO PLACE TRANSITIONS

    trie_build->trie = (uchar *)malloc(trie_build->pos_code*sizeof(uchar));
    if (trie_build->trie==NULL)
        fatal_error("\n Cannot allocate memory for the trie !\n");
    memset(trie_build->trie,0,trie_build->pos_code);

    trie_build->size_trie=trie_build->pos_code;   // size of the trie
    trie_build->pos_code = 0;

    // insert the information in Trie (not all info) , and make updates to the insertion position.

    for (k=0; k<=trie_build->last_node; k+=NUM_PAR)
    {
        trie_build->pos_code = trie_build->nodes[k+7];  // position of the node in the trie chunck (its place in the reserved memory)

        if (trie_build->nodes[k]!=0) // not a leaf
        {
            trie_build->local_size = trie_build->pos_code + NUM_OCTETS_RESERVED_BY_NODE;

            /*
            if (strlen((char*)trie_build->text_info->line_table[trie_build->nodes[k+3]])==trie_build->nodes[k+2])
            {
                encode_int(trie_build->topK_tab[trie_build->nodes[k+3]],trie_build->trie+trie_build->pos_code); // encode the topK score

                trie_build->local_size += all_encode_int(trie_build->text_info->pos_line_table[trie_build->nodes[k+3]],trie_build->trie+trie_build->local_size); // encoding seqeunce
            }
            else
            {
                trie_build->local_size+=all_encode_int(0,trie_build->trie+trie_build->local_size);
            }
            */
            // -------------------- if else precedent will be : cause dans un node intermidière on met pas de topK au debut, aussi un node intermidière dans cette veresion ni jami finale
            trie_build->local_size += all_encode_int(trie_build->text_info->pos_line_table[trie_build->nodes[k+3]],trie_build->trie+trie_build->local_size); // encoding seqeunce
            // -------------------------------------------------------------------------

            trie_build->local_size += all_encode_int(trie_build->nodes[k],trie_build->trie+trie_build->local_size);// encoding of the number of transitions


            beg_utf8 = trie_build->local_size;

            trie_build->local_size += trie_build->nodes[k+1]; // plus the size of utf8

            trie_build->nodes[k+1] = beg_utf8;  // beginning of the uft8 coding
            trie_build->nodes[k+4] = trie_build->local_size; // beginning of the transition coding
        }
        else // a leaf
        {
            // position of the node in the trie chunck
            trie_build->local_size = trie_build->pos_code + NUM_OCTETS_RESERVED_BY_NODE;  // encoding of the number of sequence

            encode_int(trie_build->topK_tab[trie_build->nodes[k+3]],trie_build->trie+trie_build->pos_code); // encode the topK score

            encode_int(trie_build->text_info->pos_line_table[trie_build->nodes[k+3]],trie_build->trie+trie_build->local_size); // encoding the sequence number

        }
    }


    // FOURTH PHASE: FILLING THE TRANSITIONS OF THE TRIE

    // Using the FATHER of EACH NODE to fill the utf8 transitions
    for (k=0; k<=trie_build->last_node; k+=NUM_PAR)
    {
        if (trie_build->nodes[k+5]>-1) // there is a father
        {
            father = trie_build->nodes[k+5]; // utf8 managing

            trie_build->pos_code = trie_build->nodes[father+1];   // position of the chunk utf8 of the father

            memcpy(trie_build->trie+trie_build->pos_code , trie_build->text_info->line_table[trie_build->nodes[k+3]]+trie_build->nodes[father+2] , u8_seqlen((char*)(trie_build->text_info->line_table[trie_build->nodes[k+3]]+trie_build->nodes[father+2])));

            // add current char size to go to the next position to store the next transition char.
            trie_build->nodes[father+1] += u8_seqlen((char*)(trie_build->text_info->line_table[trie_build->nodes[k+3]]+trie_build->nodes[father+2]));
        }
    }

    // Using the FATHER of EACH NODE to fill the integer transitions
    for (k=0; k<=trie_build->last_node; k+=NUM_PAR)
    {
        if (trie_build->nodes[k+5]>-1) // there is a father
        {
            father = trie_build->nodes[k+5];// utf8 managing

            trie_build->pos_code = trie_build->nodes[father+1];   // position of the chunk utf8 of the father

            // encoded the position of the beginning of the transition in the text              (pointer to the text)
            trie_build->pos_code += all_encode_int(trie_build->text_info->pos_line_table[trie_build->nodes[k+3]]+trie_build->nodes[father+2] , trie_build->trie+trie_build->pos_code);
            // number of char between two nodes                  (length)
            trie_build->pos_code += all_encode_int(trie_build->nodes[k+2]-trie_build->nodes[father+2] , trie_build->trie+trie_build->pos_code);
            // start position to encode this node in the Trie (pointed node)
            trie_build->pos_code += all_encode_int(trie_build->nodes[k+7] , trie_build->trie+trie_build->pos_code);
            trie_build->nodes[father+1] = trie_build->pos_code;  // new bound for the transition zone of the father
        }
    }

    trie_build->first_node=trie_build->nodes[7];

    /// ///////////////////////////////////////////////////////////////////////////////////
    ///
    //    SUMMARY: insertion in the Trie the following information in order:
    // 0)  topK score
    // 1)  the position of the sequence in the text of the node "k" for leafs and intermediary node (both)
    // 2)  the number of transition
    //
    // 3) filling the first character of each transition in the Trie
    //
    // 4) encode all transition, for each transition we have :
    //  4')               the starting position of the transition in the text          (pointer to the text)
    //  4'')              number of char that are between two nodes                  (length)
    //  4''')             start position to encode this node in the Trie (pointed node)
    //
    /// so for :
    ///       Leaf node: 0, 1
    //
    ///   internal node :  0, 1 , 2 , 3 , 4
    /// ///////////////////////////////////////////////////////////////////////////////////

    return;
}


// update (TopK score) for all internal node
// each node will take the max value of his transitions (son node)
// initially the score TopK is in the leaf node
static int maxTopK_update(int nodeToGetTopK, Construct_Trie *trie_build)
{
    int nb_sequence=0; // the start position of the sequence of the node in the text
    int nb_trans=0;    // the number of outgoing transition from the node
    int pointer_sequence=0; // the starting position of the transition in the text  (pointer to the text)
    int length_transition=0; // the subsequence length between two nodes                  (length)
    int pointed_node=0;
    int pos_code=0;


    int k=0;
    int max_TopK_ofTransitionNode=0;

    int tomp_max=0;



    pos_code = nodeToGetTopK; // get the node position from the trie

    if ( pos_code < trie_build->limit_leaf ) // all the leaf nodes are coded 0 -> limit leaf (contiguous space)
    {
        // note : for leafs we have just :
        //    0)  topK score
        //    1)  the start position of the sequence of the node in the text.

        tomp_max = decode_int(trie_build->trie+nodeToGetTopK);

        if(tomp_max > max_TopK_ofTransitionNode)
        {
            max_TopK_ofTransitionNode = tomp_max;
        }
    }
    else // not a leaf
    {
        pos_code += NUM_OCTETS_RESERVED_BY_NODE; // we avoid the topK score

        nb_sequence = length_decode_int(&pos_code,trie_build->trie+pos_code); // number of sequnce

        // it means that this node is not only intermediate. but it is also leaf
        /// we haven't this case, because we changed the structure to support topK
        if ( nb_sequence > 0 )
        {
            tomp_max = decode_int(trie_build->trie+nodeToGetTopK);

            if(tomp_max > max_TopK_ofTransitionNode)
            {
                max_TopK_ofTransitionNode = tomp_max;
            }
        }


        nb_trans = length_decode_int(&pos_code,trie_build->trie+pos_code); // number of trans


        // avoid the first char of each outgoing transition from this node
        for (k=0; k < nb_trans; k++)
        {
            pos_code+=u8_seqlen((char*)(trie_build->trie+pos_code)); // go to the next char
        }

        // for each transition node , we apply the update of the score topK
        for (k=0; k<nb_trans; k++)
        {
            pointer_sequence=length_decode_int(&pos_code,trie_build->trie+pos_code); //just to advance in the trie
            length_transition=length_decode_int(&pos_code,trie_build->trie+pos_code); //just to advance in the trie
            pointed_node=length_decode_int(&pos_code,trie_build->trie+pos_code); // transition node position in the trie, (we need just this information)

            tomp_max = maxTopK_update(pointed_node,trie_build);

            if(tomp_max > max_TopK_ofTransitionNode)
            {
                max_TopK_ofTransitionNode = tomp_max;
            }
        }
    }

    encode_int(max_TopK_ofTransitionNode,trie_build->trie+nodeToGetTopK);

    return max_TopK_ofTransitionNode;
}


// printing the whole trie
void printingWholeTrie(uchar * text,int longest_line,uchar * trie,int first_node,int limit_leaf, int num_octet_by_node)
{
    uchar stmp[10];

    int * file_last_nodes;
    int file_last_nodes_counter =0;

    int nb_sequence;
    int nb_trans;
    int pointer_sequence;
    int length_transition;
    int pointed_node;
    int pos_code;


    fprintf(stdout,"Is a prefix \n");

    // going through the subtree at the end of the transition

    file_last_nodes = (int  *) malloc(longest_line*sizeof(int)); //  a table which acts as a stack,
    if (file_last_nodes == NULL)
        fatal_error("\n Cannot allocate memory for file_last_nodes\n");
    file_last_nodes_counter = 0;

    int k=0;


    file_last_nodes[0]=first_node; // stacking the root node
    file_last_nodes_counter=0;

    while ( file_last_nodes_counter >=0 ) // while still nodes in the stack we treat them
    {
        pos_code = file_last_nodes[file_last_nodes_counter]; // get node from the stack

        if ( pos_code < limit_leaf ) // all the leaf nodes are coded 0 -> limit leaf (contiguous space)
        {
            nb_sequence = decode_int(trie+pos_code+num_octet_by_node)-1;

            fprintf(stdout,"leaf corresponding to sequence ->");
            fprintf(stdout,"%s",nb_sequence+text);
            fprintf(stdout,"\n");

            file_last_nodes_counter--; //we finish with this node, so it pops
        }
        else // not a leaf
        {

            pos_code += num_octet_by_node; // skip the the topK

            nb_sequence = length_decode_int(&pos_code,trie+pos_code); // number of sequnce

            // it means that this node is not only intermediate. but it is also leaf
            /// we haven't this case, because we changed the structure to support topK
            if ( nb_sequence > 0 )
            {
                fprintf(stdout,"Internal node corresponding to sequence ->");
                fprintf(stdout,"%s",(--nb_sequence)+text);
                fprintf(stdout,"\n");
            }

            nb_trans = length_decode_int(&pos_code,trie+pos_code); // number of trans

            fprintf(stdout,"Transition characters:");

            // print the first char of each outgoing transition from this node
            // they are stored one after the other in a contiguous space
            for (k=0; k < nb_trans; k++)
            {
                memcpy(stmp,trie+pos_code,u8_seqlen((char*)(trie+pos_code))); // get the char from the Trie
                stmp[u8_seqlen((char*)(trie+pos_code))]=0;

                fprintf(stdout," char num %d = ",k);
                fprintf(stdout,"%s ,",stmp);

                pos_code+=u8_seqlen((char*)(trie+pos_code)); // go to the next char
            }

            fprintf(stdout,"\n");

            file_last_nodes_counter--; //we finish with this node, so it pops

            // stack the transition  nodes of the current node in the stack. (then we choose the last node stacked to treat it)
            for (k=0; k<nb_trans; k++)
            {
                pointer_sequence=length_decode_int(&pos_code,trie+pos_code);  //just to advance in the trie
                length_transition=length_decode_int(&pos_code,trie+pos_code); //just to advance in the trie
                pointed_node=length_decode_int(&pos_code,trie+pos_code); // transition node position in the trie, (we need just this information)

                file_last_nodes[++file_last_nodes_counter]=pointed_node; // stacking the transition node.

            }
            fprintf(stdout,"\n");
        }
    }
}


// saving the trie in a file "name_file"
void save_trie(const char * nameFile_dic,uchar * trie,int size,int nb_trans_max,int limit_leaf,int pos_begin, int longest_line)
{
    char name_file[250];
    strcpy(name_file,nameFile_dic);
    strcat(name_file,".trie1");


    FILE *f;
    int k;
    unsigned char tmp[15];

    f = fopen (name_file,"w");

    k= all_encode_int(size,tmp);  // size of the trie
    if (fwrite (tmp,k,1,f) != 1)
        fatal_error("\n Could not write file 1\n");

    k= all_encode_int(NUM_OCTETS_RESERVED_BY_NODE,tmp);  // number of free octets by node, topK
    if (fwrite (tmp,k,1,f) != 1)
        fatal_error("\n Could not write file 1\n");

    k= all_encode_int(nb_trans_max,tmp); // maximal number of transitions
    if (fwrite (tmp,k,1,f) != 1)
        fatal_error("\n Could not write file 2\n");

    k= all_encode_int(limit_leaf,tmp); // position of the limit of the leaf zone
    if (fwrite (tmp,k,1,f) != 1)
        fatal_error("\n Could not write file 3\n");

    k= all_encode_int(pos_begin,tmp); // position of the first node
    if (fwrite (tmp,k,1,f) != 1)
        fatal_error("\n Could not write file 4\n");

    k= all_encode_int(longest_line,tmp); // longest_line
    if (fwrite (tmp,k,1,f) != 1)
        fatal_error("\n Could not write file 4\n");

    if (fwrite ((const void *)trie,size,1,f) != 1) // the trie itself
        fatal_error("\n Could not write file 5\n");

    fclose(f);

}



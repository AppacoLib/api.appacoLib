#include "MainConsol.h"


static inline void empty_scanf_buffer()
{
    // vider le buffer de scanf (le meme buff pour gets)
    scanf ("%*[^\n]");
    getchar ();
}

// get the query word from UTF-8 file
// the queryWord must be in the 2ed ligne of the file
static inline char* get_queryWord_fromFile(char *nameFile)
{
    char *queryWord = (char*)malloc((MAX_SIZE_WORD+1)*sizeof(char));

    int sizeWord = 0;

    // read the queryWord from file :
    {
        FILE *file_dico = NULL;
        file_dico = fopen(nameFile, "r");

        if (file_dico == NULL)
        {
            printf("\n Unable to load Word query \n there is a problem with the file that contain the queryWord");
            exit(-1);
        }

        // read the first line
        if(fgets(queryWord, MAX_SIZE_WORD, file_dico) == NULL )
        {
            fclose(file_dico);

            printf("\n\t error : the queryWord must be in the 2ed ligne of the file");

            exit(EXIT_FAILURE);
        }

        // read the query word from the second line
        if(fgets(queryWord, MAX_SIZE_WORD, file_dico) != NULL )
        {
            sizeWord=strlen(queryWord);

            if(queryWord[sizeWord-1]=='\n')
            {
                queryWord[sizeWord-1]='\0';
                sizeWord--;
            }
        }

        fclose(file_dico);
    }

    return queryWord;

}

static inline char* get_queryWord_console()
{
    char *queryWord = (char*)malloc((MAX_SIZE_WORD+1)*sizeof(char));

    printf(" \n\n ========================================================  ");
    printf(" \n\n enter the query word :  ");
    gets(queryWord);

    // vider le buffer de scanf (le meme buff pour gets)
    /// empty_scanf_buffer();

    return queryWord;
}

static inline char* get_dicName_console()
{
    char *dic = (char*)malloc((MAX_SIZE_WORD+1)*sizeof(char));

    printf(" \n\n ========================================================  ");
    printf(" \n\n enter the dictionary name (path) :  ");
    gets(dic);

    // vider le buffer de scanf (le meme buff pour gets)
    /// empty_scanf_buffer();

    return dic;
}

static inline int is_continue()
{
    int is_test_mor=0;
    printf(" \n for exit enter 0 , otherwise too continue : ");
    scanf("%d",&is_test_mor);
    empty_scanf_buffer(); // vider le buffer de scanf

    return is_test_mor;
}

// beginning of the program
void start_mainConsol()
{

    // 1st phase : initialisation

    Options *my_option = new_options();

    // Build the tree and retrieve information necessary for the search
    char *source = get_dicName_console();

    initialize_AutoComplete(source,my_option);

    // 2en phase : approximate autocomplite
    while(true)
    {
        char* queryWord = get_queryWord_console();

        ListString* choices=get_results(queryWord);

        Print_ListString(choices); // display a list of suggestion


        // ----------------------------------------------
        if(!is_continue())
            break;
    }

    return;
}

#include "MainConsol.h"

static CGI_varlist *varlist=NULL;


static inline char* get_queryWord_cgi()
{
    varlist =  CGI_get_all();
    char *queryWord = (char*)CGI_lookup(varlist, "queryWord");
    return queryWord;
}


// --------------------------------------------------

// get option from env (cgi/fastcgi)
Options* get_option()
{
    Options *my_option = new_options();


        ///varlist=CGI_get_query(varlist);
    varlist =  CGI_get_all();
    my_option->TOPK_SIZE_GROUP= strtol(((char*)CGI_lookup(varlist, "SIZE_GROUP")), NULL, 10);

    return my_option;
}


// beginning of the program
void start_mainConsol()
{

    // 1st phase : initialisation
    Options *my_option = new_options();

    // Build the tree and retrieve information necessary for the search
    char *source = "dico.txt";

    initialize_AutoComplete(source,my_option);

    // 2en phase : approximate autocomplite
    /// while(true)
    {
        char* queryWord = get_queryWord_cgi();

        ListString* choices=get_results(queryWord);

        json_Print_ListString(choices); // display a list of suggestion

    }

    return;
}

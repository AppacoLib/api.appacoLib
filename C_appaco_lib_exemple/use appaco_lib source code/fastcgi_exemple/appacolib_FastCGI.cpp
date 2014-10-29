<<<<<<< .mine
#include <string>
#include <list>
#include <map>
#include <set>
#include <iostream>
#include <cstdlib>
#include <string.h>

using namespace std;

#include "cgicc/CgiDefs.h"
#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

#include "../appaco_lib/approximate_autocomplete.h" // la lib : appaco_Lib

using namespace cgicc;

#include "FCgiIO.h"

char *source = "towns.txt";  // la source pour construire notre index.

void init()
{
  
  // 1st phase : initialisation
    Options *my_option = new_options();
	
	/// ................................................................\\
	
	my_option->isExactSearchMethod = false; // approximate search
    my_option->typePrintResults = PRINT_WORD_LIST_TOP_K_byGroup; // 1) affichage selon le topk par groupe
	my_option->TOPK_SIZE_GROUP = 15; // nombre des mots dans un seul groupe est 10.
    
	my_option->topK_CHAR_SEPARATOR ='#'; // le char separtor de score topk

	/// my_option->words_SEPARATOR = '\n'; // on a pas cette option, car on a met un truc standrad, chaque mot doit etre dans une nouvelle ligne.
	
    /// my_option->doSort = true; // tu peut la mettre l'activer si le dictionaire ni pas ordonner
	

	/// ................................................................\\

    // Build the tree
    initialize_AutoComplete(source,my_option);

}

string mainurl;

void keymain(FCgiIO &IO, Cgicc &cgi)
{  
  const CgiEnvironment& env = cgi.getEnvironment();
  
  mainurl=cgi.getEnvironment().getScriptName(); 
  
  bool debug=false;

  string str=cgi("queryWord");

  IO << HTTPHTMLHeader();

  // ------------------------------------
  ListString* choices=get_results((char *)str.c_str());
  
  if(choices == NULL)
  {
	IO<< "[]";
	return;
  }
  
  int listSize = choices->size;

  if (listSize==0)
  {
	IO<< "[]";
	return;
  }

  listSize-=1;
	
	int i;
  
    IO<<"[";
    for(i =0; i<choices->size; i++)
    {
        IO<<"\"";
		IO<<choices->buffer[i];
		IO<<"\"";
		IO<<",";
    }
    IO<<"\""<<choices->buffer[i]<<"\"]"; // the last one
  // ------------------------------------
}

#if 0
pthread_mutex_t mainmutex;

void *main_core(void*)
{
  while(1) {

    //pthread_mutex_lock(&mainmutex);

    //core();
    
    //pthread_mutex_unlock(&mainmutex);

    usleep(1000*1000);
   
  }

  return NULL;
}
#endif

void main_treat(FCGX_Request &request)
{
  FCgiIO IO(request);
  Cgicc cgi(&IO);
  
  //pthread_mutex_lock(&mainmutex);
  
  keymain(IO,cgi);
  
  //pthread_mutex_unlock(&mainmutex);
  
  FCGX_Finish_r(&request);
}
    
void *main_request(void *)
{
  FCGX_Request request;

  FCGX_Init();
  FCGX_InitRequest(&request, 0, 0);

  while(FCGX_Accept_r(&request) == 0) {
    main_treat(request);
  }

  return NULL;
}


int main(void)
{
  init();

#if 0
  pthread_mutex_init(&mainmutex,NULL);

  pthread_t tc; //,tr;
  pthread_create(&tc,NULL,main_core,NULL);
  //pthread_create($tr,NULL,main_request,NULL);

  //pthread_t tdb; //,tr;
  //pthread_create(&tdb,NULL,main_checkdb,NULL);
#endif

  main_request(NULL);

  return 0;
}
=======
#include <string>
#include <list>
#include <map>
#include <set>
#include <iostream>
#include <cstdlib>
#include <string.h>

using namespace std;

#include "cgicc/CgiDefs.h"
#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

#include "../appaco_lib/approximate_autocomplete.h" // la lib : appaco_Lib

using namespace cgicc;

#include "FCgiIO.h"

char *source = "dico.txt";  // la source pour construire notre index.

void init()
{
  
  // 1st phase : initialisation
    Options *my_option = new_options();

    // Build the tree
    initialize_AutoComplete(source,my_option);

}

string mainurl;

void keymain(FCgiIO &IO, Cgicc &cgi)
{  
  const CgiEnvironment& env = cgi.getEnvironment();
  
  mainurl=cgi.getEnvironment().getScriptName(); 
  
  bool debug=false;

  string str=cgi("q");

  IO << HTTPHTMLHeader();

  // ------------------------------------
  ListString* choices=get_results((char *)str.c_str());
  
  if(choices == NULL) return;
  
  int listSize = choices->size;

  if (listSize==0) return;

  listSize-=1;
	
	int i;
  
    IO<<"[";
    for(i =0; i<choices->size-1; i++)
    {
        IO<<"\"";
		IO<<choices->buffer[i];
		IO<<"\"";
		IO<<",";
    }
    IO<<"\""<<choices->buffer[i]<<"\"]"; // the last one
  // ------------------------------------
}

#if 0
pthread_mutex_t mainmutex;

void *main_core(void*)
{
  while(1) {

    //pthread_mutex_lock(&mainmutex);

    //core();
    
    //pthread_mutex_unlock(&mainmutex);

    usleep(1000*1000);
   
  }

  return NULL;
}
#endif

void main_treat(FCGX_Request &request)
{
  FCgiIO IO(request);
  Cgicc cgi(&IO);
  
  //pthread_mutex_lock(&mainmutex);
  
  keymain(IO,cgi);
  
  //pthread_mutex_unlock(&mainmutex);
  
  FCGX_Finish_r(&request);
}
    
void *main_request(void *)
{
  FCGX_Request request;

  FCGX_Init();
  FCGX_InitRequest(&request, 0, 0);

  while(FCGX_Accept_r(&request) == 0) {
    main_treat(request);
  }

  return NULL;
}


int main(void)
{
  init();

#if 0
  pthread_mutex_init(&mainmutex,NULL);

  pthread_t tc; //,tr;
  pthread_create(&tc,NULL,main_core,NULL);
  //pthread_create($tr,NULL,main_request,NULL);

  //pthread_t tdb; //,tr;
  //pthread_create(&tdb,NULL,main_checkdb,NULL);
#endif

  main_request(NULL);

  return 0;
}
>>>>>>> .r22

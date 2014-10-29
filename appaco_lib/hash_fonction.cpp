#include "hash_fonction.h"


#ifndef __cplusplus
#define bool int
#define true 1
#define false 0
#endif

#define LOADFACTOR (0.7)   // load factor determine le porcentage de remplissage de tableau de hachage (0.7 donc le tableau sera remplé à 70%)
#define P (4294967291uL) // le P un nombre premier pour calculer la fonction de hachage
                         // sont type est "unsigned long", 32 bites.


static unsigned long randomeNB=0; // un nombre aleatoire, cette variable est accecible que dans cet fichier.
// la fonction de hashage est base sur ce numéro.
// dans le programe cette variable est inisialiser une seule fois.

static bool is_randomNB_initialise=false; // pour s'assorer que le nombre aleatoire est initialiser une seule fois.
// meme si on veus l'initialiser plusieur fois on ne peut pas.
static unsigned long *tab_pow_rand_nb=NULL; // un tableau qui va contenir les puissence de nombre aleatoire.


static int MAX_INDICE_POW;

static int initial_capacity=0;


// renvoie un nombre aleatoire compris entre 1 et P-1.
// telle que: P est un nombre premier definer dans le fichier "define_commun.h"
unsigned long long randomeNomber()
{
    time_t t; // une variable time; pour initialiser le generateur de pseudo aleatoire.

    unsigned long long aleatoireNomber=0;

    // initialise le generateur de nombre pseudo aléatoire.
    // avec le temps actuel de la machine.
    // la fonction time(&t) renvoie le temps de la machine.
    srand( (unsigned) time(&t));

    // la fonction rand() renvoie un nombre pseudo aléatoire
    // dans notre cas on prend pas la valeur 0
    // on cherche un nombre entre 1 et P-1.
    while(aleatoireNomber==0) // tant que le resultat est 0, on redemende un autre nombre aleatoire.
    {
        aleatoireNomber= rand() % (P-1);
    }

    return aleatoireNomber;

}


// cette foction calcule les puissance de nombre aleatoire
// et de la lettre speciale, et staucke les valeaurs dans 2 tableau
// pour que on peut les utiliser après sans refaire le calcule.
static void inline calculePuissanceNB()
{
    int i=0;// compteur.

    // tab_pow_rand_nb[0] = pow(randomeNB,0+1)
    tab_pow_rand_nb[0] = mod_P_multiply(randomeNB,1);


    for(i=1;i<MAX_INDICE_POW;i++)
    {
        // pow(nb,0)= 1, pow(nb,1)= nb, donc on commence pow(nb,1)
        // car pow(nb,0)= 1 ne va rein changer dans les calcule
        //
        // aussi la fonction de hachage est définer :
        // F(S)= Somme 1 vers |S| de  (S[i]*pow(rand,i)) mod p
        // mais on sais que S commence de 0 donc la fonction devien:
        // F(S)= Somme 0 vers |S|-1 de  (S[i]*pow(rand,i+1)) mod p
        //
        // cela est un choix, on peut commencer les puissance de "0".


        // tab_pow_rand_nb[i] = randomeNB*tab_pow_rand_nb[i-1] == pow(randomeNB,i+1)
        tab_pow_rand_nb[i] = mod_P_multiply(randomeNB,tab_pow_rand_nb[i-1]);
    }
}


void augmant_capacity(HashTable* my_hashTable)
{
    int i=0;

    char** old_buffer=my_hashTable->buffer;
    int size_old_buffer=my_hashTable->size_hashTable;

    my_hashTable->size_hashTable += (initial_capacity/LOADFACTOR);
    my_hashTable->percentage_loadFator = my_hashTable->size_hashTable*LOADFACTOR;
    my_hashTable->nb_element=0;

    my_hashTable->buffer=(char**)malloc(my_hashTable->size_hashTable*sizeof(char*));
    memset(my_hashTable->buffer,0,(my_hashTable->size_hashTable*sizeof(char*)));

    for(i=0;i<size_old_buffer;i++)
    {
        if(old_buffer[i]!=0)
        {
            hash_addItem(my_hashTable,old_buffer[i]);

            free(old_buffer[i]);
        }
    }

    free(old_buffer);

}



HashTable* initialisation_HF(int nbElement,int max_size_word)
{

    MAX_INDICE_POW = max_size_word+1;

    initial_capacity=nbElement;

    // initialise le nobre aleatoire dans le fichier calcule de hashage.
    // cette fonction doit etre apller en premier avant la fonction "calculePuissanceNB".
    if(!is_randomNB_initialise)
    {
        randomeNB=randomeNomber();

        is_randomNB_initialise=true;


        int i;
        // allocation de l'espace memoire pour le tableau "tab_pow_rand_nb", qui va contenir les puissence de nombre aleatoire
        tab_pow_rand_nb=(unsigned long *)malloc(MAX_INDICE_POW*sizeof(unsigned long));
        for(i=0;i<MAX_INDICE_POW;i++)
        {
            tab_pow_rand_nb[i]=0;
        }

        // cette fonction doit etre appler après la fonction "initialise_randomeNB".
        calculePuissanceNB();

    }


    HashTable* my_hashTable=(HashTable*)malloc(sizeof(HashTable));

    my_hashTable->size_hashTable = (initial_capacity/LOADFACTOR);
    my_hashTable->percentage_loadFator = my_hashTable->size_hashTable*LOADFACTOR;
    my_hashTable->nb_element =0;

    my_hashTable->buffer=(char**)malloc(my_hashTable->size_hashTable*sizeof(char*));
    memset(my_hashTable->buffer,0,(my_hashTable->size_hashTable*sizeof(char*)));


    return my_hashTable;
}



// calculer la valeur de hachage de mot "string"
// les puissance de nombre aleatoire sont prés calculer.
// la valeur retourner est de taille 8 octet.
//
// on utilise les puissance on commance de tab_puissance[1]
// pour que puisse dans le cas de deletion decaler 1 vers la gauche
// donc on commence avec  tab_puissance[0]
inline unsigned long hash(char *string, int sizeString)
{
    unsigned long hashValue=0;
    int i=0;

    for(i=0;i<sizeString;i++)
    {
        hashValue = mod_P_multiply_add(string[i],tab_pow_rand_nb[i+1],hashValue); // i+1, car dans la deltion on utilise juste 'i'
    }

    return hashValue;
}

void hash_addItem(HashTable *my_hashTable,char *string)
{
    int sizeString=strlen(string);
    // calculer la valeur de hachage de notre mot.
    unsigned long hashValue=hash(string,sizeString);

    if(my_hashTable->nb_element>=my_hashTable->percentage_loadFator)
        augmant_capacity(my_hashTable);

    int hashIndex= hashValue % my_hashTable->size_hashTable;


    // si la case est vide on insert le mot
    if( my_hashTable->buffer[hashIndex] ==0 )
    {

        my_hashTable->buffer[hashIndex]=(char *)malloc((sizeString+1)*sizeof(char));
        strcpy(my_hashTable->buffer[hashIndex],string);
    }
    else // sinon on cherche une case vide pour l'inserer.
    {
        while(true)
        {
            hashIndex++;
            if(hashIndex >= my_hashTable->size_hashTable)
            {
                hashIndex = 0;
            }

            if( my_hashTable->buffer[hashIndex] == 0)
            {
                my_hashTable->buffer[hashIndex]=(char *)malloc((sizeString+1)*sizeof(char));
                strcpy(my_hashTable->buffer[hashIndex],string);

                break;
            }
        }
    }

    my_hashTable->nb_element++;
}

// -1 if not exist
int hash_findItem(HashTable* my_hashTable,char *string)
{
    if(my_hashTable->size_hashTable==0) return -1;

    int sizeString=strlen(string);

    // calculer la valeur de hachage de notre mot.
    unsigned long hashValue=hash(string,sizeString);

    int hashIndex= hashValue % my_hashTable->size_hashTable;

    // si la case est vide donc pas de solution
    if( my_hashTable->buffer[hashIndex] == 0)
    {
        return -1;
    }
    else // dans le cas contraire, donc la case n'est pas vide on fait la comparaison jusqu'à une case vide.
    {
            // si la case est vide on insert le mot
        if( strcmp(my_hashTable->buffer[hashIndex],string)==0 )
        {
            return hashIndex;
        }
        else // sinon on cherche une case vide pour l'inserer.
        {
            while(true)
            {
                hashIndex++;
                if(hashIndex >= my_hashTable->size_hashTable) // car on commence de '0'
                {
                    hashIndex = 0;
                }

                if( my_hashTable->buffer[hashIndex] ==  0)
                {
                    return -1;
                }
                else
                {
                    if( strcmp(my_hashTable->buffer[hashIndex],string)==0 )
                    {
                        return hashIndex;
                    }
                }
            }
        }
    }


    return -1;
}



// lebirer l'espace memoire allouer dynamiquement
void hash_free_space(HashTable *my_hashTable)
{
    if(my_hashTable==NULL) return;

    int i=0;

    if(my_hashTable->buffer!=NULL)
    {
        for(i=0;i<my_hashTable->size_hashTable;i++)
        {
            free(my_hashTable->buffer[i]);
        }
    }

    free(my_hashTable->buffer);

    free(my_hashTable);
}



// ------------------------------------------------------------------------

void augmant_capacity_int(HashTable_int* my_hashTable)
{
    int i=0;

    int* old_buffer=my_hashTable->buffer;
    int size_old_buffer=my_hashTable->size_hashTable;

    my_hashTable->size_hashTable += (initial_capacity/LOADFACTOR);
    my_hashTable->percentage_loadFator = my_hashTable->size_hashTable*LOADFACTOR;
    my_hashTable->nb_element=0;

    my_hashTable->buffer=(int*)malloc(my_hashTable->size_hashTable*sizeof(int));
    memset(my_hashTable->buffer,0,(my_hashTable->size_hashTable*sizeof(int)));

    for(i=0;i<size_old_buffer;i++)
    {
        if(old_buffer[i]!=0)
        {
            hash_addItem_int(my_hashTable,old_buffer[i]);
        }
    }

    free(old_buffer);

}



HashTable_int* initialisation_HF_int(int nbElement)
{
    initial_capacity=nbElement;


    HashTable_int* my_hashTable=(HashTable_int*)malloc(sizeof(HashTable_int));

    my_hashTable->size_hashTable = (initial_capacity/LOADFACTOR);
    my_hashTable->percentage_loadFator = my_hashTable->size_hashTable*LOADFACTOR;
    my_hashTable->nb_element =0;

    my_hashTable->buffer=(int*)malloc(my_hashTable->size_hashTable*sizeof(int));
    memset(my_hashTable->buffer,0,(my_hashTable->size_hashTable*sizeof(int)));

    return my_hashTable;
}




void hash_addItem_int(HashTable_int *my_hashTable, int val)
{
    if(my_hashTable->nb_element>=my_hashTable->percentage_loadFator)
        augmant_capacity_int(my_hashTable);

    int hashIndex= val % my_hashTable->size_hashTable;


    // si la case est vide on insert le mot
    if( my_hashTable->buffer[hashIndex] ==0 )
    {
        my_hashTable->buffer[hashIndex]=val;
    }
    else // sinon on cherche une case vide pour l'inserer.
    {
        while(true)
        {
            hashIndex++;
            if(hashIndex >= my_hashTable->size_hashTable)
            {
                hashIndex = 0;
            }

            if( my_hashTable->buffer[hashIndex] == 0)
            {
                my_hashTable->buffer[hashIndex]=val;

                break;
            }
        }
    }

    my_hashTable->nb_element++;
}

// -1 if not exist
int hash_findItem_int(HashTable_int *my_hashTable, int val)
{
    if(my_hashTable->size_hashTable==0) return -1;

    int hashIndex= val % my_hashTable->size_hashTable;

    // si la case est vide donc pas de solution
    if( my_hashTable->buffer[hashIndex] == 0)
    {
        return -1;
    }
    else // dans le cas contraire, donc la case n'est pas vide on fait la comparaison jusqu'à une case vide.
    {
            // si la case est vide on insert le mot
        if( my_hashTable->buffer[hashIndex]==val )
        {
            return hashIndex;
        }
        else // sinon on cherche une case vide pour l'inserer.
        {
            while(true)
            {
                hashIndex++;
                if(hashIndex >= my_hashTable->size_hashTable) // car on commence de '0'
                {
                    hashIndex = 0;
                }

                if( my_hashTable->buffer[hashIndex] ==  0)
                {
                    return -1;
                }
                else
                {
                    if( my_hashTable->buffer[hashIndex]==val )
                    {
                        return hashIndex;
                    }
                }
            }
        }
    }


    return -1;
}



// lebirer l'espace memoire allouer dynamiquement
void hash_free_space_int(HashTable_int *my_hashTable)
{
    if(my_hashTable==NULL) return;

    free(my_hashTable->buffer);

    free(my_hashTable);
}




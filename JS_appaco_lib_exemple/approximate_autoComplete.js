
// search types, how to search query word in the trie
var SEARCH_WHOLE_WORD_EACH_TIME = 0; // search query word each time from root
var SEARCH_SUFFIX_BY_PREFIXE    = 1; // search the second query word from the node of the first query word, if the first is total prefix of the second, So it comes to search just the suffix added
var SEARCH_DELET_SUFFIX_ALL     = 2; // we start from the node where the two query word share a prefix, in the 2nd query we delete or add character

// display types, how to display the result
// var PRINT_SIMPLE_DIRECTLY          = 0; // direct display from the tri (from nodes).
var PRINT_WORD_LIST                = 1; // simple display, words are in the list
var PRINT_WORD_LIST_TOP_K          = 2; // words are ordered according to the top k, and they are displayed all at once
var PRINT_WORD_LIST_TOP_K_byGroup  = 3; // words are ordered according to the top k, and they are displayed group by group (withe "next" options)


var options=
{
    typeSearch:SEARCH_WHOLE_WORD_EACH_TIME,
    typePrintResults:PRINT_WORD_LIST_TOP_K_byGroup,
    isExactSearchMethod:false, // exact or approximate search with one error.

    topK_CHAR_SEPARATOR:'#',
    words_SEPARATOR:/[\n\r\t\0]+/g, // sa peut etre un char, string, ou regexp
    // words_SEPARATOR:'\r\n', // you must be vry carful, in windows we use \r\n tu retourn to new line
    
    doSort : true, // sort the dictionary or not, on disactive cette options si et seulement si le dic est ordonnie, sinon en risque de ne pas avoire des boone resultat, bon la construction sera dans un desourdre totale
    
    // _________________________________________________________________________
    
    TOPK_SIZE_GROUP : 8,  // the number of words that will be displayed at the same time according to (top-k) in the same group
    
    delay : 300, // The delay in milliseconds between when a keystroke occurs and when a search is performed.
    
    disabled: false, // Disables the autocomplete if set to true.
    
    minLength:1, // The minimum number of characters a user must type before a search is performed.
    
    // ibra : très très imporatant : +++++++++++++++++++++++++++++++++++++++++++
    // dans jQuery autocmpite : a chaque fois il dise que 
    // ( Zero is useful for local data with just a few items) dans le nombre min de char, 
    // nous dans notre cas on a pas ce probleme car on utiliseun trie, donc un seule char suffi pour faire la recherche
    // biensur après l'affichage doite etre avec topK (par groupe) sinon sa marche pas
    // bon j'ai une idée :) , meme si on utilise pas le topK on peut afficher les resultat par groupe
    // comme sa sa marche beaucoup mieu pour l'utilisaeur et pour le programe ;)
    
    sourceFromUrl:false,
    resultsFromUrl:false,
	
	score_update_unit:0, // l'unitée de mise a joure de score, si 0 donc pas de mise a joure
	
	item_ID_OnClick_nextResult:'undefined', // le id d'un item (buton,lien) pour l'utiliser pour get nextresult lorsque de l'evenement on click
	
	dataToSend:{SIZE_GROUP :10,minLength:2,delay:300}, // all types of datas (name/value) you want to sned to a server, this in case "resultsFromUrl=true", sa ci en+ de queryWord qui est automaticmemnt envoyer
	              // we have "queryWord" that's its automaticly send
				  
	ajaxRequestMethod :'GET' // method to use 'get or poste' to send data to server when 'resultsFromUrl=true'
};


// setOption  et getOption
function setOption(attribute,value)
{
    if(options[attribute])
        options[attribute]=value;
}

function getOption(attribute)
{
    if(options[attribute])
        return options[attribute];
}



function approximate_autoComplete(input_id,source,options_in)
{
	
// ------------------------------------------------------
if(typeof options_in !== 'undefined') // si options_in est defiiner, car on peut modifier les option derectemet sans les passer par paramettre
{
    // 1st inisilize some options by input user
    for(var options_item in options_in)
    {   
        if (typeof options[options_item] !== 'undefined') 
            options[options_item]=options_in[options_item];
    }
}	
// ------------------------------------------------------

	// si on fait appel a approximate_autocomplete une autre foi, donc on doit cacher les resltat afficher
	var hide_previeur_result_if_anathoer_call = document.getElementById('_ibra_results_id_to_use_only_if_we_have_another_call');
	if(hide_previeur_result_if_anathoer_call!==null)
		hide_previeur_result_if_anathoer_call.style.display = 'none';
	

    // ========================================================================\\
    // ----------------- define
    var NUM_PAR = 8,
        NUM_OCTETS_RESERVED_BY_NODE = 1, // une seule case dans JS  // free space added in the trie for each node (top K)
        trie_construit = false;

    var TrieInformation=
    {
        line_table:[],
        longest_line:0,
        limit_leaf:0,
        trie:[],
        first_node:0,
        Number_Bytes_Occupied_by_Trie:0
    };

    function exact_and_1err_Solution_nodes()
    {
        this.list_solution_1error=[];
        this.exact_solution=-1;      // -1 : mean we have no exact solution
    };
    
    
    // call the main function
///    _main(input_id,source,options_in);
_main(input_id,source);
    

    // ========================================================================\\
    // ----------------- main 

	// recupirer l'element HTML pour le lier avec l'evenemt onClick pour getNextresult
	var next_result_button;
	
	if (options.item_ID_OnClick_nextResult !== 'undefined')
	{
		next_result_button = document.getElementById(options.item_ID_OnClick_nextResult);
		next_result_button.onclick = getNext_Result; // lier l'evenement avec la fonction getNext_Result qui affiche les next result
	}
	
	
	
    var isExistNextResult = false; // pour virifier si il rest des result a afficher ou pas (rearque : les next resultat sont recupirer derectement depuis le trie a la demande)
	var choices; // contient les mots suggestion
	var pos_node_stop_searsh = -1; // le node solution pour la recherche exact.
	var list_solution_1error; // l'ensemble des node solution pour la recherche approuchée ( 1 node pour recherche exat et les autres pour l'approuchée)
	var isFirstCall; // verifier si ci la 1er appele de la fonction "xyz" pour ré-initialiser les var, si la 1er on reinitialze sinon non.
	
	var searchElement; // l'element HTML pour faire approximate auocomplete.
	var results; // pour crier un elemeent HTML pour afficher les resultat.
	
	var selectedResult = -1; // Permet de savoir quel résultat est sélectionné : -1 signifie « aucune sélection »
		

	// recupirer un ensemble de resultat depuit le Trie ensuit les afficher
	// cette fonction doit ete appler aprée _main qui contient "getResult"
    function getNext_Result()
	{
		if(isExistNextResult && options.typePrintResults===PRINT_WORD_LIST_TOP_K_byGroup)
		{
			// display the result
			if(options.isExactSearchMethod) // display results for exact search
			{
				choices =  wordList_TopK_byGroup(TrieInformation, pos_node_stop_searsh, options.TOPK_SIZE_GROUP, isFirstCall);

				displayResults(choices); // display a list of suggestion

				if(choices.length<options.TOPK_SIZE_GROUP) // this means that the words number in the list is unless of the number topk_element, so wholes nodes were treated
					isExistNextResult = false;
			}
			else //  display results for approximate search
			{
				choices = wordList_exact_1err_TopK_byGroup(TrieInformation,list_solution_1error,options.TOPK_SIZE_GROUP, isFirstCall);

				displayResults(choices);

				if(choices.length<options.TOPK_SIZE_GROUP) // this means that the words number in the list is unless of the number topk_element, so wholes nodes were treated
				isExistNextResult = false;

				console.log("in Next button onClick , isExistNextResult = "+isExistNextResult);
			}
		}
		
		selectedResult = -1; // On remet la sélection à zéro
		searchElement.focus();
	};    
 
    // Affiche les résultats d'une requête
	function displayResults(response) 
    { 
       
		console.log("\n displayResults response.length = %d \n",response.length);
		
        results.style.display = response.length ? 'block' : 'none'; // On cache le conteneur si on n'a pas de résultats
 
        if (response.length) // On ne modifie les résultats que si on en a obtenu
        { 
            var responseLen = response.length;
 
            results.innerHTML = ''; // On vide les résultats
 
            for (var i = 0, div; i < responseLen ; i++) 
            {
                div = results.appendChild(document.createElement('div'));
                div.innerHTML = response[i];
                 
                div.onclick = function() 
                {
                    chooseResult(this);
                };
            }
        }
    }
    
    // Choisit un des résultats d'une requête et gère tout ce qui y est attaché
	function chooseResult(result) 
    { 
       
        searchElement.value = result.innerHTML; // On change le contenu du champ de recherche et on enregistre en tant que précédente valeur
        results.style.display = 'none'; // On cache les résultats
        result.className = ''; // On supprime l'effet de focus
        
		
        /////////////////////////////////
        // faire la modification pour le scoure de string choiser
        /// if( options.score_update_unit>=0 && (options.typePrintResults === PRINT_WORD_LIST_TOP_K || options.typePrintResults === PRINT_WORD_LIST_TOP_K_byGroup))
		if( options.score_update_unit>0 )
        {
            /// console.log(" listStringInfo.length = "+listStringInfo.length+" , selectedResult = "+selectedResult+" , "+result.innerHTML);
            /// topK_updating_AllWayNodes_bySearching(selectedResult, TrieInformation, listStringInfo);
            console.log(" selectedResult = "+selectedResult+" , result.innerHTML == "+result.innerHTML);
            topK_updating_AllWayNodes_bySearching(result.innerHTML, TrieInformation);
        }
        /////////////////////////////////
		
        
        selectedResult = -1; // On remet la sélection à zéro
        searchElement.focus(); // Si le résultat a été choisi par le biais d'un clic, alors le focus est perdu, donc on le réattribue
    }
 
 		

/// function _main(input_id,source,options_in)
function _main(input_id,source)
{
    if(!options.resultsFromUrl) // si les resulta ne vien pas de server donc on construit le dic en local
    {
        initialize_AutoComplete(source,options);
    }
    else
    {
        trie_construit= true; // les resltat vien de server, donc on met sa a true
    }
    
	
    var previousRequest; // On stocke notre précédente requête AJAX dans cette variable
	var lastSitring="";
	var lengPrefixCommon = 0;
	var my_interval_search;

    // =====================================================================
 
    //2nd get inpute element
	searchElement = document.getElementById(input_id);
    searchElement.setAttribute( "autocomplete", "off" );
	searchElement.value='';
    
    // 3rd Create results
    results = document.createElement("div");
	results.id = "_ibra_results_id_to_use_only_if_we_have_another_call"; // ji met le id d'une façon exprit très long pour eviter la collistion des id smilaire utiliser par les autres develeppeur, comme le nom indique on utilise ce id pour cacher les resultat en cas où on refait une autre appelle à approximtae_autocomplete.
	results.className = "results";
    
    var pos = getOffset(searchElement);
    results.style.top = pos.top+ searchElement.offsetHeight+"px"; 
    results.style.left = pos.left+"px";
    results.style.width=(searchElement.offsetWidth -2)+"px"; // -2 de border (1 left et 1 rigth)
    // Add to body element
    document.body.appendChild(results);
    
    // Notre fonction qui calcule le positionnement complet
    function getOffset(element) 
    {
        var top = 0, left = 0;
    
        do 
        {
            top += element.offsetTop;
            left += element.offsetLeft;
        } while (element = element.offsetParent);
    
        return { top: top, left: left };
    };
    
    // récupère les résultats depuit un trie local
	function getResults(queryWord) 
    { 
        console.log(" getResults : queryWord = "+queryWord);
        
        if(queryWord.length<options.minLength)
        {
            results.style.display = 'none'; // On cache le conteneur si on n'a pas de résultats
    		
			lastSitring = ""; // re-initialiser
           
           return;
        }

        if(queryWord===lastSitring) return; // on laisse la meme resulat qui es afficher
        
        console.log(" lastSitring = "+lastSitring+", queryWord = "+queryWord);
        lengPrefixCommon = utf8_char_len_pgpc(lastSitring,queryWord);
        console.log("lengPrefixCommun = "+lengPrefixCommon);
        
		// juste pour debougage pour voire le temps d'execution
        var startTime =0,
            elapsedTime =0,  
            endTime =0;
    
        startTime = new Date().getTime(); 
        
        // 1 : research, look up for the queryWord
        if(options.isExactSearchMethod) // exact search
        {
            if(options.typeSearch===SEARCH_WHOLE_WORD_EACH_TIME) // search the whole word each times
            {
                pos_node_stop_searsh = searchQueryWord(queryWord,TrieInformation);
            }
            else
                if(options.typeSearch===SEARCH_DELET_SUFFIX_ALL) // search, supprme suffixe, add suffixe, (all combinison), there's prefixe commun beteween the previeus queryWord and the present queryWord
                {
                    console.log("avant l'appel lengPrefixCommon = "+lengPrefixCommon);
                    pos_node_stop_searsh = search_Prefix_by_Suffix_Delete(queryWord,lengPrefixCommon,TrieInformation);
                }
                else  // search the word, but we begin in the end of the befor word if it's a suffixe, we search the just the suffixe
                {
                    pos_node_stop_searsh = searchSuffixbyPrefix(queryWord,lengPrefixCommon,TrieInformation);
                }
        }
        else // approximate search with one error
        {
            if(options.typeSearch===SEARCH_WHOLE_WORD_EACH_TIME)
            {
                list_solution_1error = searchQueryWord_1error(queryWord,TrieInformation);
                
               // alert(list_solution_1error);
            }
            else
                if(options.typeSearch===SEARCH_DELET_SUFFIX_ALL)
                {
                    list_solution_1error = search_Prefix_by_Suffix_Delete_1error(queryWord, lengPrefixCommon,TrieInformation);
                }
                else
                {
                    list_solution_1error = searchSuffixbyPrefix_1error(queryWord, lengPrefixCommon,TrieInformation);
                }
        }
        
        endTime = new Date().getTime(); 
		elapsedTime = endTime - startTime;
        
        console.log(" search Time = "+elapsedTime);
        
        console.log(" pos_node_stop_searsh = %d",pos_node_stop_searsh);
        
        startTime = new Date().getTime(); 
        
        // display the result
        if(options.isExactSearchMethod) // display results for exact search
        {
            if(pos_node_stop_searsh!==-1)
            {
                    if(options.typePrintResults===PRINT_WORD_LIST)
                    {
                        choices = wordList(TrieInformation,pos_node_stop_searsh); // get a list of words to display

                        displayResults(choices); // display a list of suggestion

                    }
                    else
                    if(options.typePrintResults===PRINT_WORD_LIST_TOP_K_byGroup)
                    {
                        isFirstCall = true;
                        isExistNextResult = true;

                            choices =  wordList_TopK_byGroup(TrieInformation, pos_node_stop_searsh, options.TOPK_SIZE_GROUP, isFirstCall);
                            
                            endTime = new Date().getTime(); 
                            elapsedTime = endTime - startTime;
                            console.log(" wordList_TopK_byGroup Time = "+elapsedTime);

                            displayResults(choices); // display a list of suggestion

                            if(choices.length<options.TOPK_SIZE_GROUP) // this means that the words number in the list is unless of the number topk_element, so wholes nodes were treated
                                isExistNextResult = false;

                            isFirstCall=false;
                    }
                    else // "if(typePrintResults==PRINT_WORD_LIST_TOP_K)", print all results at once, words are ordered by topK
                    {   
                         choices = wordList_TopK(TrieInformation,pos_node_stop_searsh);

                        displayResults(choices); // display a list of suggestion
                    }
            }
            else
            {
                // on fait sa pour cachi le resultat presedent
                choices=[];
                displayResults(choices);
                console.log("\n no solution ");
            } 
        }
        else //  display results for approximate search
        {
            if(list_solution_1error.exact_solution!==-1 || list_solution_1error.list_solution_1error.length!==0)
            {
                /// alert(" exact : "+list_solution_1error.exact_solution+"\n err : \n"+list_solution_1error.list_solution_1error);
                
                if(options.typePrintResults===PRINT_WORD_LIST)
                {
                    choices = wordListe_exact_1err(TrieInformation, list_solution_1error);

                    displayResults(choices);

                }
                else
                    if(options.typePrintResults===PRINT_WORD_LIST_TOP_K_byGroup)
                    {
                        isFirstCall = true;
                        isExistNextResult = true;
                  
                            choices = wordList_exact_1err_TopK_byGroup(TrieInformation,list_solution_1error,options.TOPK_SIZE_GROUP, isFirstCall);
                            
                            endTime = new Date().getTime(); 
                            elapsedTime = endTime - startTime;
                            console.log(" wordList_exact_1err_TopK_byGroup Time = "+elapsedTime+" nb mot sol = "+choices.length);

                            displayResults(choices);

                            if(choices.length<options.TOPK_SIZE_GROUP) // this means that the words number in the list is unless of the number topk_element, so wholes nodes were treated
                                isExistNextResult = false;

                            isFirstCall=false;
                            
                            console.log("in keyUp , isExistNextResult = "+isExistNextResult);
                    }
                    else
                    {                        
                        choices = wordList_exact_1err_TopK(TrieInformation,list_solution_1error);

                        displayResults(choices);
                    }
            }
            else
            {
                // on fait sa pour cachi le resultat presedent
                choices=[];
                displayResults(choices);
                console.log("\n no solution ");
            }
        }

        
        
        lastSitring = queryWord;
    }
    
    function getResults_fromServer(queryWord) // récupère les résultats depuit un server
    {
        if(queryWord.length<options.minLength)
        {
            results.style.display = 'none'; // On cache le conteneur si on n'a pas de résultats      
           return;
        }

        if(queryWord===lastSitring) return; // on laisse la meme resulat qui es afficher
        
        if (previousRequest && previousRequest.readyState < 4) 
        {
            previousRequest.abort(); // Si on a toujours une requête en cours, on l'arrête
        }
        
        var xhr = new XMLHttpRequest();
		
		xhr.onreadystatechange = function() 
        {
            if (xhr.readyState === 4 && xhr.status === 200) 
            {     
                choices = JSON.parse(xhr.responseText); // recupier  et parser le resultat
                displayResults(choices);
            }
        };
        
		var my_url = source;
		var my_url_data = 'queryWord='+ encodeURIComponent(queryWord);
		
		// add all data to my_url
		for(var data_item in options.dataToSend)
		{   
			my_url_data+= '&'+data_item+'='+encodeURIComponent(options.dataToSend[data_item]);
		}
        
		if(options.ajaxRequestMethod==='GET')
		{
			my_url+='?'+my_url_data;
			
			xhr.open('GET', my_url);
			xhr.send(null);
		}
		else // must be 'POST'
		{
			xhr.open('POST', my_url);
			xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
			xhr.send(my_url_data);
		}
 
        previousRequest = xhr;
        lastSitring = queryWord;
    }
 
    searchElement.onkeyup = function(e) 
    {        
        if(trie_construit===false) //  je doit bien specifier le msg d'erreur selon le type de source
        {
            /// alert(" you should build the dictionary, click on choose file "); // dans le cas source from local file
			alert(" you should build the dictionary first");
			
            return false;
        }

        e = e || window.event; // IE compatibility
 
        var divs = results.getElementsByTagName('div');
 
        if (e.keyCode === 38 && selectedResult > -1) // up
        { 
            divs[selectedResult--].className = '';
             
            if (selectedResult > -1) // avoids modifying childNodes [-1].
            { 
                divs[selectedResult].className = 'result_focus';
            }
        }
        else if (e.keyCode === 40 && selectedResult < divs.length - 1) // down
        { 
           
            results.style.display = 'block'; //display the résults
 
            if (selectedResult > -1) // avoids modifying childNodes [-1].
			{ 
                divs[selectedResult].className = '';
            }
 
            divs[++selectedResult].className = 'result_focus';
 
        } 
        else if (e.keyCode === 13 && selectedResult > -1) //  entry
        { 
            chooseResult(divs[selectedResult]);
        }
		else if (e.ctrlKey && e.keyCode === 39 ) // ctrl + Right arrow
        { 
            getNext_Result();
        }
        else
        {
            clearTimeout(my_interval_search); // Stop the preveius Execution
            
            my_interval_search = setTimeout(function()
            {
                if(options.resultsFromUrl)
                {
                        getResults_fromServer(searchElement.value);
                }
                else
                {
                    getResults(searchElement.value);
                }
            }
            ,options.delay);
            
            selectedResult = -1; // On remet la sélection à zéro à chaque caractère écrit
        }
 
    };

    // calculate the length of greatest common prefix (the chars are in utf-8)
 // cette def exist dans le fichier : trie_coding, donc si encapsule le fichier donc on doit la defini ici ;)
function utf8_char_len_pgpc(str1,str2)
{
    var i=0,
        length = (str1.length < str2.length)? str1.length : str2.length;
    
    while(i<length  && str1.charAt(i)=== str2.charAt(i)) i++;
    
    return i;
};

}


// ========================================================================\\
// ----------------- trie_coding

var result_fromURL; // we use this as buffer to containe the result from ajax result
// we put this variable in this scop to avoid the browser pbm, (he desplay a messag error when the page take a lot off time to execut and not finich)

function initialize_AutoComplete(source,options)
{	
    var TextInformation=
    {
         text:"",//a memory area that will contain all our dictionary file.
         size_text :0, // in javascript size_text = nb of char (utf8 == 1 char)
         nb_line:0,//the number of line

         longest_line:0,//the longest line (the longest word in the dictionary)
         line_table:[]//a table will point to the start address of each line in the memory (the table contains the address of the beginning of each line in the text)
    };


    var Construct_Trie=
    {
        text_info:{},

        topK_tab:[], // a table that will contain the score value for each word from our dictionary
                            // Note: the score is at the end of each dictionary word with a separator '#'
                            // example  : abcde#05
                            //            abcguip#890

        long_pref_line_next:[],  //a table that will contain the length of common prefix between line (i) and (i+1)
        file_last_nodes:[], // a table which acts as a stack,
        file_last_nodes_counter:0,
        linking_nodes:[], // file of linking each node of same depth in the trie
                         // Each box will contain the last node number of the same depth.

        nodes:[],     // a table that will contain the information of nodes,a node consists of a block of 8 successive memory box.
        last_node:0,   // indicate the position of the last nodes inserted into the table

        pos_code:0, // it just to advance in tree, byte by byte
        limit_leaf:0,    // end of the zone devoted to the leaves
        local_size:0, // the size of a single nodes

        trie:[],
        size_trie:0,
        first_node:0,

        Number_Bytes_Occupied_by_Trie:0
    };

// ========================================================================== \\
// tester le type de source pour appeler la bonne fonction

    // 1) string (from file, wela string simple, indiquer le separateur des mots)
    // 2) array  (simple ou avec topk inclu dans le mot lui meme)
    // 3) array : objet (label,value)
    // 4) array : array (0:string, 1:value)
    if(toType(source)==="string")
    {	
        if(options.sourceFromUrl) // recupirer le fichier depuit un serveur
        {
            /// alert (options.sourceFromUrl);
            // =================================================
            var xhr = new XMLHttpRequest();

            // On souhaite juste récupérer le contenu du fichier, la méthode GET suffit amplement :
            xhr.open('GET', source);

            xhr.onreadystatechange = function() // On gère ici une requête asynchrone
            { 
                    if (xhr.readyState === 4 && xhr.status === 200) // Si le fichier est chargé sans erreur
                    {
                        /// alert(xhr.responseText);
						result_fromURL = xhr.responseText;
                        trie_coding_fromString(result_fromURL,options);
                    }
                    else if(xhr.readyState === 4 && xhr.status !== 200) // En cas d'erreur !
                    { 
                            alert('Une erreur est survenue !\n\nCode :' + xhr.status + '\nTexte : ' + xhr.statusText);
                    }
            };

            xhr.send(null); // La requête est prête, on envoie tout !
            // =================================================
        }
        else
        {
            /// alert (" else : "+options.sourceFromUrl);
            trie_coding_fromString(source,options);
        }
    }
    else if(toType(source)==="array")
    {
        if(toType(source[0])==="string")
        {
            trie_coding_fromArray(source,options);
        }
        else if(toType(source[0])==="object")
        { // là je ne sais pas si on doit verifier que notre objet contien precisiment : label et value ou non
           // if(toType(source[0].label)==="string" && toType(source[0].value)==="number")
            trie_coding_fromTabObject(source,options);
        }
        else if(toType(source[0])==="array")
        { // là aussi meme chose pour source[0][0], et source[0][1] : string, number
            // if(toType(source[0][0])==="string" && toType(source[0][1])==="number")
            trie_coding_fromArray_OfArray(source,options);
        }
    }


// ========================================================================== \\

	
    function toType(obj) 
    {
        return ({}).toString.call(obj).match(/\s([a-zA-Z]+)/)[1].toLowerCase();
    };


    function trie_coding()
    {
        calculLongestPrefixLines(Construct_Trie);

        nodesIdentify(Construct_Trie);

        spaceDeterminingForTrie(Construct_Trie);

        fillingTrie(Construct_Trie);

        maxTopK_update(Construct_Trie.first_node,Construct_Trie); // we start from the root node, then the update is done in a recursive way

        /// printingWholeTrie(Construct_Trie.text_info.line_table, Construct_Trie.text_info.longest_line, Construct_Trie.trie, Construct_Trie.first_node, Construct_Trie.limit_leaf, 1); // 1 : num_octet_by_node

        TrieInformation.first_node = Construct_Trie.first_node;
        TrieInformation.limit_leaf = Construct_Trie.limit_leaf;
        TrieInformation.longest_line=Construct_Trie.text_info.longest_line;
        TrieInformation.line_table = Construct_Trie.text_info.line_table;
        TrieInformation.trie=Construct_Trie.trie;
        /// TrieInformation.Number_Bytes_Occupied_by_Trie = Construct_Trie.Number_Bytes_Occupied_by_Trie;

        trie_construit = true;
        /// return TrieInformation;

        alert(" trie_construit == true ");
    };

    // build the Trie from an input file that contains the dictionary
    // return a data structure that contains all Trie information
    function trie_coding_fromString(source,options)
    {     
        TextInformation.text = source;
        TextInformation.size_text=source.lenght;

        cuttingTextIntoLines(TextInformation,options.words_SEPARATOR); 

        get_nb_line_longest_line(TextInformation);

        // le trie prend bueaucoup de temps, donc on propose sa comme options au utilisature
        if(options.doSort)
        {
            TextInformation.line_table.sort(function (a, b) {
                    return a.localeCompare(b);
                });
        }

        Construct_Trie.text_info = TextInformation; 
        get_TopK_fromLines(Construct_Trie,options.topK_CHAR_SEPARATOR);

        trie_coding();
    };


    // build the Trie from an input file that contains the dictionary
    // return a data structure that contains all Trie information
    function trie_coding_fromArray(source,options)
    {        
        /// alert(" we are trie_coding_fromArray ");

        TextInformation.line_table = source;

        get_nb_line_longest_line(TextInformation);

        if(options.doSort)
        {
            TextInformation.line_table.sort(function (a, b) {
                    return a.localeCompare(b);
                    });
        }

        Construct_Trie.text_info = TextInformation;

        if(!options.topK_CHAR_SEPARATOR)
        {
            initialize_topK_tab(Construct_Trie);
        }
        else
        {
            console.log(" CHAR_SEPARATOR is defined donc : get_TopK_fromLines ");
            get_TopK_fromLines(Construct_Trie,CHAR_SEPARATOR);
        }

        trie_coding();
    };


    // build the Trie from an input file that contains the dictionary
    // return a data structure that contains all Trie information
    function trie_coding_fromTabObject(source,options)
    {        
        /// alert(" we are from tab Object ");

        // 1st : sort the table of object, trie le tableau (qui contien et les mot et en meme temps leurs topK)
        if(options.doSort)
        {
            source.sort(function (a, b) {
                        return a.label.localeCompare(b.label);
                    });
        }

        // 2nd : recupirere les mot et leur topk dans des deferenet tableau
        TextInformation.line_table = new Array(source.length);
        Construct_Trie.topK_tab = new Array(source.length);
        for(var i=0; i<source.length;i++)
        {
            TextInformation.line_table[i] = source[i].label;
            Construct_Trie.topK_tab[i] = source[i].value;
        }

        get_nb_line_longest_line(TextInformation);

        Construct_Trie.text_info = TextInformation;

        trie_coding();
    };


    function trie_coding_fromArray_OfArray(source,options)
    {        
        /// alert(" we are from Array_OfArray ");

        // fisrt : sort the table of object, trie le tableau (qui contien et les mot et en meme temps leurs topK)
        if(options.doSort)
        {
            source.sort(function (a, b) {
                    return a[0].localeCompare(b[0]);
                });
        }

        // 2nd : recupirere les mot et leur topk dans des deferenet tableau
        TextInformation.line_table = new Array(source.length);
        Construct_Trie.topK_tab = new Array(source.length);
        for(var i=0; i<source.length;i++)
        {
            TextInformation.line_table[i] = source[i][0];
            Construct_Trie.topK_tab[i] = source[i][1];
        }

         get_nb_line_longest_line(TextInformation);

         Construct_Trie.text_info = TextInformation;

        trie_coding();
    };


    // count the number of lines, including a fake char O as newline
    // Note: if you use the strlen function, we'll miss the 0 as a line, because for it 0 considered as an end of string
    function utf8_nb_line_text(text, length)
    {
        var i=0,
            nl=0;

        while (i<length)
        {
            if (
                    (text.charAt(i)==='\n') ||
                    (text.charAt(i)==='\r') ||
                    (text.charAt(i)==='\0')
               )
            {
                nl++;
            }

            i++;
        }

        return nl;
    };



    // cutting the overall text to lines,
    // each line is pointed by a memory box in "line_table"
    function cuttingTextIntoLines(text_info,regExp)
    {
        text_info.text=text_info.text.trim();
        // text_info.text.replace(/^[\n\t\0\s]+|[\n\t\0\s]+$/g, '');

        //text_info.line_table = text_info.text.split(/[\n\r\t\0]+/g);
        text_info.line_table = text_info.text.split(regExp);
    };


    function get_nb_line_longest_line(text_info)
    {
        text_info.longest_line =0;

        text_info.nb_line = text_info.line_table.length;

        for(i=0;i<text_info.nb_line;i++)
        {
            if(text_info.line_table[i].length>text_info.longest_line)
            {
                text_info.longest_line = text_info.line_table[i].length;
            }
        }
    };


    function initialize_topK_tab(trie_build)
    {
        trie_build.topK_tab = new Array(trie_build.text_info.nb_line);

        for(var i=0; i<trie_build.text_info.nb_line;i++)
        {
            trie_build.topK_tab[i] = 0;
        }
    };


    // retrieve TopK score from each line, and put the result in "topK_tab".
    // values ??are at the end of words with a separator, example: abcd#34
    // when we recover the number, we replace it with 0 at the memory zone(to get a new end of line)
    // Consequently, we must recalculate the longest line
    function get_TopK_fromLines(trie_build,CHAR_SEPARATOR)
    {

        var regx =new RegExp(''+CHAR_SEPARATOR+'[0-9]+$'),
            k=0,
            str_lenght=0,
            pos_separator;

        trie_build.text_info.longest_line =0;

        initialize_topK_tab(trie_build);

        for(k=0; k<trie_build.text_info.nb_line; k++)
        {    
            // go to the separateur position
            pos_separator = trie_build.text_info.line_table[k].search(regx);

            if(pos_separator >0)
            {
                trie_build.topK_tab[k] = ( parseInt(trie_build.text_info.line_table[k].substring(pos_separator+1),10) );
                trie_build.text_info.line_table[k] = trie_build.text_info.line_table[k].substring(0,pos_separator);
            }
            // in the case where no separator exist, so we have a default  Value '0'

            // check the longest line
            str_lenght = trie_build.text_info.line_table[k].length ;
            if(str_lenght > trie_build.text_info.longest_line)
            {
                trie_build.text_info.longest_line = str_lenght;
            }
        }
    };


    // calculate the length of common prefix between the line i and i +1
    function calculLongestPrefixLines(trie_build)
    {
        for (var k=0;k<trie_build.text_info.nb_line-1;k++)
        {
            trie_build.long_pref_line_next.push( utf8_char_len_pgpc(trie_build.text_info.line_table[k],trie_build.text_info.line_table[k+1]));
        }
    };


    // calculate the length of greatest common prefix (the chars are in utf-8)
    function utf8_char_len_pgpc(str1,str2)
    {
        var i=0,
            length = (str1.length < str2.length)? str1.length : str2.length;

        while(i<length  && str1.charAt(i)=== str2.charAt(i)) i++;

        return i;
    };


    // first steps: identifying necessary nodes to build our compact trie
    function nodesIdentify(trie_build)
    {
        var k=0,
            node_tmp=0,
            node_pointed=0, // in the middle of a transition
            node_father=0;

        // file of the last nodes in the trie
        trie_build.file_last_nodes = new Array(trie_build.text_info.longest_line);
        trie_build.file_last_nodes_counter = 0;

        // file of linking each node of same depth in the trie
        // Each box will contain the last node number of the same depth.
        trie_build.linking_nodes = new Array(1+trie_build.text_info.longest_line);
        for (k=0;k<=trie_build.text_info.longest_line;k++)
        {
            trie_build.linking_nodes[k]=-1;// initialization
        }


        // a table that will contain the information of nodes,a node consists of a block of 8 successive memory box.
        trie_build.nodes = new Array(trie_build.text_info.nb_line*NUM_PAR*2+2*NUM_PAR);

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
        trie_build.last_node = -NUM_PAR;


        // insert the 2 first nodes,(2 first nodes of the trie, root and first transition for the first line).

        // 1st nodes is the root
        add_node(trie_build,1,0,0,0,0,-1);

        // node for the 1st line
        add_node(trie_build,0,0,trie_build.text_info.line_table[0].length,0,0,0);


        trie_build.file_last_nodes[0] =0;       // the first position of nodes (which is the root) is "0"
        trie_build.file_last_nodes[1] =NUM_PAR; // the position of 2nd node is "NUM_PAR == 8", because each node occupy 8 byte
        trie_build.file_last_nodes_counter=1;


        // treat others lines
        // for each line we have a leaf node
        // if we have common prefix between two line, we create intermediary node
        for (k=1;k<trie_build.text_info.nb_line;k++)
        {
            node_tmp= trie_build.file_last_nodes[trie_build.file_last_nodes_counter]; // retrieve the position of the last node inserted

            // check if we have the same depth (common prefix) between the line k-1 and K
            // '=='    that's means we are in this case    {1)abc,    2)abc }
            // '>'     that's means we are in this case    {1)abc,    2)abcd }
            if (trie_build.long_pref_line_next[k-1]>= trie_build.nodes[node_tmp+2])
            {
                // same depth or longer

                if (trie_build.text_info.line_table[k].length===trie_build.long_pref_line_next[k-1]) // in this case 1)abc, 2)abc, we do nothing
                {
                    console.log("Lines %d and %d are identical !\n", trie_build.nodes[node_tmp+3],k);
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
                    node_tmp = trie_build.file_last_nodes[--trie_build.file_last_nodes_counter];

                    // N1 : pointed by N0
                    node_pointed = trie_build.file_last_nodes[trie_build.file_last_nodes_counter+1];

                    // adding a new node (an intermediate node N2), the current line correspond to a node
                    add_node(trie_build,2,0,trie_build.long_pref_line_next[k-1],k,0,node_tmp);

                    node_father =trie_build.last_node;// N2

                    trie_build.nodes[node_pointed+5]=trie_build.last_node;   // new father for N1,(update)

                    trie_build.file_last_nodes[++trie_build.file_last_nodes_counter]=trie_build.last_node; // added in the stack

                    // adding a new leaf for the line (N3)
                    add_node(trie_build,0,0,trie_build.text_info.line_table[k].length,k,0,node_father);

                    trie_build.file_last_nodes[++trie_build.file_last_nodes_counter]=trie_build.last_node; // added in the stack
                }
            }
            else // we are in this case : example a: 1)abcde  2)abcxyz  or example b:  1)abcde  2)abcxyz 3)abzz
            {
                // return backwards to find the node of common prefix (example b, insert the 3rd word)
                while (trie_build.long_pref_line_next[k-1]< trie_build.nodes[node_tmp+2] )
                {
                    node_tmp= trie_build.file_last_nodes[--trie_build.file_last_nodes_counter];
                }


                // Here we divide the transition, create an intermediary node and then insert the leaf node
                if (trie_build.long_pref_line_next[k-1]> trie_build.nodes[node_tmp+2])
                {
                    // in the middle of a transition
                    node_pointed = trie_build.file_last_nodes[trie_build.file_last_nodes_counter+1];  // node pointed by the old transition

                    add_node(trie_build,2,0,trie_build.long_pref_line_next[k-1],k,0,node_tmp);

                    node_father =trie_build.last_node;

                    trie_build.nodes[node_pointed+5]=trie_build.last_node;

                    trie_build.file_last_nodes[++trie_build.file_last_nodes_counter]=trie_build.last_node;

                }
                else    // here we have a prefix of the same depth as that of father (or grand father), we insert directly the leaf node
                {
                    trie_build.nodes[node_tmp]++;   // one more transition
                    node_father =node_tmp;

                }

                // adding a new leaf for the line
                add_node(trie_build,0,0,trie_build.text_info.line_table[k].length,k,0,node_father);

                trie_build.file_last_nodes[++trie_build.file_last_nodes_counter]=trie_build.last_node; // added in the stack
            }
        }
    };


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
    //function modifies_node(int * linking, int * ntable,int * lnode, int nb_trans, int tot_8, int depth,int seq_num, int tot_trans, int p_father)
    function modifies_node(trie_build, nb_trans, tot_8, depth, seq_num, tot_trans, p_father)
    {
        trie_build.nodes[trie_build.last_node]=nb_trans; // it's the same ntable[*lnode+0]=nb_trans, we are in the first memory box
        trie_build.nodes[trie_build.last_node+1]=tot_8;
        trie_build.nodes[trie_build.last_node+2]=depth;
        trie_build.nodes[trie_build.last_node+3]=seq_num;
        trie_build.nodes[trie_build.last_node+4]=tot_trans;
        trie_build.nodes[trie_build.last_node+5]=p_father;
        trie_build.nodes[trie_build.last_node+6]=trie_build.linking_nodes[depth];
        trie_build.nodes[trie_build.last_node+7]=-1;             //  pointer to its place in the (future) reserved memory.

        trie_build.linking_nodes[depth]=trie_build.last_node;  // The last node number of the same depth
    };

    // adds a node in the table of nodes "ntable"
    function add_node(trie_build, nb_trans, tot_8, depth,seq_num, tot_trans,p_father)
    {
        trie_build.last_node+=NUM_PAR; // (last_node) must proceed with "NUM_PAR==8" position.

        modifies_node(trie_build,nb_trans,tot_8,depth,seq_num,tot_trans,p_father);
    };


    // SECOND PHASE: DETERMINING THE SPACE REQUIRED BY THE TRIE

    // calculer l'espace memoire nécessaire pour chaque noeud.
    // set the address of each node in the memory according to the size of previous nodes.
    function spaceDeterminingForTrie(trie_build)
    {
        var father=0,
            k=0,
            node_tmp=0;


    // tous cela va etre changer, car la on des tableau array qui contient n'import uelle valeur avec n'importte quelle type
    // donc a chaque foi que je aurai une donnée a stoket je reserve une case, donc ci juste +1

        // coding the leaves in a reserved space, (and make update to the father and leaf node)

        for (k=0; k<=trie_build.last_node; k+=NUM_PAR)
        {
            if (trie_build.nodes[k]===0) // a leaf
            {
                /*
                trie_build.local_size=1; // free space added for TopK
                trie_build.local_size+=1; // number of the line k.
                */
               // the same :
                trie_build.local_size=2; // 1: free space added for TopK , 1: number of the line k.

                if (trie_build.nodes[k+5]>-1) // if the node have a father
                {
                    // adding the size of coding the (transition to the node) to (the size required) to code this node

                    father=trie_build.nodes[k+5];

                    trie_build.nodes[father+4] +=3; // 1: pour code la profendeur de father pour que on trouve le debut de caratère transtion de fils
                                                   // exemple : 1)abcd, 2)abdd pr trouver dd on a le num de sequence qui est i=2, et le profndeur de node
                                                   // father (ab) qui est j=2 "trie_build->nodes[father+2]", donc on peut accider line_table[i].charAt(j)
                                                   // donc la on est obliger de sauvgardi les numero de chaque sequence
                                                   // ci pas comme en c on sauvgarde suaf si on été dans node feuille (car laba pour acceder à char dans node intermidere on a sa position pointeur derectement vers le text)

                                                   // 1: the transition length between the two node, father and son, so the number of chars between them.
                                                   // 1 : start position (to store node "k" information ) in the Trie

                    trie_build.nodes[father+1]+=1; // size in char utf_8 first symbol of outgoing transition, here in JS ci juste 1 case
                }

                trie_build.nodes[k+7]=trie_build.pos_code;  // future place of the node in the memory chunck
                // the node [0] is at 0, the node [1] will be at the address (space node [0]), ... etc

                trie_build.pos_code+=trie_build.local_size;
            }
        }


        trie_build.limit_leaf = trie_build.pos_code; // end of the zone devoted to the leaves !
        // all leaf nodes are encoded in a contiguous memory area

        // cette instruction (affichage) na aucun sense dans javascript, car chaque 1 element est un objet (et non 1octet)
        // console.log("\n Number of Bytes to code the leaves %d o, %d Ko, %d Mo",trie_build.limit_leaf,trie_build.limit_leaf/1024,(trie_build.limit_leaf/1024)/1024);


        /// coding the other nodes, starting from limit_leaf !
        // do the same with the remaining nodes (so intermediary nodes)

        for (k=trie_build.text_info.longest_line;k>=0;k--) // for all possible depth in decreasing order
        {
            node_tmp = trie_build.linking_nodes[k];

            //processing the nodes which are at the same depth
            while (node_tmp>-1) // there are nodes at this depth, follow the guide
            {
                if (trie_build.nodes[node_tmp]!==0) // not a leaf, otherise already coded
                {
                    /*
                    // trie_build.local_size=NUM_OCTETS_RESERVED_BY_NODE; //free space added for the topK
                    trie_build.local_size=1;

                    // (we retrieves the position of the beginning of this sequence )=== numero de sequenece
                    trie_build.local_size+=1;

                    //trie_build->local_size+=length_encode_int(trie_build->nodes[node_tmp]);  // number of transitions
                    trie_build.local_size+=1;

                    trie_build.local_size+=trie_build.nodes[node_tmp+1]; // plus the size of utf8

                    trie_build.local_size+=trie_build.nodes[node_tmp+4]; // plus the size of coding the transitions,

                    */
                   // the same
                   // 1 : free space added for the topK
                   // 1 : numero de sequenece
                   // 1 : number of transitions
                   // plus the size of utf8
                   // plus the size of coding the transitions
                   trie_build.local_size = 3 + trie_build.nodes[node_tmp+1] + trie_build.nodes[node_tmp+4];


                    // we do the same thing as the leaf nodes
                    if (trie_build.nodes[node_tmp+5]>-1) // to the father
                    {
                        // adding the size of coding the transition to the node to the size required to code the node
                        father=trie_build.nodes[node_tmp+5];

                      trie_build.nodes[father+4] +=3; // 1+1+1

                        // trie_build->nodes[father+1]+=u8_seqlen((char*)(trie_build->text_info->line_table[trie_build->nodes[node_tmp+3]]+trie_build->nodes[father+2]));
                        trie_build.nodes[father+1]+=1; // pr le char qui lier ce node transition depui son father aussi
                    }

                    trie_build.nodes[node_tmp+7]=trie_build.pos_code;  // future place of the node in the memory chunck

                    trie_build.pos_code+=trie_build.local_size;
                }

                node_tmp = trie_build.nodes[node_tmp+6];  // next node at the same depth
            }
        }

        // cette instruction na pas de sense dans javascript, car chaque elemeent est objet
        trie_build.Number_Bytes_Occupied_by_Trie = trie_build.pos_code;

        // console.log("\n Number of Bytes to code all nodes %d o, %d Ko, %d Mo",trie_build.pos_code,trie_build.pos_code/1024,(trie_build.pos_code/1024)/1024);
    };



    // filling the informations nodes in the tree.
    //      Initializes the positions where to place the transitions,
    //      and filling them in the tree
    function fillingTrie(trie_build)
    {
        var k=0,
            beg_utf8,
            father;

    // THIRD PHASE: INITIALIZES THE POSITION WHERE TO PLACE TRANSITIONS

        //trie_build->trie = (uchar *)malloc(trie_build->pos_code*sizeof(uchar));
        trie_build.trie = new Array(trie_build.pos_code);

        //memset(trie_build->trie,0,trie_build->pos_code);
        // ibra : no inisialization needed here, car les objet sont deferent,
        // on peut initialiser le tableau avec la valeur 0 en utilisant la boucle for
        // jai chercher et ji pas rouver une methode d'initialisation sauf la boucle

        trie_build.size_trie=trie_build.pos_code;   // size of the trie
        trie_build.pos_code = 0;

        // insert the information in Trie (not all info) , and make updates to the insertion position.

        for (k=0; k<=trie_build.last_node; k+=NUM_PAR)
        {
            trie_build.pos_code = trie_build.nodes[k+7];  // position of the node in the trie chunck (its place in the reserved memory)

            if (trie_build.nodes[k]!==0) // not a leaf
            {
                //trie_build->local_size = trie_build->pos_code + NUM_OCTETS_RESERVED_BY_NODE;
                trie_build.local_size = trie_build.pos_code + 1; // +1 : NUM_OCTETS_RESERVED_BY_NODE for the topK

                /*
                //if (strlen((char*)trie_build->text_info->line_table[trie_build->nodes[k+3]])==trie_build->nodes[k+2])
                if (trie_build.text_info.line_table[trie_build.nodes[k+3]].length===trie_build.nodes[k+2])
                {
                    // encode_int(trie_build->topK_tab[trie_build->nodes[k+3]],trie_build->trie+trie_build->pos_code); // encode the topK score
                    trie_build.trie[trie_build.pos_code]=trie_build.topK_tab[trie_build.nodes[k+3]];// encode the topK score

                    // trie_build->local_size += all_encode_int(trie_build->text_info->pos_line_table[trie_build->nodes[k+3]]+1,trie_build->trie+trie_build->local_size); // encoding seqeunce

                    //ibra :
                    // enregestri le numero de string
                    trie_build.trie[trie_build.local_size] = trie_build.nodes[k+3]; // encoding seqeunce
                    trie_build.local_size += 1;
                }
                else
                {
                    // trie_build->local_size+=all_encode_int(0,trie_build->trie+trie_build->local_size);

                    trie_build.trie[trie_build.local_size] = trie_build.nodes[k+3]; // encoding seqeunce number
                    trie_build.local_size+=1;
                }
                */
                // -------------------- if else precedent will be : cause dans un node intermidière on met pas de topK au debut, aussi un node intermidière dans cette veresion ni jami finale
                trie_build.trie[trie_build.local_size] = trie_build.nodes[k+3]; // encoding seqeunce number
                trie_build.local_size+=1;
                // -------------------------------------------------------------------------

                // je peut la simplifier, ne pas mettre if else, donc je met le topk et aussi le numero en meme temps
                // et pour le topK de node intermidère ci pas un pbm car on va faire une mise a jour après
                // je laisse comme sa maintenat pour que la structeur de code resmble au code de langage C.

                // trie_build->local_size += all_encode_int(trie_build->nodes[k],trie_build->trie+trie_build->local_size);// encoding of the number of transitions
                trie_build.trie[trie_build.local_size] = trie_build.nodes[k];
                trie_build.local_size += 1;

                beg_utf8 = trie_build.local_size;

                trie_build.local_size += trie_build.nodes[k+1]; // plus the size of utf8

                trie_build.nodes[k+1] = beg_utf8;  // beginning of the uft8 coding
                trie_build.nodes[k+4] = trie_build.local_size; // beginning of the transition coding
            }
            else // a leaf
            {
                // position of the node in the trie chunck
                //trie_build->local_size = trie_build->pos_code + NUM_OCTETS_RESERVED_BY_NODE;  // encoding of the number of sequence
                trie_build.local_size = trie_build.pos_code + 1;  // encoding of the number of sequence, + 1: for the topK

                //encode_int(trie_build->topK_tab[trie_build->nodes[k+3]],trie_build->trie+trie_build->pos_code); // encode the topK score
                trie_build.trie[trie_build.pos_code] = trie_build.topK_tab[trie_build.nodes[k+3]];

                //encode_int(trie_build->text_info->pos_line_table[trie_build->nodes[k+3]]+1,trie_build->trie+trie_build->local_size); // encoding the sequence number
                trie_build.trie[trie_build.local_size] = trie_build.nodes[k+3]; // encoding the sequence number
            }
        }


        // FOURTH PHASE: FILLING THE TRANSITIONS OF THE TRIE

        // Using the FATHER of EACH NODE to fill the utf8 transitions
        for (k=0; k<=trie_build.last_node; k+=NUM_PAR)
        {
            if (trie_build.nodes[k+5]>-1) // there is a father
            {
                father = trie_build.nodes[k+5]; // utf8 managing

                trie_build.pos_code = trie_build.nodes[father+1];   // position of the chunk utf8 of the father

                // memcpy(trie_build->trie+trie_build->pos_code , trie_build->text_info->line_table[trie_build->nodes[k+3]]+trie_build->nodes[father+2] , u8_seqlen((char*)(trie_build->text_info->line_table[trie_build->nodes[k+3]]+trie_build->nodes[father+2])));

                trie_build.trie[trie_build.pos_code] = trie_build.text_info.line_table[trie_build.nodes[k+3]].charAt( trie_build.nodes[father+2] );

                // add current char size to go to the next position to store the next transition char.
                trie_build.nodes[father+1] += 1;
            }
        }

        // Using the FATHER of EACH NODE to fill the integer transitions
        for (k=0; k<=trie_build.last_node; k+=NUM_PAR)
        {
            if (trie_build.nodes[k+5]>-1) // there is a father
            {
                father = trie_build.nodes[k+5];// utf8 managing

                trie_build.pos_code = trie_build.nodes[father+1];   // position of the chunk utf8 of the father

                // encoded the position of the beginning of the transition in the text              (pointer to the text)
                trie_build.trie[trie_build.pos_code] = trie_build.nodes[father+2] ; // le profndeur de pere pour trouver le debut de char dans le mot "k"
                trie_build.pos_code +=1;

                // number of char between two nodes                  (length)
                trie_build.trie[trie_build.pos_code] = trie_build.nodes[k+2]-trie_build.nodes[father+2];
                trie_build.pos_code +=1;

                // start position to encode this node in the Trie (pointed node)
                trie_build.trie[trie_build.pos_code] = trie_build.nodes[k+7];
                trie_build.pos_code += 1;

                //trie_build->nodes[father+1] = trie_build->pos_code;  // new bound for the transition zone of the father
                trie_build.nodes[father+1] = trie_build.pos_code;
            }
        }

        trie_build.first_node=trie_build.nodes[7];

        /// ///////////////////////////////////////////////////////////////////////////////////
        ///
        //    SUMMARY: insertion in the Trie the following information in order:
        // 0)  topK score
        // 1)  the sequence number
        // 2)  the number of transition
        //
        // 3) filling the first character of each transition in the Trie
        //
        // 4) encode all transition, for each transition we have :
        //  4')               the starting position of the transition in the word          (pointer to the word) by the depth of his father ;)
        //  4'')              number of char that are between two nodes                  (length)
        //  4''')             start position to encode this node in the Trie (pointed node)
        //
        /// so for :
        ///       Leaf node: 0, 1
        //
        ///   internal node :  0, 1 , 2 , 3 , 4
        /// ///////////////////////////////////////////////////////////////////////////////////
    };



    // update (TopK score) for all internal node
    // each node will take the max value of his transitions (son node)
    // initially the score TopK is in the leaf node
    function maxTopK_update(nodeToGetTopK, trie_build)
    {
        var nb_trans=0,    // the number of outgoing transition from the node
            pointed_node=0,
            k=0,
            max_TopK_ofTransitionNode=0,
            tomp_max=0,
            pos_code = nodeToGetTopK; // get the node position from the trie

        if ( pos_code < trie_build.limit_leaf ) // all the leaf nodes are coded 0 -> limit leaf (contiguous space)
        {
            // note : for leafs we have just :
            //    0)  topK score
            //    1)  the sequence number.

            tomp_max = trie_build.trie[nodeToGetTopK];

            if(tomp_max > max_TopK_ofTransitionNode)
            {
                max_TopK_ofTransitionNode = tomp_max;
            }
        }
        else // not a leaf
        {
            /*
            pos_code += 1; // 1: NUM_OCTETS_RESERVED_BY_NODE; // we avoid the topK score

            nb_sequence = trie_build.trie[pos_code++]; // number of sequnce
            */

            pos_code += 2;

            /*
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
            */


            nb_trans = trie_build.trie[pos_code++]; // number of trans


            // avoid the first char of each outgoing transition from this node
            /*
            for (k=0; k < nb_trans; k++)
            {
                pos_code+=1; // go to the next char
            }*/
            pos_code+= nb_trans;

            // for each transition node , we apply the update of the score topK
            for (k=0; k<nb_trans; k++)
            {
                /*
                pointer_sequence=trie_build.trie[pos_code++]; //just to advance in the trie
                length_transition=trie_build.trie[pos_code++]; //just to advance in the trie
                */
                pos_code+=2;
                pointed_node=trie_build.trie[pos_code++]; // transition node position in the trie, (we need just this information)

                tomp_max = maxTopK_update(pointed_node,trie_build);

                if(tomp_max > max_TopK_ofTransitionNode)
                {
                    max_TopK_ofTransitionNode = tomp_max;
                }
            }
        }

        trie_build.trie[nodeToGetTopK] = max_TopK_ofTransitionNode;

        return max_TopK_ofTransitionNode;
    };




    // printing the whole trie , text : là il respresent le tableau line_table qui contient tous les mots
    function printingWholeTrie(text, longest_line, trie, first_node, limit_leaf, num_octet_by_node)
    {
        var stmp,
            nb_sequence,
            nb_trans,
            pointed_node,
            pos_code,
            file_last_nodes = new Array(longest_line), //  a table which acts as a stack,
            file_last_nodes_counter = 0,
            k=0;

        file_last_nodes[0]=first_node; // stacking the root node

        while ( file_last_nodes_counter >=0 ) // while still nodes in the stack we treat them
        {
            pos_code = file_last_nodes[file_last_nodes_counter]; // get node from the stack

            if ( pos_code < limit_leaf ) // all the leaf nodes are coded 0 -> limit leaf (contiguous space)
            {
                nb_sequence = trie[pos_code+num_octet_by_node];

                console.log("leaf corresponding to sequence ->");
                console.log("%s",text[nb_sequence]);
                console.log("\n");

                file_last_nodes_counter--; //we finish with this node, so it pops
            }
            else // not a leaf
            {

                pos_code += num_octet_by_node; // skip the the topK

                // nb_sequence = length_decode_int(&pos_code,trie+pos_code); // number of sequnce
                nb_sequence = trie[pos_code]; // number of sequnce
                pos_code +=1;


                /*
                // it means that this node is not only intermediate. but it is also leaf
                /// we haven't this case, because we changed the structure to support topK
                if ( nb_sequence > 0 )
                {
                    fprintf(stdout,"Internal node corresponding to sequence ->");
                    fprintf(stdout,"%s",(--nb_sequence)+text);
                    fprintf(stdout,"\n");
                }

                */

                nb_trans = trie[pos_code]; // number of trans
                pos_code +=1;

                console.log("\n Transition characters:");

                // print the first char of each outgoing transition from this node
                // they are stored one after the other in a contiguous space
                for (k=0; k < nb_trans; k++)
                {
                    stmp=trie[pos_code];// get the char from the Trie


                    console.log(" char num %d = ",k);
                    console.log("%s ,",stmp);

                    pos_code++;// go to the next char
                }

                console.log("\n");

                file_last_nodes_counter--; //we finish with this node, so it pops

                // stack the transition  nodes of the current node in the stack. (then we choose the last node stacked to treat it)
                for (k=0; k<nb_trans; k++)
                {
                    /*
                    pointer_sequence=trie[pos_code++];  //just to advance in the trie                
                    length_transition=trie[pos_code++]; //just to advance in the trie
                    */
                    pos_code+=2;
                    pointed_node=trie[pos_code++]; // transition node position in the trie, (we need just this information)

                    file_last_nodes[++file_last_nodes_counter]=pointed_node; // stacking the transition node.

                }
                console.log("\n");
            }
        }
    };
};

// ========================================================================\\
// ----------------- query trie : exact

var lasteQueryWord_size=0, // the size in char (le nombre d'octet)
    pos_LastNode=-1,// solution node for the lasteQueryWord (give us the prefix in the Trie)
    tab_nodes_way=null,  // contain all nodes in the path followed by the solution
    tab_nodes_way_counter=0, // number of nodes added to the table
    isSolutionExist=false, // if we found solution with the previous query Word
    pos_last_node_char_tested =0; // the number of the last char tested at the node


//the query word must be encoded in UTF-8

// look up for the query word in the Trie,and return the solution node (otherwise -1).
// query word search starts each time from the root
function searchQueryWord(pattern,trie_info)
{   
    var line_table = trie_info.line_table,
        trie = trie_info.trie,
        firstNode = trie_info.first_node,
        i=0,
        p=0,
        k=0,
        nb_sequence=0, // the sequence number, the idx in the line_table
        nb_trans=0,    // the number of outgoing transition from the node
        pointer_sequence=0, // the starting position of the transition in the word  (pointer to the word), == char number (position) dans word
        length_transition=0, // the subsequence length between two nodes                  (length)
        pointed_node=0, // the address of outgoing node from father node (pointed node (child) from the  father node)
        pos_code=firstNode,  // the position by the number of byte. to move through the trie ,=  the first node (root)
        nb_letters = pattern.length; // number of char  of the pattern
    
    if(nb_letters===0) return -1;
    
    // console.log(" pattern = %s  , length = %d", pattern, nb_letters);


    while (k<nb_letters)
    {
        // console.log("\n char num : %d , le {%s}",k,pattern.charAt(k));
        
        /*
        pos_code += 1; // 1: NUM_OCTETS_RESERVED_BY_NODE; // skip the space of the score topK
        nb_sequence = trie[pos_code++]; //get sequence number and advance in the trie, go to the next information
        */
        pos_code += 2; // 1: NUM_OCTETS_RESERVED_BY_NODE && 1 : sequence number, skip theme both
        
        nb_trans = trie[pos_code++]; // get the number of outgoing transition and advance in the trie, go to the next information



        // finding the right transition that match the pattern character
        for (i=0; i<nb_trans; i++)
        {   
            if(pattern.charAt(k) === trie[pos_code]) break;

            pos_code += 1; // go to the character of the next transition
        }
        
        // console.log(" nb_trans = %d , i=%d",nb_trans,i);


        if (i===nb_trans) //we didn't find a match
        {
            break;
        }
        else // there is a transition by the letter
        {
            pos_code += (nb_trans - i); // skip all characters of remaining transition, going to the transitions chunck
             
            // skip all the information for each transition from the first transition until we get the transition where we find a match.
            pos_code += i*3; // car pour chaque node on a 3 information
            
            // ensuit on va extraire les info de notre node fils
            pointer_sequence = trie[pos_code++]; //  the starting position of the transition in the word  (pointer to the word)
            length_transition = trie[pos_code++]; //  the subsequence length between two nodes
            pointed_node = trie[pos_code++];      // transition node position in the trie
            
            nb_sequence = trie[pointed_node+1]; // recupirer le numero de sequence de node fils, car on va travailler sur
                                                // ces caractère qui sont entre lui et sont pere
            
            // on peut sauter sa derectemt, on va just pultiplier par 3, car chaque node conteint 3 information

            k += 1; // go to the next character in the pattern
            pointer_sequence += 1; // go to the next position in the word
            length_transition -= 1; // the length of the sub-segment between two nodes decreases with the tested character length
            
            
            
            // console.log(" char in pattern = %d , char_in_word = %d , num_seq = %d , length_trans = %d , %s",k,pointer_sequence,nb_sequence, length_transition,line_table[nb_sequence]);

            // check the characters that are between two nodes (from word) with the pattern
            p=0;
            while (p<length_transition && k<nb_letters)
            {

                if (pattern.charAt(k)!==line_table[nb_sequence].charAt(pointer_sequence)) break;

                p += 1;
                k += 1;
                pointer_sequence += 1;
            }

            // if we tested all character between the the Two node, so we go to the outgoing node (the son node)
            if (p===length_transition)
            {
                pos_code=pointed_node;

                // note : we can have this case : p==length_transition  && k==nb_letters , so here we have solution
            }
            else
            {
                // we tested all pattern character before reaching the next node (pointed_node), we have solution
                if (k===nb_letters) break; //

                // case : p!=length_transition && k!=nb_letters , so no solution
                /// console.log("\n\n Fail reading the transition on character %d\n",k);
                return -1;
            }
        }
    }


    if (k===nb_letters) // if we tested all the pattern chars, so we have a solution
    {
        return pointed_node;
    }

    return -1;
};



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

function searchSuffixbyPrefix(pattern,lengPrefixCommon,trie_info)
{   
    var line_table = trie_info.line_table,
        trie = trie_info.trie,
        firstNode = trie_info.first_node,
        i=0,
        p=0,
        k=0,
        nb_sequence=0, // the sequence number, the idx in the line_table
        nb_trans=0,    // the number of outgoing transition from the node
        pointer_sequence=0, // the starting position of the transition in the word  (pointer to the word), == char number (position) dans word
        length_transition=0, // the subsequence length between two nodes                  (length)
        pointed_node=0, // the address of outgoing node from father node (pointed node (child) from the  father node)
        pos_code= firstNode, // the first node (root)
        last_pointed_node = pointed_node =firstNode, // position of the first node (root) of the trie
        nb_letters = pattern.length; // number of char  of the pattern

    if(nb_letters===0) // empty query word, no solution, reset variables, which allows to search from the node of previous request
    {
        lasteQueryWord_size=0;
        pos_last_node_char_tested =0;
        pos_LastNode = firstNode; // root
        return -1; // return no solution
    }


    // lengPrefixCommon must be > 0, to avoid the conflict of the first execution
    if(lengPrefixCommon>0 && lengPrefixCommon===lasteQueryWord_size) // last query word is prefix total of this query word, (or both are the same)
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

    // console.log(" k = "+k);

    while (k<nb_letters)
    {
        pos_code += 2;
        nb_trans = trie[pos_code++];

        // finding the right transition that match the pattern character
        for (i=0; i<nb_trans; i++)
        {   
            if(pattern.charAt(k) === trie[pos_code]) break;

            pos_code += 1; // go to the character of the next transition
        }

        if (i===nb_trans)
        {
            break;
        }
        else
        {
            pos_code += (nb_trans - i);
             
            pos_code += i*3;
            
            pointer_sequence = trie[pos_code++]; 
            length_transition = trie[pos_code++];
            pointed_node = trie[pos_code++];     
            
            nb_sequence = trie[pointed_node+1]; 
            
            k += 1; 
            pointer_sequence += 1;
            length_transition -= 1;

            p=0;
            while (p<length_transition && k<nb_letters)
            {

                if (pattern.charAt(k)!==line_table[nb_sequence].charAt(pointer_sequence)) break;

                p += 1;
                k += 1;
                pointer_sequence += 1;
            }

            if (p===length_transition)
            {
                pos_code=pointed_node;

                last_pointed_node = pointed_node; // save the node

                pos_last_node_char_tested=k; // save the number of chars which correspond to the node level
            }
            else
            {
                if (k===nb_letters) break;

                // no solution, reset variables, to use them in the next request
                lasteQueryWord_size=nb_letters;
                isSolutionExist = false;
                pos_last_node_char_tested = 0;
                pos_LastNode = firstNode;
                return -1;
            }
        }
    }

    if (k===nb_letters)
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

};


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
function search_Prefix_by_Suffix_Delete(pattern, lengPrefixCommon,trie_info)
{   
    var line_table = trie_info.line_table,
        trie = trie_info.trie,
        firstNode = trie_info.first_node,
        longest_line = trie_info.longest_line,
        i=0,
        p=0,
        k=0,
        nb_sequence=0,
        nb_trans=0,
        pointer_sequence=0,
        length_transition=0,
        pointed_node=0,
        pos_code = firstNode,
        last_pointed_node = pointed_node =firstNode,
        nb_letters = pattern.length;
    
    if(tab_nodes_way===null) tab_nodes_way =new Array(longest_line);
    
    if(nb_letters===0)
    {
        lasteQueryWord_size=0;
        pos_last_node_char_tested =0;
        pos_LastNode = firstNode;
        return -1;
    }


    if(lengPrefixCommon === 0) //no common prefix. reset the table that will contain the search path nodes
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

        if(lengPrefixCommon === lasteQueryWord_size) // exactly like the previous function : the 1st query Word is a total prefix of the 2nd
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
                while(tab_nodes_way[tab_nodes_way_counter]===-1)
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
        /*
        pos_code += 1; // 1: NUM_OCTETS_RESERVED_BY_NODE; // skip the space of the score topK
        nb_sequence = trie[pos_code++]; //get sequence number and advance in the trie, go to the next information
        */
        pos_code += 2; // 1: NUM_OCTETS_RESERVED_BY_NODE && 1 : sequence number, skip theme both
        
        nb_trans = trie[pos_code++]; // get the number of outgoing transition and advance in the trie, go to the next information



        // finding the right transition that match the pattern character
        for (i=0; i<nb_trans; i++)
        {   
            if(pattern.charAt(k) === trie[pos_code]) break;

            pos_code += 1; // go to the character of the next transition
        }

        if (i===nb_trans)
        {
            break;
        }
        else
        {
            pos_code += (nb_trans - i); // skip all characters of remaining transition, going to the transitions chunck
             
            // skip all the information for each transition from the first transition until we get the transition where we find a match.
            pos_code += i*3; // car pour chaque node on a 3 information
            
            // ensuit on va extraire les info de notre node fils
            pointer_sequence = trie[pos_code++]; //  the starting position of the transition in the word  (pointer to the word)
            length_transition = trie[pos_code++]; //  the subsequence length between two nodes
            pointed_node = trie[pos_code++];      // transition node position in the trie
            
            nb_sequence = trie[pointed_node+1]; // recupirer le numero de sequence de node fils, car on va travailler sur
                                                // ces caractère qui sont entre lui et sont pere
            
            // on peut sauter sa derectemt, on va just pultiplier par 3, car chaque node conteint 3 information

            k += 1; // go to the next character in the pattern
            pointer_sequence += 1; // go to the next position in the word
            length_transition -= 1; // the length of the sub-segment between two nodes decreases with the tested character length
            
            // check the characters that are between two nodes (from word) with the pattern
            p=0;
            while (p<length_transition && k<nb_letters)
            {

                if (pattern.charAt(k)!==line_table[nb_sequence].charAt(pointer_sequence)) break;

                p += 1;
                k += 1;
                pointer_sequence += 1;
            }


            if (p===length_transition)
            {
                pos_code=pointed_node;

                last_pointed_node = pointed_node;

                pos_last_node_char_tested=k;

                tab_nodes_way_counter=k; // get the position of the node in the table (the same as the nb of chars till this node)
                tab_nodes_way[tab_nodes_way_counter] =   pos_code; // save the current node

            }
            else
            {
                if (k===nb_letters) break;

                lasteQueryWord_size=nb_letters;
                isSolutionExist = false;
                pos_last_node_char_tested = 0;
                pos_LastNode = firstNode;
                return -1;
            }
        }
    }


    if (k===nb_letters)
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
};


// ibra : tu peut changer sa par la suit, donc on a besoin qu de pattern en entrie
// cela signifier que on va supprimer la list string info, car elle ne ser a rien dans ce cas. // ok ci fait
// dynamic topK : update All path Nodes of the chosen word (by Searching)
function topK_updating_AllWayNodes_bySearching(pattern, trie_info)
{
    var line_table = trie_info.line_table,
        trie = trie_info.trie,
        firstNode = trie_info.first_node,
        list_way_info= [],
        topK_value,
        i=0,
        p=0,
        nb_letters = pattern.length,
        k=0,
        nb_sequence=0,
        nb_trans=0,
        pointer_sequence=0,
        pointed_node=0,
        length_transition=0,
        pos_code = firstNode;

    if(nb_letters===0) return;
    
    list_way_info.push(firstNode); // add the firs node to the stack,

    while (k<nb_letters)
    {
        // pos_code += 1; // 1:NUM_OCTETS_RESERVED_BY_NODE;
        // nb_sequence = trie[pos_code++];
       
        pos_code += 2;
        
        nb_trans = trie[pos_code++];

        for (i=0; i<nb_trans; i++)
        {   
            if(pattern.charAt(k) === trie[pos_code]) break;

            pos_code += 1;
        }

        if (i===nb_trans)
        {
            break;
        }
        else
        {
            pos_code += (nb_trans - i);
            
            pos_code += i*3;

            pointer_sequence = trie[pos_code++]; 
            length_transition = trie[pos_code++];
            pointed_node = trie[pos_code++];
            
            nb_sequence = trie[pointed_node+1];
            
            k += 1;
            pointer_sequence += 1;
            length_transition -= 1;

            p=0;
            while (p<length_transition && k<nb_letters)
            {

                if (pattern.charAt(k)!==line_table[nb_sequence].charAt(pointer_sequence)) break;

                p += 1;
                k += 1;
                pointer_sequence += 1;
            }

            if (p===length_transition)
            {
                pos_code=pointed_node;
                
                list_way_info.push(pos_code);
            }
            else
            {
                if (k===nb_letters) break;
                /// console.log("Fail reading the transition on character %d\n",k);
                return ;
            }
        }
    }

    // the pattern ends in a final node, or it end in the middle of a transition and in this case we go to the pointed node (finale node)
    if (k===nb_letters)
    {
        pos_code = pointed_node; // the last node

        // pos_code += 1; // 1:NUM_OCTETS_RESERVED_BY_NODE;
        // nb_sequence = trie[pos_code++];
        
        pos_code += 2;
        
        nb_trans = trie[pos_code++];

        if(nb_trans!==0) // if we are in intermediary node, we go to the next node (the finale node)
        {
            // We must find the number of the right transition, we know that the transition have '\0' in c/c++ as char, in javascrip : undefined car on a rien
            for (i=0; i<nb_trans; i++)
            {
                if (pattern.charAt(k)=== trie[pos_code]) break; // === undefined

                pos_code += 1;
            }


            if (i!==nb_trans) // there is a transition by the letter
            {
                pos_code += (nb_trans - i);
                
                pos_code += i*3;

                pointer_sequence = trie[pos_code++]; 
                length_transition = trie[pos_code++];
                pointed_node = trie[pos_code++];
                
                nb_sequence = trie[pointed_node+1];

                pos_code=pointed_node;

                // add the last node
                list_way_info.push(pos_code);
            }
        }

        pos_code = list_way_info[list_way_info.length-1]; // recupirer le dernnier node de notre mot
        topK_value = trie[pos_code]; 
        //topK_value++;
		topK_value+=options.score_update_unit;
        
        console.log(" node = "+pos_code+" , topK = "+trie[pos_code]+", sequnec = "+line_table[trie[pos_code+1]]);
        
        // updating
        var indice =0,
            sizeList_way_info = list_way_info.length;
        
        for(indice=0; indice < sizeList_way_info; indice++)
        {
            if(topK_value > trie[list_way_info[indice]] )
            {
                trie[list_way_info[indice]]=topK_value;
            }
        }
    }

    return ;
};


// ========================================================================\\
// ----------------- query trie : approximate (edit distance : 1 err)

/* // thos var exist befor as global var, in { query trie : exact}
var lasteQueryWord_size=0,
    pos_LastNode=-1,
    tab_nodes_way=null,
    tab_nodes_way_counter=0,
    isSolutionExist=false,
    pos_last_node_char_tested =0,
    */
var branch_charLevels = null,//Stores the branching levels (the char that represents a branch)
    branch_trans_num = null,//Stores the transition num of the branching character
    branch_nodes = null,// Stores the branching nodes
    branch_node_trans_chunk = null,// Stores a pointer to the chunk representing the branching node in its parent
    idx_path=0; // counter for the branch nodes and transition


// approximate search : look up for the query word in the Trie (we accept 1 error),and return the solution node (otherwise -1).
// the code is the same as searchQuery Word in query trie, just here we add the part that treat the error, so i will comment just this part
function searchQueryWord_1error(pattern,trie_info)
{   
    var line_table = trie_info.line_table,
        trie = trie_info.trie,
        longest_line = trie_info.longest_line,
        firstNode = trie_info.first_node,
        listSolution = new exact_and_1err_Solution_nodes(), // a list containing all solutions nodes : exact (if we have) and with error
        list_solution_1error={}, // hash_table
        i=0,
        p=0,
        k=0,
        nb_sequence=0,
        nb_trans=0,     
        pointer_sequence=0,
        length_transition=0,
        pos_code=firstNode,
        pointed_node =firstNode,
        isExactSolutionFound =false,
        nbChar_rest_to_test_inTransition = 0, // size of all chars in bytes still to test in transition (length of substring)
        nb_letters=pattern.length;
        
    if(nb_letters===0) return listSolution; // return empty list


    // we can made this initialization just one time. outside of this function
    if(branch_charLevels===null)
    {
        branch_charLevels = new Array(longest_line+1);//Stores the branching levels
        branch_trans_num = new Array(longest_line+1);//Stores the transition num of the branching character
        branch_nodes = new Array(longest_line+1);// Stores the branching nodes
        branch_node_trans_chunk = new Array(longest_line+1);// Stores a pointer to the chunk representing the branching node in its parent
    }

    idx_path=0;


    /// phase 1 : exact search
    // maybe you say : why don't we just call the function that do the exact search and then we make the approximate search
    // response : we have some new code to add in the middle of the exact search, example : save node, transition, chars (for using this information in the approximate search)
    while (k<nb_letters)
    {
        branch_nodes[idx_path] = pos_code; // save the node

        /*
        pos_code += 1; // 1: NUM_OCTETS_RESERVED_BY_NODE; // skip the space of the score topK
        nb_sequence = trie[pos_code++]; //get sequence number and advance in the trie, go to the next information
        */
        pos_code += 2; // 1: NUM_OCTETS_RESERVED_BY_NODE && 1 : sequence number, skip theme both
        
        nb_trans = trie[pos_code++];
        
        for (i=0; i<nb_trans; i++)
        {   
            if(pattern.charAt(k) === trie[pos_code]) break;

            pos_code += 1; // go to the character of the next transition
        }

        branch_charLevels[idx_path]=k; // Set the current branching level (the index of the branching character in the pattern)
        branch_trans_num[idx_path]=i; // Set the branching transition num

        if (i===nb_trans)
        {
            branch_node_trans_chunk[idx_path++]=pos_code; // we save the information of transition nodes where we arrived in the test
            break;
        }
        else // there is a transition by the letter
        {
            pos_code += (nb_trans - i);

            branch_node_trans_chunk[idx_path++]=pos_code; // we save the information of transition nodes where we arrived in the test
   
            pos_code += i*3; 
                        
            pointer_sequence = trie[pos_code++]; 
            length_transition = trie[pos_code++];
            pointed_node = trie[pos_code++];     
            
            nb_sequence = trie[pointed_node+1]; 
                                                

            k += 1; 
            pointer_sequence += 1; 
            length_transition -= 1;
            
            
            p=0;
            while (p<length_transition && k<nb_letters)
            {

                if (pattern.charAt(k)!==line_table[nb_sequence].charAt(pointer_sequence)) break;

                p += 1;
                k += 1;
                pointer_sequence += 1;
            }


            if (p===length_transition)
            {
                pos_code=pointed_node;
            }
            else
            {
                if (k===nb_letters) break;

                /// console.log("\n\n Fail reading the transition on character %d\n",k);
                break; // We have no solution, go out to do approximate search with 1 error.
            }
        }
    }


    if (k===nb_letters)
    {
        isExactSolutionFound = true;

        // listSolution.push(pointed_node); // add the solution node of the exact search
        listSolution.exact_solution = pointed_node;
    }
    /*
    else
    {
        console.log("\n\t no solution exact found ");
    }
    */

    /// phase 2: approximate search ********************************************
    // pos_mismatchCharPattern = k
    // pos_mismatchCharTrie = pointer_sequence
    // pos_code = pos_code
    // pointed_node = pointed_node
    // p = p
    // length_transition = length_transition
    //  nomber of char rest to test  = length_transition-p

    nbChar_rest_to_test_inTransition = length_transition-p;

    test_OneError(trie_info,pattern,nb_letters,k,pointer_sequence,nbChar_rest_to_test_inTransition,pos_code,pointed_node,list_solution_1error,isExactSolutionFound);

    // recupirer les resulta et es mettre dans un tableau normale (bon on peut ne pas faire sa, et on travil avec se hash tableau partout donc meme dans les autres fonction, bon il y a juste une seule etuilisation celle de retrouver les resultat, donc on fait un seule parcoure au llieu de 2, mais bon le tableau generalement est trs petite donc cette manipulation matebanche :p)
    for(var val in list_solution_1error)
    {
        listSolution.list_solution_1error.push(list_solution_1error[val]);
    }
    
    return listSolution;
};


// there are cases where the result come duplicate, for example find the same word with deletion and insertion

// call the three function for edit distance : deletion, insertion and substitution
function edit_Distance(trie_info,pattern, nb_letters, k, pointer_sequence, nbChar_rest_to_test_inTransition, pos_code, pointed_node,list_solution_1error)
{
    edit_Distance_DELET(trie_info,pattern,nb_letters,k,pointer_sequence,nbChar_rest_to_test_inTransition,pos_code,pointed_node,list_solution_1error);

    edit_Distance_INSERT(trie_info,pattern,nb_letters,k,pointer_sequence,nbChar_rest_to_test_inTransition,pos_code,pointed_node,list_solution_1error);

    edit_Distance_SUBST(trie_info,pattern,nb_letters,k,pointer_sequence,nbChar_rest_to_test_inTransition,pos_code,pointed_node,list_solution_1error);
};

// 3 : substitution : skip in the same position, the char in pattern and the char in Trie, and then test others with exact search.
function edit_Distance_SUBST(trie_info, pattern, nb_letters, save_k, save_pointer_sequence, nbChar_rest_to_test_inTransition, pos_code, pointed_node,list_solution_1error)
{
    var k = save_k + 1, // get the position in the pattern (save_k), and then add the length of the next char to skip it.
        pointer_sequence = save_pointer_sequence + 1, // get the position in the trie (it my in the text) , and then add the length of the next char to skip it.
        length_transition = nbChar_rest_to_test_inTransition - 1; // decrease the size of transition, because we skip a char in the trie.

    // test others chars with exact search (we begin from a middle of transition or from node)
    var nodeSolution = searchWildcare(pattern,trie_info,pointer_sequence,k,length_transition,nb_letters,pos_code,pointed_node);


    if(nodeSolution !== -1) // if we have solution with substitution method, we add it to the the list
    {
        if(!list_solution_1error[nodeSolution]) 
            list_solution_1error[nodeSolution] = nodeSolution;
        
        /// list_solution_1error.push(nodeSolution );
        
        /// console.log(" SUBST : nodeSolution = "+nodeSolution);
    }
};

// 1 : deletion  : skip char in pattern , and then test others with exact search.
function edit_Distance_DELET(trie_info, pattern, nb_letters, save_k, pointer_sequence, nbChar_rest_to_test_inTransition, pos_code, pointed_node,list_solution_1error)
{
    var k = save_k + 1;// get the position in the pattern (save_k), and then add the length of the next char to skip it.

    var nodeSolution = searchWildcare(pattern,trie_info,pointer_sequence,k,nbChar_rest_to_test_inTransition,nb_letters,pos_code,pointed_node);

    if(nodeSolution !== -1)
    {
        if(!list_solution_1error[nodeSolution]) 
            list_solution_1error[nodeSolution] = nodeSolution;
        
        /// list_solution_1error.push(nodeSolution );
        
        /// console.log(" DELET : nodeSolution = "+nodeSolution);
    }
};

// 2 : insertion : skip char in Trie, and then test others with exact search.
function edit_Distance_INSERT(trie_info, pattern, nb_letters, save_k, save_pointer_sequence, nbChar_rest_to_test_inTransition, pos_code, pointed_node,list_solution_1error)
{
    var pointer_sequence = save_pointer_sequence + 1,
        length_transition = nbChar_rest_to_test_inTransition - 1;

    var nodeSolution  = searchWildcare(pattern,trie_info,pointer_sequence,save_k,length_transition,nb_letters,pos_code,pointed_node);

    if(nodeSolution !== -1)
    {
        if(!list_solution_1error[nodeSolution]) 
            list_solution_1error[nodeSolution] = nodeSolution;
        
        /// list_solution_1error.push(nodeSolution );
        
        /// console.log(" INSERT : nodeSolution = "+nodeSolution);
    }
};

// check nodes path already traversed by the exact search (till the error position)
function all_wayNodes(trie_info, pattern, nb_letters,list_solution_1error)
{
    var nbranches=idx_path,  // the number of node and transition chars traversed till the error position
        pos_code=0,
        nb_trans=0,
        j=0,
        p=0,
        pointer_sequence=0,
        length_transition=0,
        pointed_node=0;

    /// console.log(" all_wayNodes ");

    // for each node traversed we make edit distance verification just in the first char of each transition
    for(idx_path=0;idx_path<nbranches;idx_path++)
    {
        /// console.log("   ---- branch_nodes n=° "+idx_path);
        
        pos_code=branch_nodes[idx_path]; //  get the node

        // get the node information
        /*
        pos_code+=1; //1:NUM_OCTETS_RESERVED_BY_NODE;
        nb_sequence=trie_info.trie[pos_code++]; // number of sequnce
        */
        pos_code+=2; //1:NUM_OCTETS_RESERVED_BY_NODE , 1 : number of sequnce
       
        nb_trans=trie_info.trie[pos_code++]; // number of trans

        // for each transition we make edit distance verification (just the first char), we skip the transition traversed before in exact search
        for(j=0;j<nb_trans;j++)
        {
            // if this transition is in the table, this means that we traversed by it in exact search, so we skip it
            if(branch_trans_num[idx_path]===j)
            {
                continue;
            }

            pos_code=branch_node_trans_chunk[idx_path]; // the begin of all outgoing node (their informations)

            // go to the information of node j
            /*
            for (p=0;p<=j;p++)
            {
                pointer_sequence=trie_info.trie[pos_code++];
                length_transition=trie_info.trie[pos_code++];
                pointed_node=trie_info.trie[pos_code++];
            }
            */
            // ibra : optimiser cette boucle avec :
            
            pos_code += j*3;
            pointer_sequence=trie_info.trie[pos_code++];
            length_transition=trie_info.trie[pos_code++];
            pointed_node=trie_info.trie[pos_code++];
            

            /// console.log("branch_charLevels[idx_path] = "+branch_charLevels[idx_path]+" == "+pattern.charAt(branch_charLevels[idx_path]));
            // branch_charLevels[idx_path] == save_k, (char  position in pattern)
            // call the edit_Distance function to test error in the beginning of the node
            edit_Distance(trie_info,pattern,nb_letters,branch_charLevels[idx_path],pointer_sequence,length_transition,pos_code,pointed_node,list_solution_1error);
        }
    }
};

// continue the search with one error from the position where we stopped in the exact search
function test_OneError(trie_info, pattern, nb_letters, k, pointer_sequence, nbChar_rest_to_test_inTransition, pos_code, pointed_node,list_solution_1error, isExactSolutionFound)
{
    // no exact solution
    // check the error where we stopped and also at each node traversed before (the nodes of the path)
    if(!isExactSolutionFound)
    {
        edit_Distance(trie_info,pattern,nb_letters,k,pointer_sequence,nbChar_rest_to_test_inTransition,pos_code,pointed_node,list_solution_1error);

        // edit distance with all nodes of the traversed path
        all_wayNodes(trie_info,pattern,nb_letters,list_solution_1error);
    }
    else // check just nodes of the traversed path
    {
        all_wayNodes(trie_info,pattern,nb_letters,list_solution_1error);
    }
};

// check the rest of the chars (exact search), after the operation of edit distance jumps char in the pattern or in the trie or two at the same time
function searchWildcare(pattern, trie_info, pointer_sequence, k, length_transition, nb_letters, pos_code, pointed_node)
{
    var line_table = trie_info.line_table,
        p=0,
        nb_sequence = trie_info.trie[pointed_node+1],
        pos_node_stop_searsh,
        save_first_node;

    // if the error is in the middle of transition, so we start the verification from the middle
    while (p<length_transition && k<nb_letters)
    {
        if (pattern.charAt(k)!==line_table[nb_sequence].charAt(pointer_sequence)) break;

        p += 1;
        k += 1;
        pointer_sequence += 1;
    }


    if (p===length_transition)
    {
        pos_code=pointed_node;

        pos_node_stop_searsh = pointed_node;

        if(k!==nb_letters)
        {
            // Now the rest of verification made ??from the position of a node.
            // here we call directly the function of the exact search with the position of our node

            save_first_node = trie_info.first_node;
            trie_info.first_node = pointed_node; // set the current node

            // pos_node_stop_searsh = searchQueryWord(pattern.substring(k),trie_info); // Call function exact search to continue the checking
            pos_node_stop_searsh = searchQueryWord_from_idx_pattern(pattern,k,trie_info); // Call function exact search to continue the checking from the position 'k'
            
            trie_info.first_node = save_first_node;

        }

        return pos_node_stop_searsh;
    }
    else
    {
        if (k===nb_letters)
        {
            return pointed_node;
        }

        return -1; // no solution
    }

    return -1;

};

// look up for the query word in the Trie,and return the solution node (otherwise -1).
// this function is exactly the same as 'searchQueryWord', 
// the deference is here we begain search from idx and not from 0, so we use this function to avoid the use of substring to creat a new pattern, exemple pattern = pattern.substring(idx);
function searchQueryWord_from_idx_pattern(pattern,idx,trie_info)
{   
    var line_table = trie_info.line_table,
        trie = trie_info.trie,
        firstNode = trie_info.first_node,
        i=0,
        p=0,
        k=idx, // we begain search from this position
        nb_sequence=0, // the sequence number, the idx in the line_table
        nb_trans=0,    // the number of outgoing transition from the node
        pointer_sequence=0, // the starting position of the transition in the word  (pointer to the word), == char number (position) dans word
        length_transition=0, // the subsequence length between two nodes                  (length)
        pointed_node=0, // the address of outgoing node from father node (pointed node (child) from the  father node)
        pos_code=firstNode,  // the position by the number of byte. to move through the trie ,=  the first node (root)
        nb_letters = pattern.length; // number of char  of the pattern
    
    if(nb_letters===0) return -1;
    
    // console.log(" pattern = %s  , length = %d", pattern, nb_letters);


    while (k<nb_letters)
    {
        // console.log("\n char num : %d , le {%s}",k,pattern.charAt(k));
        
        /*
        pos_code += 1; // 1: NUM_OCTETS_RESERVED_BY_NODE; // skip the space of the score topK
        nb_sequence = trie[pos_code++]; //get sequence number and advance in the trie, go to the next information
        */
        pos_code += 2; // 1: NUM_OCTETS_RESERVED_BY_NODE && 1 : sequence number, skip theme both
        
        nb_trans = trie[pos_code++]; // get the number of outgoing transition and advance in the trie, go to the next information



        // finding the right transition that match the pattern character
        for (i=0; i<nb_trans; i++)
        {   
            if(pattern.charAt(k) === trie[pos_code]) break;

            pos_code += 1; // go to the character of the next transition
        }
        
        // console.log(" nb_trans = %d , i=%d",nb_trans,i);


        if (i===nb_trans) //we didn't find a match
        {
            break;
        }
        else // there is a transition by the letter
        {
            pos_code += (nb_trans - i); // skip all characters of remaining transition, going to the transitions chunck
             
            // skip all the information for each transition from the first transition until we get the transition where we find a match.
            pos_code += i*3; // car pour chaque node on a 3 information
            
            // ensuit on va extraire les info de notre node fils
            pointer_sequence = trie[pos_code++]; //  the starting position of the transition in the word  (pointer to the word)
            length_transition = trie[pos_code++]; //  the subsequence length between two nodes
            pointed_node = trie[pos_code++];      // transition node position in the trie
            
            nb_sequence = trie[pointed_node+1]; // recupirer le numero de sequence de node fils, car on va travailler sur
                                                // ces caractère qui sont entre lui et sont pere
            
            // on peut sauter sa derectemt, on va just pultiplier par 3, car chaque node conteint 3 information

            k += 1; // go to the next character in the pattern
            pointer_sequence += 1; // go to the next position in the word
            length_transition -= 1; // the length of the sub-segment between two nodes decreases with the tested character length
            
            
            
            // console.log(" char in pattern = %d , char_in_word = %d , num_seq = %d , length_trans = %d , %s",k,pointer_sequence,nb_sequence, length_transition,line_table[nb_sequence]);

            // check the characters that are between two nodes (from word) with the pattern
            p=0;
            while (p<length_transition && k<nb_letters)
            {

                if (pattern.charAt(k)!==line_table[nb_sequence].charAt(pointer_sequence)) break;

                p += 1;
                k += 1;
                pointer_sequence += 1;
            }

            // if we tested all character between the the Two node, so we go to the outgoing node (the son node)
            if (p===length_transition)
            {
                pos_code=pointed_node;

                // note : we can have this case : p==length_transition  && k==nb_letters , so here we have solution
            }
            else
            {
                // we tested all pattern character before reaching the next node (pointed_node), we have solution
                if (k===nb_letters) break; //

                // case : p!=length_transition && k!=nb_letters , so no solution
                /// console.log("\n\n Fail reading the transition on character %d\n",k);
                return -1;
            }
        }
    }


    if (k===nb_letters) // if we tested all the pattern chars, so we have a solution
    {
        return pointed_node;
    }

    return -1;
};

// approximate search with 1 error
// start the search from the solution node of the last query word . if it is total prefix of the current word query, otherwise start the search from the root
function searchSuffixbyPrefix_1error(pattern, lengPrefixCommon, trie_info)
{
    var line_table = trie_info.line_table,
        trie = trie_info.trie,
        longest_line = trie_info.longest_line,
        firstNode = trie_info.first_node,
        listSolution = new exact_and_1err_Solution_nodes(), // a list containing all solutions nodes : exact (if we have) and with error
        list_solution_1error={}, // hash_table
        i=0,
        p=0,
        k=0,
        nb_sequence=0,
        nb_trans=0,
        pointer_sequence=0,
        pointed_node=0,
        length_transition=0,
        pos_code=firstNode,
        last_pointed_node = pointed_node =firstNode,
        isExactSolutionFound =false,
        nbChar_rest_to_test_inTransition = 0,
        nb_letters = pattern.length;
    
    if(nb_letters===0)
    {
        lasteQueryWord_size=0;
        pos_last_node_char_tested =0;
        pos_LastNode = firstNode;

        idx_path=0;

        return listSolution;
    }

    if(branch_charLevels===null)
    {
        branch_charLevels = new Array(longest_line+1);//Stores the branching levels

        branch_trans_num = new Array(longest_line+1);//Stores the transition num of the branching character

        branch_nodes = new Array(longest_line+1);// Stores the branching nodes

        branch_node_trans_chunk = new Array(longest_line+1);// Stores a pointer to the chunk representing the branching node in its parent
    }

    if(lengPrefixCommon>0 && lengPrefixCommon===lasteQueryWord_size)
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

        /*
        pos_code += 1; // 1: NUM_OCTETS_RESERVED_BY_NODE; // skip the space of the score topK
        nb_sequence = trie[pos_code++]; //get sequence number and advance in the trie, go to the next information
        */
        pos_code += 2; // 1: NUM_OCTETS_RESERVED_BY_NODE && 1 : sequence number, skip theme both
        
        nb_trans = trie[pos_code++];
        
        for (i=0; i<nb_trans; i++)
        {   
            if(pattern.charAt(k) === trie[pos_code]) break;

            pos_code += 1; // go to the character of the next transition
        }

        branch_charLevels[idx_path]=k;

        branch_trans_num[idx_path]=i;

        if (i===nb_trans)
        {
            branch_node_trans_chunk[idx_path++]=pos_code;
            break;
        }
        else
        {
            pos_code += (nb_trans - i);

            branch_node_trans_chunk[idx_path++]=pos_code;

            pos_code += i*3; 
            
            
            pointer_sequence = trie[pos_code++]; 
            length_transition = trie[pos_code++];
            pointed_node = trie[pos_code++];     
            
            nb_sequence = trie[pointed_node+1]; 
                                                
            k += 1; 
            pointer_sequence += 1; 
            length_transition -= 1;

            p=0;
            while (p<length_transition && k<nb_letters)
            {

                if (pattern.charAt(k)!==line_table[nb_sequence].charAt(pointer_sequence)) break;

                p += 1;
                k += 1;
                pointer_sequence += 1;
            }

            if (p===length_transition)
            {
                pos_code=pointed_node;

                last_pointed_node = pointed_node;

                pos_last_node_char_tested=k;
            }
            else
            {
                if (k===nb_letters) break;

                lasteQueryWord_size = nb_letters;
                isSolutionExist = false;
                pos_last_node_char_tested = 0;
                pos_LastNode = firstNode;

                break; // no solution
            }
        }
    }

    if (k===nb_letters)
    {
        lasteQueryWord_size=k;
        pos_LastNode=last_pointed_node;

        isSolutionExist = true;
        isExactSolutionFound = true;

        /// listSolution.push(pointed_node);
        listSolution.exact_solution = pointed_node;
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

    test_OneError(trie_info,pattern,nb_letters,k,pointer_sequence,nbChar_rest_to_test_inTransition,pos_code,pointed_node,list_solution_1error,isExactSolutionFound);
    
    // recupirer les resulta et es mettre dans un tableau normale (bon on peut ne pas faire sa, et on travil avec se hash tableau partout donc meme dans les autres fonction, bon il y a juste une seule etuilisation celle de retrouver les resultat, donc on fait un seule parcoure au llieu de 2, mais bon le tableau generalement est trs petite donc cette manipulation matebanche :p)
    for(var val in list_solution_1error)
    {
        listSolution.list_solution_1error.push(list_solution_1error[val]);
    }

    return listSolution;

};

// approximate search with 1 error
// if there is a common prefix between the two query word, we start the search from node in this level (depth of common prefix)
function search_Prefix_by_Suffix_Delete_1error(pattern, lengPrefixCommon, trie_info)
{   
    var line_table = trie_info.line_table,
        trie = trie_info.trie,
        longest_line = trie_info.longest_line,
        firstNode = trie_info.first_node,
        listSolution = new exact_and_1err_Solution_nodes(), // a list containing all solutions nodes : exact (if we have) and with error
        list_solution_1error={}, // hash_table
        i=0,
        p=0,
        k=0,
        nb_sequence=0,
        nb_trans=0,
        pointer_sequence=0,
        pointed_node=0,
        length_transition=0,
        pos_code=firstNode,
        last_pointed_node = pointed_node =firstNode,
        isExactSolutionFound =false,
        nbChar_rest_to_test_inTransition = 0,
        nb_letters=pattern.length;

    if(nb_letters===0)
    {
        lasteQueryWord_size=0;
        pos_last_node_char_tested =0;
        pos_LastNode = firstNode;
        idx_path=0;

        return listSolution;
    }
    
    if(tab_nodes_way===null)
    {
        tab_nodes_way= new Array(longest_line);
    }

    if(branch_charLevels===null)
    {
        branch_charLevels = new Array(longest_line+1);//Stores the branching levels
        branch_trans_num = new Array(longest_line+1);//Stores the transition num of the branching character
        branch_nodes = new Array(longest_line+1);// Stores the branching nodes
        branch_node_trans_chunk = new Array(longest_line+1);// Stores a pointer to the chunk representing the branching node in its parent
    }


    if(lengPrefixCommon === 0)
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
        if(lengPrefixCommon === lasteQueryWord_size)
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

                while(tab_nodes_way[tab_nodes_way_counter]===-1)
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
                    if(tab_nodes_way[tab_nodes_way_counter]!==-1)
                    {
                        idx_path++;
                    }
                }
            }
    }



    while (k<nb_letters)
    {
        branch_nodes[idx_path] = pos_code;

                /*
        pos_code += 1; // 1: NUM_OCTETS_RESERVED_BY_NODE; // skip the space of the score topK
        nb_sequence = trie[pos_code++]; //get sequence number and advance in the trie, go to the next information
        */
        pos_code += 2; // 1: NUM_OCTETS_RESERVED_BY_NODE && 1 : sequence number, skip theme both
        
        nb_trans = trie[pos_code++];
        
        for (i=0; i<nb_trans; i++)
        {   
            if(pattern.charAt(k) === trie[pos_code]) break;

            pos_code += 1; // go to the character of the next transition
        }


        branch_charLevels[idx_path]=k;

        branch_trans_num[idx_path]=i;

        if (i===nb_trans)
        {
            branch_node_trans_chunk[idx_path++]=pos_code;
            break;
        }
        else
        {
            p =i;

            pos_code += (nb_trans - i);

            branch_node_trans_chunk[idx_path++]=pos_code;

            pos_code += i*3; 
            
            
            pointer_sequence = trie[pos_code++]; 
            length_transition = trie[pos_code++];
            pointed_node = trie[pos_code++];     
            
            nb_sequence = trie[pointed_node+1]; 
                                                
            k += 1; 
            pointer_sequence += 1; 
            length_transition -= 1;

            p=0;
            while (p<length_transition && k<nb_letters)
            {

                if (pattern.charAt(k)!==line_table[nb_sequence].charAt(pointer_sequence)) break;

                p += 1;
                k += 1;
                pointer_sequence += 1;
            }

            if (p===length_transition)
            {
                pos_code=pointed_node;

                last_pointed_node = pointed_node;

                pos_last_node_char_tested=k;

                tab_nodes_way_counter=k;
                tab_nodes_way[tab_nodes_way_counter] =   pos_code;
            }
            else
            {
                if (k===nb_letters) break;

                lasteQueryWord_size=nb_letters;
                isSolutionExist = false;
                pos_last_node_char_tested = 0;
                pos_LastNode = firstNode;

                break;
            }
        }
    }

    if (k===nb_letters)
    {
        lasteQueryWord_size=k;
        pos_LastNode=last_pointed_node;

        isSolutionExist = true;
        isExactSolutionFound = true;

        /// listSolution.push(pointed_node);
        listSolution.exact_solution = pointed_node;
    }
    else
    {
        isSolutionExist = false;
        lasteQueryWord_size = nb_letters;

        pos_last_node_char_tested = 0;
        pos_LastNode = firstNode;

    }


    nbChar_rest_to_test_inTransition = length_transition-p;

    test_OneError(trie_info,pattern,nb_letters,k,pointer_sequence,nbChar_rest_to_test_inTransition,pos_code,pointed_node,list_solution_1error,isExactSolutionFound);

    // recupirer les resulta et es mettre dans un tableau normale (bon on peut ne pas faire sa, et on travil avec se hash tableau partout donc meme dans les autres fonction, bon il y a juste une seule etuilisation celle de retrouver les resultat, donc on fait un seule parcoure au llieu de 2, mais bon le tableau generalement est trs petite donc cette manipulation matebanche :p)
    for(var val in list_solution_1error)
    {
        listSolution.list_solution_1error.push(list_solution_1error[val]);
    }
    
    return listSolution;
};


// ========================================================================\\
// ----------------- get list words


var my_heap = null; // to use it in wordList_TopK_byGroup, cause we call this function sevreal time

// use those var in wordList_exact_1err_TopK_byGroup
var still_exact_solution = true, // extract the exat solution and then we go to the approximate solution
    my_heap_err,
    list_words_hash_err; // on fait sa, pour que meme es resultat de next ne se deplique pas ;) cela vaut dir que il ne sont pas déja sorti dans les groupe precedent ;)

// compare 2 node by there score
// if there scor is equal so we compare theme by there str
function compare(a, b) 
{
    if( a.val > b.val) return 1; // decroissant
    else
        if( a.val < b.val) return -1; // decroissant
        else
            return b.str.localeCompare(a.str); // on change le b par a, car on vaudri un ordre alphabitique donc croissant
};

// node information for the topK
// val : topK  score
// data : node number
// str : string of this node
function item(val,data,str)
{
    this.val = val;
    this.data = data;
    this.str = str;
};


// _____________________________________________________________________________//
// ------------------------------ exact solution -------------------------------//

// get solution words (going through the subtree at the end of the transition)
//  it's like search function, but we don't make comparison, and we add the transition child in reverse way to stack (to order of result lexecugraphy)
function wordList(trie_info, Last_pointed_node)
{   
    var line_table = trie_info.line_table,
        longest_line = trie_info.longest_line,
        trie = trie_info.trie,
        limit_leaf = trie_info.limit_leaf,
        list_words = [],
        k2,
        nb_sequence,
        nb_trans,
        pos_code,
        file_last_nodes = new Array(longest_line),
        file_last_nodes_counter = 0,
        save_pos_code;

    file_last_nodes[0] = Last_pointed_node;

    while (file_last_nodes_counter>=0)
    {
        pos_code=file_last_nodes[file_last_nodes_counter];

        if (pos_code<limit_leaf)
        {
            nb_sequence= trie[pos_code+1]; // 1 : NUM_OCTETS_RESERVED_BY_NODE

            list_words.push(line_table[nb_sequence]); // save the word of this node

            file_last_nodes_counter--;
        }
        else
        {   
            /*
            pos_code += 1;// 1 : NUM_OCTETS_RESERVED_BY_NODE;
            nb_sequence = trie[pos_code++];
            */

            pos_code += 2; // 1 : topK, 1 : num de sequence
            nb_trans = trie[pos_code++];

            /*
            for (k2=0; k2<nb_trans; k2++)
            {
                pos_code += u8_seqlen((char*)(trie+pos_code));
            }*/
            pos_code += nb_trans; // deppasser les caractère (first char of each transition)

            file_last_nodes_counter--; // on a terminer avec ce node, doka on va traier ces fils

            /*
            save_pos_code = pos_code + (3*nb_trans);
            for (k2=0;k2<nb_trans;k2++)
            {
                // pointer_sequence = trie[pos_code++];
                // length_transition = trie[pos_code++];
                pos_code +=2;
                Last_pointed_node = trie[pos_code++];

                file_last_nodes[++file_last_nodes_counter] = Last_pointed_node;
            }
            console.log("pos_code = "+pos_code+" , save_pos_code_fin = "+save_pos_code);
            */
            
           
           
            // ajouter les node d'une façon inverse, on commence depuis the last node child, 
            // on fait sa pour que les result seront selon l'ordre alphabitique (car on a une pile :)
            pos_code += nb_trans *3;
            save_pos_code = pos_code;
            pos_code -=1;
            for (k2=0;k2<nb_trans;k2++)
            {
                Last_pointed_node = trie[pos_code--];
                
                pos_code -=2;

                file_last_nodes[++file_last_nodes_counter] = Last_pointed_node;
            }
            // remettre pos_code a sa place :) donc a la fin des info des nodes transition
            pos_code = save_pos_code;
            
        }
    }
    
    return list_words;
};

// get solution words (with their information), words are ordered by their score topK
function wordList_TopK(trie_info, Last_pointed_node)
{   
    var line_table = trie_info.line_table,
        trie = trie_info.trie,
        limit_leaf = trie_info.limit_leaf,
        list_words =[],
        k2,
        nb_sequence,
        nb_trans,
        pos_code,
        my_heap = new Heap (compare);
    
    my_heap.add_item(new item(trie[Last_pointed_node],Last_pointed_node,line_table[trie[Last_pointed_node+1]])); // add the first node to the heap sort, topK && node

    while (my_heap.get_size()>0)
    {
        pos_code = my_heap.get_top().data;

        if (pos_code<limit_leaf)
        {            
            nb_sequence= trie[pos_code+1];
            
            list_words.push(line_table[nb_sequence]); // save the word of this node

            my_heap.delete_top();
        }
        else
        {    
            // pos_code += 1; // 1 : NUM_OCTETS_RESERVED_BY_NODE
            // nb_sequence = trie[pos_code++];
            

            pos_code += 2; // 1 : topK, 1 : num de sequence
            nb_trans = trie[pos_code++];
            
            pos_code += nb_trans; // deppasser les caractère (first char of each transition)

            my_heap.delete_top(); // on a terminer avec ce node, doka on va traier ces fils

            // ajouter les node d'une façon normale, car le heap se comporte comme une file dans notre cas car A>B>C...>Z et les node sont ordonnerdans le trie
            // comme sa on va iviter les swap lors de l'insrtion
            for (k2=0;k2<nb_trans;k2++)
            {
                //pointer_sequence = trie[pos_code++];
                //length_transition = trie[pos_code++];
                pos_code+=2;
                Last_pointed_node = trie[pos_code++];
                
                my_heap.add_item(new item(trie[Last_pointed_node],Last_pointed_node,line_table[trie[Last_pointed_node+1]]));
            }
        }
    }
    
    return list_words;
};

// get solution words (with their information), words are ordered by their score topK
// each time we return a group of words (size == topK_Element)
function wordList_TopK_byGroup(trie_info, Last_pointed_node, topK_Element, isFirstCall)
{
    // ibra : on  peut sortir tous ces variable en dore de function pour eviter la declaration a chaque foi, car on appell cette function +eurs fois, 
    var line_table = trie_info.line_table,
        trie = trie_info.trie,
        limit_leaf = trie_info.limit_leaf,
        list_words = [],
        k2,
        nb_sequence,
        nb_trans,
        pos_code,
        idx_TopK; //to calculate the number of words to return in a single group

    if(isFirstCall) //initialize, and add the first node to the "prio_queue".
    {
        my_heap = new Heap (compare);          
        my_heap.add_item(new item(trie[Last_pointed_node],Last_pointed_node,line_table[trie[Last_pointed_node+1]])); // add the first node to the heap sort, topK && node
    }

    idx_TopK = 0; //to calculate the number of words to return in a single group

    while (my_heap.get_size()>0 && idx_TopK < topK_Element)
    {
        pos_code = my_heap.get_top().data;

        if (pos_code<limit_leaf)
        {
            nb_sequence= trie[pos_code+1];
            
            list_words.push(line_table[nb_sequence]); // save the word of this node

            idx_TopK++;

            my_heap.delete_top();
        }
        else
        {
            // pos_code += 1;
            // nb_sequence = trie[pos_code++];
           
           pos_code += 2;

            nb_trans = trie[pos_code++];

            pos_code += nb_trans;

            my_heap.delete_top();

            
            for (k2=0;k2<nb_trans;k2++)
            {
                //pointer_sequence = trie[pos_code++];
                //length_transition = trie[pos_code++];
                pos_code+=2;
                Last_pointed_node = trie[pos_code++];

                my_heap.add_item(new item(trie[Last_pointed_node],Last_pointed_node,line_table[trie[Last_pointed_node+1]]));
            }
        }
    }

    return list_words;
};


// _____________________________________________________________________________//
// ----------------------- exact && 1err solution ------------------------------//

// get all solution words  from a list of solution node
function wordListe_exact_1err(trie_info, listSolutionNodes)
{
    var line_table = trie_info.line_table,
        longest_line = trie_info.longest_line,
        trie = trie_info.trie,
        limit_leaf = trie_info.limit_leaf,
        k2,
        nb_sequence,
        nb_trans,
        pos_code,
        file_last_nodes = new Array(longest_line),
        file_last_nodes_counter,
        save_pos_code,
        list_words =[],
        list_words_hash={},
        i=0,
        Last_pointed_node,
        sizeListSolutionNodes = listSolutionNodes.list_solution_1error.length;


    if(listSolutionNodes.exact_solution>=0) // dans la solution exact on a pas besoin de verifier si les mot exist ou non (bon je pense, mais je ne sais pas si après on peut avoir des solution approuchée qui sont déjà dans l'exact et donc on doit aussi inclure sa la table de hashage, bon il suffi just d'ajouter le node exact en primier dans la liste des err ensuit faire la boucle for)
    {
        list_words = wordList(trie_info, listSolutionNodes.exact_solution); 
    }
    

    //  extraire les resulta de la recherrche approuchée
    for(i=0; i<sizeListSolutionNodes; i++)
    {
        
        file_last_nodes_counter = 0;
        file_last_nodes[0] = listSolutionNodes.list_solution_1error[i];

        while (file_last_nodes_counter>=0)
        {
            pos_code=file_last_nodes[file_last_nodes_counter];

            if (pos_code<limit_leaf)
            {
                nb_sequence= trie[pos_code+1]; // 1 : NUM_OCTETS_RESERVED_BY_NODE

                /// list_words.push(line_table[nb_sequence]); // save the word of this node
                
                if(!list_words_hash[line_table[nb_sequence]])
                {
                    list_words_hash[line_table[nb_sequence]] = line_table[nb_sequence];// add the word to the hash table
                    list_words.push(line_table[nb_sequence]); // save the word of this node
                }

                file_last_nodes_counter--;
            }
            else
            {   
                pos_code += 2; // 1 : topK, 1 : num de sequence
                nb_trans = trie[pos_code++];

                pos_code += nb_trans; // deppasser les caractère (first char of each transition)

                file_last_nodes_counter--; // on a terminer avec ce node, doka on va traier ces fils


                // ajouter les node d'une façon inverse, on commence depuis the last node child, 
                // on fait sa pour que les result seront selon l'ordre alphabitique (car on a une pile :)
                pos_code += nb_trans *3;
                save_pos_code = pos_code;
                pos_code -=1;
                for (k2=0;k2<nb_trans;k2++)
                {
                    Last_pointed_node = trie[pos_code--];

                    pos_code -=2;

                    file_last_nodes[++file_last_nodes_counter] = Last_pointed_node;
                }
                // remettre pos_code a sa place :) donc a la fin des info des nodes transition
                pos_code = save_pos_code;

            }
        }

    }
    
   return list_words;
};

// get all solution words (with their information),  from a list of solution node. words are ordered by their score topK
function wordList_exact_1err_TopK( trie_info, listSolutionNodes)
{
    var line_table = trie_info.line_table,
        trie = trie_info.trie,
        limit_leaf = trie_info.limit_leaf,
        k2,
        nb_sequence,
        nb_trans,
        pos_code,
        my_heap=new Heap(compare),
        list_words =[],
        Last_pointed_node,
        list_words_hash={};
    
    
    if(listSolutionNodes.exact_solution>=0)
    {
        list_words = wordList_TopK(trie_info, listSolutionNodes.exact_solution); 
    }

    
    var sizeListSolutionNodes = listSolutionNodes.list_solution_1error.length,
        indice_listSolutionNodes = 0;
    
    // la aussi je pense que je doit les empiler à l'inverse
     for(indice_listSolutionNodes=0; indice_listSolutionNodes < sizeListSolutionNodes ; indice_listSolutionNodes++)
    {
        Last_pointed_node = listSolutionNodes.list_solution_1error [indice_listSolutionNodes];
        my_heap.add_item(new item(trie[Last_pointed_node],Last_pointed_node,line_table[trie[Last_pointed_node+1]]));
    }

    while (my_heap.get_size()>0)
    {
        pos_code = my_heap.get_top().data;

        if (pos_code<limit_leaf)
        {            
            nb_sequence= trie[pos_code+1];
            
            /// list_words.push(line_table[nb_sequence]); // save the word of this node
            
            if(!list_words_hash[line_table[nb_sequence]])
            {
                list_words_hash[line_table[nb_sequence]] = line_table[nb_sequence];// add the word to the hash table
                list_words.push(line_table[nb_sequence]); // save the word of this node
            }

            my_heap.delete_top();
        }
        else
        {
            
            // pos_code += 1; // 1 : NUM_OCTETS_RESERVED_BY_NODE
            // nb_sequence = trie[pos_code++];
            

            pos_code += 2; // 1 : topK, 1 : num de sequence
            nb_trans = trie[pos_code++];
            
            pos_code += nb_trans; // deppasser les caractère (first char of each transition)

            my_heap.delete_top();

            
            for (k2=0;k2<nb_trans;k2++)
            {
                //pointer_sequence = trie[pos_code++];
                //length_transition = trie[pos_code++];
                pos_code+=2;
                Last_pointed_node = trie[pos_code++];
                
                my_heap.add_item(new item(trie[Last_pointed_node],Last_pointed_node,line_table[trie[Last_pointed_node+1]]));
            }
        }
    }

    return list_words;
};

// get all solution words (with their information),  from a list of solution node. words are ordered by their score topK
// each time we return a group of words (size == topK_Element)
function wordList_exact_1err_TopK_byGroup(trie_info, listSolutionNodes, topK_Element, isFirstCall)
{
    var line_table = trie_info.line_table,
        trie = trie_info.trie,
        limit_leaf = trie_info.limit_leaf,
        sizeListSolutionNodes = listSolutionNodes.list_solution_1error.length,
        indice_listSolutionNodes,
        list_words = [],
        k2,
        nb_sequence,
        nb_trans,
        pos_code,
        Last_pointed_node,
        idx_TopK = 0; //to calculate the number of words to return in a single group
    
    
    if(isFirstCall) //initialize, and add the first node to the "prio_queue".
    {   
        still_exact_solution = true;
        my_heap_err = new Heap (compare);          
        list_words_hash_err={};
        
        for(indice_listSolutionNodes=0; indice_listSolutionNodes < sizeListSolutionNodes ; indice_listSolutionNodes++)
        {
            Last_pointed_node = listSolutionNodes.list_solution_1error [indice_listSolutionNodes];
            my_heap_err.add_item(new item(trie[Last_pointed_node],Last_pointed_node,line_table[trie[Last_pointed_node+1]]));
        }
    }
    
    if(still_exact_solution)
    {
        if(listSolutionNodes.exact_solution>=0)
        {
            list_words  =wordList_TopK_byGroup(trie_info, listSolutionNodes.exact_solution, topK_Element, isFirstCall);
            
            if(list_words.length<topK_Element) // this means that the words number in the list is unless of the number topk_element, so chod go to treat the node of 1err result
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
    
    while (my_heap_err.get_size()>0 && idx_TopK < topK_Element)
    {
        pos_code = my_heap_err.get_top().data;

        if (pos_code<limit_leaf)
        {
            nb_sequence= trie[pos_code+1];

            /// list_words.push(line_table[nb_sequence]); // save the word of this node
            if(!list_words_hash_err[line_table[nb_sequence]])
            {
                list_words_hash_err[line_table[nb_sequence]] = line_table[nb_sequence];// add the word to the hash table
                list_words.push(line_table[nb_sequence]); // save the word of this node
            }

            idx_TopK++;

            my_heap_err.delete_top();
        }
        else
        {
            // pos_code += 1;
            // nb_sequence = trie[pos_code++];
           
           pos_code += 2;

            nb_trans = trie[pos_code++];

            pos_code += nb_trans;

            my_heap_err.delete_top();

            for (k2=0;k2<nb_trans;k2++)
            {
                //pointer_sequence = trie[pos_code++];
                //length_transition = trie[pos_code++];
                pos_code+=2;
                Last_pointed_node = trie[pos_code++];

                my_heap_err.add_item(new item(trie[Last_pointed_node],Last_pointed_node,line_table[trie[Last_pointed_node+1]]));
            }
        }
    }

    return list_words;
};

// ========================================================================\\
// ----------------- Heap
function Heap(compare)
{
	
	if(!compare)
	{
		Heap.prototype.compare= function(a, b) 
		{
			return a - b; // croissant
		   // return b - a; // decroissant
		};
	}
	else
		Heap.prototype.compare = compare;

    this.buffer= new Array();
	

    // free the space memory occupied by our Heap
    Heap.prototype.free_space = function ()
    {
        this.buffer=null;
    };
	


    Heap.prototype.swap = function (idx1,idx2)
    {
        var tmp = this.buffer[idx1];
        this.buffer[idx1]  = this.buffer[idx2];
        this.buffer[idx2]  = tmp;
    };

    Heap.prototype.left_child=function (i)
    {
        return (i+1)*2-1;
    };

    Heap.prototype.right_child=function ( i )
    {
        return (i+1)*2;
    };

    Heap.prototype.parent=function ( i )
    {
        return Math.floor( (i+1)/2-1 );
    };

    // insert a new element in the Heap
    Heap.prototype.add_item=function (item)
    {
        var next_pos,
            pos;

        // insert the new element
        pos=this.buffer.length;
        
	this.buffer.push(item);
		
        // re-equilibrate the tree
        while(pos!==0)
        {
            next_pos=this.parent(pos);

            if(this.compare(this.buffer[next_pos],this.buffer[pos])>0)
                 break;

            this.swap(pos,next_pos);
            pos=next_pos;
        }
    };

    // remove the element that is at the top
    Heap.prototype.delete_top=function ()
    {
        var pos,
            next_pos,
            nnext_pos;

        //
        if(this.buffer.length<=1)
        {
            this.buffer.length=0;
            return ;
        }

        this.buffer[0] = this.buffer[this.buffer.length-1];
		this.buffer.pop();

        // re-equilibrate the tree
        pos=0;
        for(;;)
        {
            next_pos=this.left_child(pos);

            if((next_pos < this.buffer.length) && (this.compare(this.buffer[next_pos] , this.buffer[pos])>0))
            {
                nnext_pos=this.right_child(pos);

                if(nnext_pos>=this.buffer.length)
                {
                    this.swap(pos,next_pos);
                    break;
                }

                
                if(this.compare(this.buffer[nnext_pos] , this.buffer[next_pos])>0)
                {
                    this.swap(pos,nnext_pos);
                    next_pos=nnext_pos;
                }
                else
                    this.swap(pos,next_pos);
            }
            else
            {
                next_pos=this.right_child(pos);

                if((next_pos < this.buffer.length) && (this.compare(this.buffer[next_pos] , this.buffer[pos])>0))
                {
                    this.swap(pos,next_pos);
                }
                else
                    break;
            }

            pos=next_pos;
        }
    };

    // return the first element (that is at the top)
    Heap.prototype.get_top=function ()
    {
        if(this.buffer)
            return this.buffer[0];
        return null;
    };

    // return the size of the Heap
    Heap.prototype.get_size=function ()
    {
        if(this.buffer)
            return this.buffer.length;
        return 0;
    };

};


}

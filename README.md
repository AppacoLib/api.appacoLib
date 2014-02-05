appacoLib : 
=========
	
APProximate AutoComplete Library
	
Appaco_Lib is library which facilitates and speeds up writing by offering a list of words (suggestions) that complement the few characters typed into the input text, while allowing errors in the prefix entered by the user, all by improving the quality of the results with ranking system, according to the importance (score) of words.
		And it’s support static and dynamic ranking.


... <a href="https://github.com/AppacoLib/api.appacoLib/wiki/Detailed-Description" title="details" target="_blank">see details </a>
	

	

Appaco_lib features:
====================
•	exact autocomplete : list of suggestion contain only words that begin by the same prefix typed in text box.
•	approximate autocomplete : we allow error in prefix typed in text box, it's mean we show word that begin by the same prefix and also the word that begin by prefix similar to the prefix typed.
•	static ranking : words will be displayed according to the importance of words.
•	dynamic ranking : static ranking + update the score of the selected word (example +1)
•	local support for dictionary source, it support huge number of words all in local (example English dictionary).
•	Ajax query to get the results from server (or sometimes the dictionary source file)
•	very fast response (trie based).
•	sport one character typed to get results (use the ranking system)

	

Supported language:
====================
appaco_lib is coded in two language : javascript and c/c++.

1) JS appaco_lib  : used in client side, do all the work locally, and it can get result from server side.

2) C appaco_lib : can be used in any type of application, and also we can use it in a server side using the module FastCGI.
	
those two library can be used separately.
and also we can use them both in the same context, js version in client side and c/c++ on server side.



JS appaco_lib
=============
<a href="#" title="details">	approximate_autoComplete(input_id,source,options_in)</a> <br/>
<a href="#" title="details">    typeSearch</a> <br/>
<a href="#" title="details">    typePrintResults</a> <br/>
<a href="#" title="details">    isExactSearchMethod</a> <br/>
<a href="#" title="details">    topK_CHAR_SEPARATOR</a> <br/>
<a href="#" title="details">    words_SEPARATOR  </a><br/>
<a href="#" title="details">    doSort</a><br/>
<a href="#" title="details">    TOPK_SIZE_GROUP</a><br/>
<a href="#" title="details">    delay</a><br/>
<a href="#" title="details">    minLength</a><br/>
<a href="#" title="details">    sourceFromUrl</a><br/>
<a href="#" title="details">    resultsFromUrl</a><br/>
<a href="#" title="details">	score_update_unit</a><br/>
<a href="#" title="details">	item_ID_OnClick_nextResult</a><br/>
<a href="#" title="details">	dataToSend  </a><br/>
<a href="#" title="details">	ajaxRequestMethod</a><br/>

C appaco_lib
============
<a href="#" title="details">	Options* new_options();  </a><br/>
<a href="#" title="details">	void print_options(Options *my_option);  </a><br/>
<a href="#" title="details">	void initialize_AutoComplete(const char *source,Options *my_option_in);  </a><br/>
<a href="#" title="details">	ListString* get_results(char* queryWord);  </a><br/>
<a href="#" title="details">	ListString *get_next_result();  </a><br/>
<a href="#" title="details">	void Print_ListString(ListString *list);  </a><br/>
<a href="#" title="details">	void json_Print_ListString(ListString* list);  </a><br/>
<a href="#" title="details">	typedef struct Options {...} Options;  </a><br/>


examples
========

example 1 : <a href="source_from_local.html" title="source from local file" target="_blank">source from local file</a>
		
example 2 : <a href="source_from_server.html" title="source form server" target="_blank">source from server file</a>
		
example 3 : <a href="result_from_server_CGI.html" title="result from server" target="_blank">result from server CGI</a>
		
example 4 : <a href="result_from_server_PHP.html" title="result from server" target="_blank">result from server PHP</a>

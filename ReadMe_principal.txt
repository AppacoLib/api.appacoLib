
tous les dossier et les sous dossier contient des ReadMe qui explique le contenue de chaque dossier/fichier.

dans ce ReadMe on explique le contenue de chaque dossier :

appaco_lib :
	le code source de la bibliothèque d'autocompletion (approximate autocompletio librarry)

___________________________________________________________________________________________	

C_appaco_lib_exemple:
	des differants exemple en utilisant la bibliothèque appaco_lib.
	-les example qui utilise  derecetement le code source de appaco_lib.
	-des exemple qui utilise juste les header et la library static .a
	les exemple sont pour windows/linux  avec (makefile, Code blocks, Qt IDE ), en console et en gui interface graphic, et aussi en mode cgi FastCgi pour le coté serveur.
___________________________________________________________________________________________	

C_staticLib_include:
	ce dossier contient la version de la bibliothèque appaco_lib compiler vers une library static .a, et aussi le dossier qui contien les headers.
	
	on peut generer la bibliothèque static automatiquement selon le system:
	le dossier racine qui contient le code source appaco_lib,
	il contient un makefile qui compile la lib à une lib static .a

___________________________________________________________________________________________	

JS_appaco_lib:
	la bibliothèque appaco_lib en JavaScript.
	le fichier bibiothèque : approximate_autoComplete.js  , sa taille est environ 145 KO, et son fichier css : approximate_autoComplete.css

___________________________________________________________________________________________	

JS_appaco_lib_exemple :
	il contient different types d'exemples
	des exemple où le dictionaire est en local, aussi lorsque il est dans le server
	des exmple lorsque le calcule et la recherche se fait avec notre librarry en C en mode FastCgi dans le serveur et les resulat sont afficher dans le cotée client en javascript.
	
	le fichier : index.html  contient une petit decreption sur appaco_lib et des liens vers les exemples.
	le fichier : doc_appaco_lib.html contient une full documentation sur tout la library en C et en JS.
	
___________________________________________________________________________________________	

doc_appaco_lib:
	documentation en anglais de la "appaco_lib", explication et exemples.

example 1 :use_static_appaco_lib

	cette exemple est réalisé avec le IDE Code Blocks.

	on a une application console, on utilise la library static .a ,  donc le projet on appaco_lib.a et aussi le dossier include qui contient les header de la bibliothèque.

	dans l'IDE code blocks, il suffit juste d'ajouter appaco_lib.a au projet 

	pour cela :
	aller dans projet --> build options --> dans linker settings , add (aller dans le dossier qui contient appaco_lib.a) et clicker sur ajouter, ensuit clicker "non" dans la fenêtre qui sort et dit : garder le chemin relatif.

exemple 2: qt_gui_exemple 
	exemple avec interface graphic avec Qt framework.

	dans cette exemple on créer une application gui avec Qt, qui utilise la bibliothèque static libappaco_lib.a et les header qui sont dans le dossier include.
	
	on ajute la lib au fichier .pro : LIBS += -L. -lappaco_lib

	l'interface contient tous les fonctionnalité de la bibliothèque : recherche exact, avec erreur, TopK, type de recherche dans le trie, le temps d'afficher les resultat.

	le Qt est portable dans tout les platform, donc il suffit just d'ouvrir le projet et recompiler dans (linux,windows,Mac).

	l'interface contient un champ de saisie de requète et les resultat s'affiche automatiquement en fur et à mesure que on tape la requete.

	cette application graphique, est le meilleur exemple pour tester et voir tout les fonctionnalités de notre bibliothèque.
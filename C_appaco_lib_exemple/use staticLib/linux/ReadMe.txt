
on a deux type d'exemple realiser avec un : 1) makefile, 2) Qt IDE , 3) Code Blocks IDE

tous es exemple son tester dans linux ubuntu 14,04 LTS.

------------------------------------------------------------------------------------
-----------------------           Makefile          --------------------------------

exemple 1: makefile_use_static_appaco_lib
	
	on a une application console, on utilise la library static .a  ,  dans le projet on ajoute appaco_lib.a et aussi le dossier include qui contient les header de la bibliothèque.

	pour utiliser la library :
	dans le code source on ajoute le header : #include "include/approximate_autocomplete.h" qui referenace drecetement dans le dossier local.
	
	et dans le makefile on ajoute : -L. -lappaco_lib dans la parti libs pour referencer la libappaco_lib.a , (-L. pour dire que la library se trouve dans le meme dossier).
_________________________________________________	
exemple 2 : makefile_use_static_appaco_lib_system

	dans cette exmple les fichier header et la library static sont installer dans le system.
	pour les include (usr/local/include)  et la library (usr/local/lib).

	dans le code source on ajoute le header : #include "approximate_autocomplete.h", le compilateur va chercher automatiquement les header dans leur emplacement dans le system. et c'est la meme chose pour la library .a
	dans le makefile on ajoute que le nom de la library -lappaco_lib dans la partie libs.

------------------------------------------------------------------------------------
-----------------------           Code Blocks       --------------------------------

exemple 3: cbp_use_appaco_lib

	cette exemple est réalisé avec le IDE Code Blocks.
	on a une application console, on utilise la library static .a ,  donc le projet on appaco_lib.a et aussi le dossier include qui contient les header de la bibliothèque.

	dans l'IDE code blocks, il suffit juste d'ajouter appaco_lib.a au projet 

	pour cela :
	aller dans projet --> build options --> dans linker settings , add (aller dans le dossier qui contient appaco_lib.a) et clicker sur ajouter.
______________________________________	
exemple 4: cbp_use_appaco_lib_system
meme chose, mais les include et la bibliothèque son installer dans le system.

------------------------------------------------------------------------------------
-----------------------             Qt IDE          --------------------------------

exemple 5: qt_use_appaco_lib
exactement la meme chose que exemple 3, réaliser avec IDE Qt.
ici on ajout la lib au fichier .pro : LIBS += -L. -lappaco_lib

_______________________________________________
exemple 4: qt_use_appaco_lib_from_system
meme chose, (les header et la lib sont dans le system)
ici on ajout juste le nom de la lib dans le fichier .pro : LIBS += -lappaco_lib

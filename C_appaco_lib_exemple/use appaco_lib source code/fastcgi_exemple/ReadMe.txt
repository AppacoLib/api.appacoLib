FastCgi exemple :

dans cette exemple on cr�er une application FastCGI qui s'execute dans le server.
Le dictionnaire est crier localement seulement une fois lorsqu'on lance l'aplication.

dans cgi a chaque fois qu'on a une requ�te, la structure de donn�e et recr�er. 
Le code sura executer de nouveau de debut � la fin, comme une page .php

elle attend des requ�te http, apr�s le traitement, les resultats sont envoyer o� o� client.

on utiliser directement le code source de la library.
le code source de la biblioth�que se trouve dans "../appaco_lib"

on a un MakeFile qui compile le tout , l'exemple et le code de la biblioth�que en meme temps.
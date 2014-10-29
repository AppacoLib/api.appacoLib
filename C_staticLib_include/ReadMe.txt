ce dossier contient la version de la bibliothèque appaco_lib compiler vers une library static .a

linux : la appaco_lib.a  compiler sous linux ubuntu.
windows : la même chose compiler sous windows 7.

le dossier include qui contient tous les header de appaco_lib.

on peut generer la bibliothèque static automatiquement selon le system:
le dossier racine qui contient le code source appaco_lib,
il contient un makefile qui compile la lib à une lib static .a  (libappaco_lib.a)
et aussi il met les herder dans usr/local/include  et la libe dans usr/local/lib

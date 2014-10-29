#-------------------------------------------------
#
# Project created by QtCreator 2012-07-02T18:28:15
#
#-------------------------------------------------

QT       += core gui

TARGET = interface_searsh_preffixe
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ../appaco_lib/utf8.cpp \
    ../appaco_lib/trie_coding.cpp \
    ../appaco_lib/query_trie_editDistance_1error.cpp \
    ../appaco_lib/query_trie.cpp \
    ../appaco_lib/my_List_c.cpp \
    ../appaco_lib/integer_coding.cpp \
    ../appaco_lib/heapSort.cpp \
    ../appaco_lib/hash_fonction.cpp \
    ../appaco_lib/get_listWord.cpp \
    ../appaco_lib/approximate_autocomplete.cpp

HEADERS  += mainwindow.h \
    ../appaco_lib/utf8.h \
    ../appaco_lib/trie_coding.h \
    ../appaco_lib/query_trie_editDistance_1error.h \
    ../appaco_lib/query_trie.h \
    ../appaco_lib/my_List_c.h \
    ../appaco_lib/integer_coding.h \
    ../appaco_lib/heapSort.h \
    ../appaco_lib/hash_fonction.h \
    ../appaco_lib/get_listWord.h \
    ../appaco_lib/define.h \
    ../appaco_lib/approximate_autocomplete.h



FORMS    += mainwindow.ui

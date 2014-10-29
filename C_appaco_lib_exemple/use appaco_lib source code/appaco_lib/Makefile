CC            = gcc
CXX           = g++
CFLAGS=-c -Wall
CXXFLAGS=-c -Wall
LDFLAGS=

INSTALL_PATH = /usr/local
####### Output directory


OUTPUT_LIB=libappaco_lib.a

####### Files

SOURCES       = utf8.cpp \
		trie_coding.cpp \
		query_trie_editDistance_1error.cpp \
		query_trie.cpp \
		my_List_c.cpp \
		integer_coding.cpp \
		heapSort.cpp \
		hash_fonction.cpp \
		get_listWord.cpp \
		approximate_autocomplete.cpp \

HEADERS       = utf8.h \
		trie_coding.h \
		query_trie_editDistance_1error.h \
		query_trie.h \
		my_List_c.h \
		integer_coding.h \
		heapSort.h \
		hash_fonction.h \
		get_listWord.h \
		approximate_autocomplete.h \
		define.h \

OBJECTS=$(SOURCES:.cpp=.o)
		

$(OUTPUT_LIB): $(OBJECTS)
	ar rc $(OUTPUT_LIB) $(OBJECTS)
	ranlib $(OUTPUT_LIB)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $< -o $@


clean:
		rm -rf *.o $(OUTPUT_LIB)

install:
	install -m 644 $(OUTPUT_LIB) $(INSTALL_PATH)/lib
	install -m 644 $(HEADERS) $(INSTALL_PATH)/include

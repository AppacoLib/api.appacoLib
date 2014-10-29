CC            = gcc
CXX           = g++
CFLAGS=-c -Wall
CXXFLAGS=-c -Wall
LDFLAGS=

####### Output directory

OBJECTS_DIR   = release
EXECUTABLE=approximate_autocomplete

####### Files

SOURCES       = main.cpp \
		MainConsol.cpp \
		../appaco_lib/utf8.cpp \
		../appaco_lib/trie_coding.cpp \
		../appaco_lib/query_trie_editDistance_1error.cpp \
		../appaco_lib/query_trie.cpp \
		../appaco_lib/my_List_c.cpp \
		../appaco_lib/integer_coding.cpp \
		../appaco_lib/heapSort.cpp \
		../appaco_lib/hash_fonction.cpp \
		../appaco_lib/get_listWord.cpp \
		../appaco_lib/approximate_autocomplete.cpp \

OBJECTS=$(SOURCES:.cpp=.o)
		

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $< -o $@


clean:
		rm -rf *.o

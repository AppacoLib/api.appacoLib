
CC            = gcc
CXX           = g++
CFLAGS=-c -Wall
CXXFLAGS=-c -Wall
LDFLAGS= -lfcgi -lfcgi++ -lcgicc -lpthread

####### Output directory

OBJECTS_DIR   = release
EXECUTABLE=approximate_autocomplete
HTMLLIBS=  -I/usr/local/include  -L/usr/local/lib/  -I/usr/include -L/usr/lib 

####### Files

SOURCES       = appacolib_FastCGI.cpp \
		 FCgiIO.cpp \
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
		ccgi.cpp 

OBJECTS=$(SOURCES:.cpp=.o)
		

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

.cpp.o:
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)


clean:
		rm -rf *.o

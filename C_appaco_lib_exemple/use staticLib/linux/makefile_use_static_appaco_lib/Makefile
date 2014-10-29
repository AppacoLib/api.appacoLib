
MAKEFILE      = Makefile

####### Compiler, tools and options

CC            = gcc
CXX           = g++
CFLAGS        = -m64 -pipe -g -Wall -W -D_REENTRANT -fPIE $(DEFINES)
CXXFLAGS      = -m64 -pipe -g -Wall -W -D_REENTRANT -fPIE $(DEFINES)
INCPATH       = 
LINK          = g++
LFLAGS        = -m64
LIBS          = $(SUBLIBS) -L. -lappaco_lib 
AR            = ar cqs
RANLIB        = 
TAR           = tar -cf
COMPRESS      = gzip -9f
COPY          = cp -f
SED           = sed
COPY_FILE     = cp -f
COPY_DIR      = cp -f -R
STRIP         = strip
INSTALL_FILE  = install -m 644 -p
INSTALL_DIR   = $(COPY_DIR)
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE      = rm -f
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE          = mv -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p

####### Output directory

OBJECTS_DIR   = ./

####### Files

SOURCES       = main.cpp \
		MainConsol.cpp 
OBJECTS       = main.o \
		MainConsol.o

TARGET        = use_appaco_lib_local


first: all
####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

all: Makefile $(TARGET)

$(TARGET):  $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)
	{ test -n "$(DESTDIR)" && DESTDIR="$(DESTDIR)" || DESTDIR=.; } && test $$(gdb --version | sed -e 's,[^0-9]\+\([0-9]\)\.\([0-9]\).*,\1\2,;q') -gt 72 && gdb --nx --batch --quiet -ex 'set confirm off' -ex "save gdb-index $$DESTDIR" -ex quit '$(TARGET)' && test -f $(TARGET).gdb-index && objcopy --add-section '.gdb_index=$(TARGET).gdb-index' --set-section-flags '.gdb_index=readonly' '$(TARGET)' '$(TARGET)' && rm -f $(TARGET).gdb-index || true

clean:
	-$(DEL_FILE) $(OBJECTS) $(TARGET)



####### Compile

main.o: main.cpp MainConsol.h \
		include/approximate_autocomplete.h \
		include/define.h \
		include/my_List_c.h \
		include/trie_coding.h \
		include/integer_coding.h \
		include/utf8.h \
		include/query_trie.h \
		include/query_trie_editDistance_1error.h \
		include/hash_fonction.h \
		include/get_listWord.h \
		include/heapSort.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o main.o main.cpp

MainConsol.o: MainConsol.cpp MainConsol.h \
		include/approximate_autocomplete.h \
		include/define.h \
		include/my_List_c.h \
		include/trie_coding.h \
		include/integer_coding.h \
		include/utf8.h \
		include/query_trie.h \
		include/query_trie_editDistance_1error.h \
		include/hash_fonction.h \
		include/get_listWord.h \
		include/heapSort.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o MainConsol.o MainConsol.cpp

####### Install

install:   FORCE

uninstall:   FORCE

FORCE:


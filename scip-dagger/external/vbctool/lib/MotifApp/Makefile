CXX=g++
CXXFLAGS=-g -pedantic -Wno-write-strings
SOURCE_DIR=source
INCL=-Iheaders -I/opt/X11/include

SOURCE_FILES = $(addprefix ${SOURCE_DIR}/,$(notdir $(wildcard ${SOURCE_DIR}/*.C)))
OBJECTS=$(SOURCE_FILES:.C=.o)
LIB_FILE=lib/libMotifApp.a

.PHONY: all clean library

all: library

library: ${LIB_FILE}

${SOURCE_DIR}/%.o: ${SOURCE_DIR}/%.C
	${CXX} ${CXXFLAGS} ${INCL} -c $< -o $@

${LIB_FILE}: ${OBJECTS}
	ar -rcs ${LIB_FILE} ${OBJECTS}

clean:
	rm -f ${OBJECTS}
	rm -f ${LIB_FILE}

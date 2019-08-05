CXX=g++
CXXFLAGS=-g -Wno-write-strings -pedantic
SOURCE_DIR=source
INCL=-Iheaders -I../MotifApp/headers -I/opt/X11/include -I/usr/include/ -I/usr/local/include/

SOURCE_FILES = $(addprefix ${SOURCE_DIR}/,$(notdir $(wildcard ${SOURCE_DIR}/*.cc)))

OBJECTS=$(SOURCE_FILES:.cc=.o) ${SOURCE_DIR}/debug.o  ${SOURCE_DIR}/GFE_C-Code.o
LIB_FILE=lib/libGFE.a

.PHONY: all clean library

all: library

library: ${LIB_FILE}

${SOURCE_DIR}/debug.o: ${SOURCE_DIR}/debug.c
	gcc ${CXXFLAGS} ${INCL} -c $< -o $@

${SOURCE_DIR}/GFE_C-Code.o: ${SOURCE_DIR}/GFE_C-Code.c
	gcc ${CXXFLAGS} ${INCL} -c $< -o $@

${SOURCE_DIR}/%.o: ${SOURCE_DIR}/%.cc
	${CXX} ${CXXFLAGS} ${INCL} -c $< -o $@

${LIB_FILE}: ${OBJECTS}
	ar -rcs ${LIB_FILE} ${OBJECTS}

clean:
	rm -f ${OBJECTS}
	rm -f ${LIB_FILE}

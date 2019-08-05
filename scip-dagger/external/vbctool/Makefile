# C++ compiler
CCC = g++
LINKER = $(CCC)

# Flags
CPP_FLAGS= -ggdb -pedantic -Wno-write-strings -DLINUX

# Directories
GFE_DIR=lib/GFE
GRAPH_INTERFACE_DIR=lib/GraphInterface
MOTIF_APP_DIR=lib/MotifApp

PROGRAM= vbctool

INCLUDE = -I${GFE_DIR}/headers \
	  -I${GRAPH_INTERFACE_DIR}/headers \
	  -I${MOTIF_APP_DIR}/headers \

LIB_DIRS = -L${GFE_DIR}/lib \
	   -L${GRAPH_INTERFACE_DIR}/lib \
	   -L${MOTIF_APP_DIR}/lib -L/opt/X11/lib/

#PURIFY = purify



# Libraries
#
LIBS= ${LIB_DIRS} -lc -lm -lGraphInterface -lGFE -lMotifApp -lXm -lXt -lX11 -lXext
#SOLARISLIBS = -lsocket -lnsl -lgen

G++INCLUDE = /vol/gnu/lib/g++-include/2.8

#X11INCLUDE = /vol/X11R5/include
##GFEINCLUDE = /home/kupke/motif/gfe/include/v1.0
GFEINCLUDE = ./lib/GFE/headers/
MOTIFAPPINC = ./lib/MotifApp/headers/
GRAPHINT = ./lib/GraphInterface/headers/
#GFEHOME = /home/kupke/motif/gfe/development/quells
#X11LIB = /vol/X11R5/lib
#LOCALLIB = /usr/local/lib

#OPEN_MOTIF_DIR=

# gfe
#
INCLUDE = -I/opt/X11/include -I/usr/include/ -I/usr/local/include/ -I$(MOTIFAPPINC) -I$(GFEINCLUDE) -I$(GRAPHINT)
#-I$(X11INCLUDE) 

#STD_INCLUDE = -I$(X11LIB) -I$(LOCALLIB)

CPP_SOURCES= 	edge.cc\
		node.cc \
		basic_tree.cc\
		tree.cc \
		tree_input.cc\
		TreeInterface.cc\
		TreeDiagManager.cc\
		TreeApp.cc\
		VisualizeAlgoCmd.cc\
		TreeScalerCmd.cc\
		TreeScalerDiag.cc\
		TreeSearcherCmd.cc\
		SwitchToSearcherCmd.cc\
		TreeDiagArray.cc\
		NormalModeCmd.cc\
		EmulationCmd.cc\
		EmlStopCmd.cc\
		EmlInterruptCmd.cc\
		EmlContinueCmd.cc\
		EmlSetupCmd.cc\
		EmlSetupDiag.cc\
		StandardInputCmd.cc\
		StdinContinueCmd.cc\
		StdinInterruptCmd.cc\
		CopyrightCmd.cc\
		CopyrightDiag.cc\
		VBCCmd.cc\
		UserExample.cc

# VBCCmd.cc and UserExample.cc are not needed for Library 

CPP_OBJECTS= $(CPP_SOURCES:.cc=.o)

.PHONY: all libGFE libGraphInterface libMotifApp clean

all: libGFE libGraphInterface libMotifApp ${PROGRAM}

libGFE:
	make -C ${GFE_DIR}

libGraphInterface:
	make -C ${GRAPH_INTERFACE_DIR}

libMotifApp:
	make -C ${MOTIF_APP_DIR}

#Compile, if .o file has changed
%.o: %.cc
	$(CCC) -c -o $*.o $(CPP_FLAGS) $(INCLUDE) $<


CCFLAGS = $(CPP_FLAGS) $(INCLUDE)

TARGET_ARCH =



.KEEP_STATE:

$(PROGRAM): $(CPP_OBJECTS)
	$(PURIFY) \
	$(LINKER) -o $(PROGRAM) \
	$(CPP_OBJECTS) \
	$(LIBS) \
	$(SOLARISLIBS)

clean:
	rm -f ${CPP_OBJECTS}
	rm -f ${PROGRAM}

clean-all: clean
	make -C ${GFE_DIR} clean
	make -C ${GRAPH_INTERFACE_DIR} clean
	make -C ${MOTIF_APP_DIR} clean

dist:
	tar --exclude-vcs --transform 's,^,vbctool/,' -czf vbctool.tar.gz \
		*.cc \
		*.h \
		*.glb \
		Makefile \
		AUTHORS \
		COPYRIGHT \
		GPL \
		INSTALL \
		test.tree \
		GRAPHResource \
		lib/GFE \
		lib/GraphInterface \
		samples
#
# Dependencies:
#


depend:
	makedepend -I$(G++INCLUDE) -I$(X11INCLUDE) -I$(MOTIFAPPINC) -I$(GFEINCLUDE) $(STD_INCLUDE) -- $(GFELIB) \
			$(CPP_FLAGS) $(CPP_SOURCES)
# DO NOT DELETE THIS LINE -- make depend depends on it.


#
# Maintain the following definitions:
#
#	HDR	all header files (*.h) that you create
#	SRC	all C source files (*.cpp) that you create
#	OBJ	all object files (*.o) required to load your program
#	EXE	name of the executable
#	DOC	all the documentation files
#	CFLAGS	all the compiler options
#
# Use the following make targets:
#
#	all	(or nothing) to build your program (into EXE's value)
#	clean	to remove the executable and .o files

SRC =  cm.cpp 
OBJ =  cm.o
EXE =  cm
CFLAGS =  -Wall  


all: ${OBJ}
	g++   ${OBJ} -o ${EXE} ${CFLAGS}

cm.o:  cm.cpp tokenizer.h token.h parser.h parsenode.h symboltable.h entry.h codegenerator.h
	g++ -O2 -c -g  ${CFLAGS}$  cm.cpp

clean:
	rm -f ${OBJ} ${EXE} core



# TP6 : Fichier Makefile
#
#include ../Makefile.inc
PSE_DIR = ..
PSE_BIN = ${PSE_DIR}/bin
PSE_LIB = ${PSE_DIR}/lib
PSE_INCL_DIR = ${PSE_DIR}/include

CFLAGS = -g -I. -I${PSE_INCL_DIR} -w
		
LDLIBS = -L${PSE_LIB} -lpse -lm -pthread

CC = gcc
EXE = client tcpServer annuaire

all: ${EXE}

${EXE): ${PSE_LIB}

clean:
	rm -f *.o *~ ${EXE} journal.log



# TP6 : Fichier Makefile
#
include ../Makefile.inc

EXE = client tcpServer

all: ${EXE}

${EXE): ${PSE_LIB}

clean:
	rm -f *.o *~ ${EXE} journal.log



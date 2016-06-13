# TP6 : Fichier Makefile
#
include ../Makefile.inc

EXE = client tcpServer annuaire

all: ${EXE}
	$(shell mv client ../bin/client)
	$(shell mv tcpServer ../bin/tcpServer)
	$(shell mv annuaire ../bin/annuaire)
#	echo $(FILES)

${EXE): ${PSE_LIB}

clean:
	rm -f *.o *~ ${EXE} journal.log



#include "pse.h"

#define CMD   "[Client]"

int main(int argc, char *argv[]) {
	int sock, ret;
	struct sockaddr_in *address;
	int charEcris;
	char buff[LIGNE_MAX];
	
	if (argc != 3) {
		erreur("usage: %s machine port\n", argv[0]);
	}

	printf("%s: creating a socket\n", CMD);
	sock = socket (AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		erreur_IO("socket");
	}

	printf("%s: DNS resolving for %s, port %s\n", CMD, argv[1], argv[2]);
	address = resolv(argv[1], argv[2]);
	if (address == NULL) {
		erreur("adresse %s port %s inconnus\n", argv[1], argv[2]);
	}
	printf("%s: adr %s, port %hu\n", CMD,
	stringIP(ntohl(address->sin_addr.s_addr)),
	ntohs(address->sin_port));

	/* connexion sur site distant */
	printf("%s: connecting the socket\n", CMD);
	ret = connect(sock, (struct sockaddr *) address, sizeof(struct sockaddr_in));
	if (ret < 0) {
		erreur_IO("Connect");
	}

	freeResolv();

	while(1){
		printf("[Client] : ligne >");
		if (fgets(buff, LIGNE_MAX, stdin) == NULL){
			printf("EOF or error : arret\n");
			break;
		}
		else{
			charEcris = ecrireLigne(sock, buff);
			if (charEcris==-1)
				erreur_IO("Ecriture ligne");
			if (strcmp(buff, "exit\n") == 0){
				printf("[Client] : Deconnection demande\n");
				break;
			}
			else if( strcmp(buff,"fin\n")==0 )
			{
				printf("[Client] : Arret du serveur demande\n");
				break;
			}
			else{
				printf("[Client] : %d octets envoyes au serveur\n", charEcris);
			}
		}
	}

	exit(EXIT_SUCCESS);
}



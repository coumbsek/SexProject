#include "pse.h"
#include "InfoThread.h"
#define CMD   "[Client]"

void *connexionListener(void *tDatas);

int main(int argc, char *argv[]) {
	int sock, ret;
	struct sockaddr_in *address;
	int writeSize;
	char buff[LIGNE_MAX];
	pthread_t thread_id;

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
	
	InfoThread threadData = {0};
	threadData.sock = sock;
	threadData.thread_id = thread_id;

	if (pthread_create(&thread_id, NULL, connexionListener, (void*) &threadData) < 0){
		perror("[Client] : could not create Listener");
		return 1;
	}

	while(1){
		printf("[Client] : ligne >");
		if (fgets(buff, LIGNE_MAX, stdin) == NULL){
			printf("EOF or error : arret\n");
			break;
		}
		else{
			writeSize = ecrireLigne(sock, buff);
			if (writeSize==-1)
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
				printf("[Client] : %d octets envoyes au serveur\n", writeSize);
			}
		}
	}

	exit(EXIT_SUCCESS);
}

void *connexionListener(void *tDatas){
	InfoThread threadData = *(InfoThread *) tDatas;
	int log = threadData.logFile;
	int sock = threadData.sock;
	
	int readSize;
	char *message , buff[LIGNE_MAX];
	
	char *s = malloc(5*sizeof(char));

	while(1){
		readSize = lireLigne(sock, buff);
		if (readSize==-1)
			erreur_IO("Lecture ligne");
		else if( strcmp("[Serveur] : Extinction du serveur en cours\n",buff)==0 )
		{
			printf("[Client] : Deconnexion du client\n");
			pthread_exit(s);
		}
		else
			printf("%s\n", buff);
	}
}

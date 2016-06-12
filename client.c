#include "pse.h"
#include "InfoThread.h"
#define CMD   "[Client]"
#include "constantes.h"

void *connexionListener(void *tDatas);
void *connexionHandlerAnnuaire(void *p);

int main(int argc, char *argv[]) {
	int	sock,ret,i;
	struct	sockaddr_in *address;
	int 	writeSize;
	char 	*buff,*teste;
	char 	addressServer[17],portServer[5];

	pthread_t thread_id;

	if (argc == 1){	
		// on lance la connexion annuaire-client
		pthread_t threadAnnuaire;
		if (pthread_create(&threadAnnuaire, NULL, connexionHandlerAnnuaire,NULL)!=0){
			perror("could not create thread");
			return 1;
		}

		// on récupère l'adresse choisi par le client  
		if(pthread_join(threadAnnuaire,(void**)&buff)!=0)
		{
			perror("join"); 
			exit(1);
		}

		printf("Connexion à : %s \n", (char *) buff);

		teste=(char *)buff; 	
		

		i=0;
		while(teste[i]!=' ')
		{
			i=i+1;
		}
		printf("1\n");
		//on récupère l'adresse IP
		strncpy(addressServer,teste,i);
		addressServer[i]='\0';

		printf("%s",addressServer);
		//on récupère le port
		strncpy(portServer,teste+(i+1),4);
		portServer[4]='\0';
		printf(" %s\n",portServer);
	}
	else if(argc == 3){
		printf("T\n");
		strncpy(addressServer, argv[1], 17);
		printf("U\n");
		strncpy(portServer, argv[2], 5);
		printf("V\n");
	}
	else{
		erreur("usage: %s\nusage: %s ipV4 port\n",argv[0],argv[0]);
	}
		
//*	
	printf("%s: creating a socket\n", CMD);
	sock = socket (AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		erreur_IO("socket");
	}

	printf("%s: DNS resolving for %s, port %s\n", CMD, addressServer, portServer);
	address = resolv(addressServer, portServer);
	if (address == NULL) {
		erreur("adresse %s port %s inconnus\n", addressServer, portServer);
	}
	printf("%s: adr %s, port %hu\n", CMD,stringIP(ntohl(address->sin_addr.s_addr)),ntohs(address->sin_port));

	// connexion sur site distant 
	printf("%s: connecting the socket\n", CMD);
	ret = connect(sock, (struct sockaddr *) address, sizeof(struct sockaddr_in));
	if (ret < 0) {
		erreur_IO("Connect");
	}

	freeResolv();
	
	InfoThread threadData = {0};
	threadData.sock = sock;
	threadData.thread_id = thread_id;

	if (pthread_create(&threadData.thread_id, NULL, connexionListener, (void*) &threadData) < 0){
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
	//*/
	while(1){

	}
	exit(EXIT_SUCCESS);
}

void *connexionHandlerAnnuaire(void *p){
	int		sock,
			ret;
	struct 		sockaddr_in *address;
	static char	buff[LIGNE_MAX];
	pthread_t	thread_annuaire;
	int		readSize,
			writeSize;
	int		identifier;
	// variable ajoutée 
	 int i,ligne;
	 i=1;
	 ligne=0;
	
	sock = socket(AF_INET, SOCK_STREAM, 0);

	printf("%s Connection to annuaire : %s, port %s\n", CLIENT_ANNUAIRE, ADDR_ANNUAIRE,PORT_ANNUAIRE);
	address = resolv(ADDR_ANNUAIRE,PORT_ANNUAIRE);
	if (address == NULL)
		erreur("address %s port %s unknow\n",ADDR_ANNUAIRE,PORT_ANNUAIRE);
	printf("%s Successfully resolved  %s, port %hu\n", CLIENT_ANNUAIRE,
			stringIP(ntohl(address->sin_addr.s_addr)),
			ntohs(address->sin_port));
	/*Connexion a l'annuaire*/
	printf("%s Connecting the socket\n", CLIENT_ANNUAIRE);
	ret = connect(sock, (struct sockaddr *) address, sizeof(struct sockaddr_in));
	if (ret < 0)
		erreur_IO("Socket connection\n");
	printf("%s resolv success\n", CLIENT_ANNUAIRE);
	freeResolv();

	//Envoie de l'identifiant à l'annuaire
	identifier = ID_CLIENT;
	writeSize = send(sock, (const void *) &identifier, sizeof(identifier), 0);
	if (writeSize == -1)
		erreur_IO("Writing address line");//*/

//modification de Nabil

	// on recoit le texte envoyé par l'annuaire
	ret = lireLigne(sock, buff);
	if (ret <=0 || ret == LIGNE_MAX) {
		erreur_IO("lireLigne");
	}	  
	printf("%s \n", buff);
	
	ret = lireLigne(sock, buff);
	if (ret <=0 || ret == LIGNE_MAX) {
		erreur_IO("lireLigne");
	}	  
	printf("%s \n", buff);

	//on recoit le nombre de ligne
	ret = lireLigne(sock,buff);
	if (ret <=0 || ret == LIGNE_MAX) {
		erreur_IO("lireLigne");
	}

	//on converti le char* en int
	ligne = atoi(buff);

	// on recoit la liste des serveurs connectés
	for(i=0;i<ligne;i++)
	{
		printf("%d - ",i+1);

		//on envoie la ligne coresspondante
		ret = lireLigne(sock, buff); 
		if (ret <=0 || ret == LIGNE_MAX) {
			erreur_IO("lireLigne");
		}		  
		printf("%s \n", buff);

	}

	// on envoie à l'annuaire le choix du serveur
	printf("Votre choix : ");
	if (fgets(buff, LIGNE_MAX, stdin) == NULL) {
		erreur("fgets");
	}
	  
	ret = ecrireLigne(sock, buff);
	if (ret == -1) {
		erreur_IO("ecrireLigne");
	}


	// on récupère le serveur choisi
	ret = lireLigne(sock, buff);
	if (ret == -1) {
		erreur_IO("ecrireLigne");
	}

	printf("%s \n", buff);
	shutdown(sock,2);
	if (close(sock) == -1) {
		erreur_IO("close socket");
	}	
	
	pthread_exit(buff);
	//fin de modification
}
//*
void *connexionListener(void *tDatas){
	InfoThread threadData = *(InfoThread *) tDatas;
	int log = threadData.logFile.fd;
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
//*/


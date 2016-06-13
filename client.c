#include "pse.h"
#include "InfoThread.h"
#define CMD   "[Client]"
#include "constantes.h"

void *connexionListener(void *tDatas);
void *connexionHandlerAnnuaire(void *p);
void *downloadFile(void *tDatas);
void *commandHandler(void *p);

int main(int argc, char *argv[]) {
	int	sock,ret,i;
	struct	sockaddr_in *address;
	int 	writeSize;
	char 	*buff,*teste;
	char	commandBuff[11];
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
		strncpy(addressServer, argv[1], 17);
		strncpy(portServer, argv[2], 5);
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

	if (pthread_create(&threadData.thread_id, NULL, downloadFile, (void*) &threadData) < 0){
		perror("[Client] : could not create download the file");
		return 1;
	}

	while(1){
		fgets(commandBuff, 10, stdin);
		if (strcmp("stop\n",commandBuff) == 0){
			write(ret, buff, strlen(buff));
			shutdown(ret,2);
			close(ret);
			exit(EXIT_SUCCESS);
		}	
	}
//*/
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
void *commandHandler(void *p){
	int retval,
	    j;
	char buff[11];

	while(1){
		fgets(buff, 10, stdin);
		if (strcmp("stop\n",buff) == 0){
			
			printf("je veux m'arreter\n");
			
		}	
	}
}

void *connexionHandlerServer(void *datas){
	int readSize = 0;
	int buff;

	InfoThread *thread = (InfoThread *) datas;
	fd_set 	rfds;

	FD_ZERO(&rfds);
	FD_SET(thread->sock, &rfds);

	readSize = select(thread->sock+1, &rfds, NULL, NULL, 0);
	if (readSize == -1)
	       perror("select()");
	else if (readSize){
		while(1){
			readSize = recv(thread->thread_id, &buff, sizeof(int), 0);
			if (readSize==-1)
				erreur_IO("Reception Error");
			else if( STOP == buff )
			{
				printf("[Client] : Deconnexion du serveur\n");
				shutdown(thread->sock,2);
				close(thread->sock);
				pthread_exit(NULL);
			}
			else
				printf("Code intruction %d\n", buff);
		}
	}
//*/
}

void *downloadFile(void *tDatas){
	InfoThread threadData = *(InfoThread *) tDatas;
	int log = threadData.logFile.fd;
	int sock = threadData.sock;
	
	int readSize;
	char *message;
	char buff[LIGNE_MAX];
	
	char *s = malloc(5*sizeof(char));

	// variable ajoutée 
	int i,ligne,ret;
	i=0;
	ligne=0;
	FILE *fi;
	
	fi=fopen("tmp.txt","w+"); //fichié ouvert en ecriture+lecture et création si existe pas

	// on recoit le message de début d'envoie
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

	// on voir le contenu du fichier
	for(i=0;i<ligne;i++)
	{

		ret = lireLigne(sock, buff); 
		if (ret <=0 || ret == LIGNE_MAX) {
			erreur_IO("lireLigne");
		}		  
		fprintf(fi, buff);
		fprintf(fi, "\n");
	}

	fclose(fi);

	// on recoit le message de fin d'envoie
	ret = lireLigne(sock, buff);
	if (ret <=0 || ret == LIGNE_MAX) {
		erreur_IO("lireLigne");
	}	  
	printf("%s \n", buff);

	pthread_exit(buff);
//fin de modification

}


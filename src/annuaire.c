#include "pse.h"
#include <pthread.h> 
#include "InfoThread.h"
#include "LockableFile.h"
#include <sys/time.h>
#include "constantes.h"

//the thread function
void	*connexionHandler(void *);
void	connexionHandlerClient(void *);
void	connexionHandlerServer(void *);
int	rAzLog(int fd); 
void	iniCohorte(InfoThread *cohorte, int size, int type);

InfoThread cohorteServer[NBSERVERS];
InfoThread cohorteClient[NBCLIENTS];

int	main(int argc , char *argv[])
{
	int	isServer = -1,
		readSize,
		identifier,
		j, k;
	int 	ecoute, 
		client_sock, 
		c;
	struct	sockaddr_in annuaire, 
			   client;
	FileL	log, 
		datasServers;
	//Create socket
	ecoute = socket(AF_INET , SOCK_STREAM , 0);
	if (ecoute == -1)
	{
		erreur_IO("Could not create socket");
	}
	//puts("Socket created");
	
	
	//Prepare the sockaddr_in structure
	annuaire.sin_family = AF_INET;
	annuaire.sin_addr.s_addr = INADDR_ANY;
	annuaire.sin_port = htons( atoi(PORT_ANNUAIRE) );
	
	//Bind
	if( bind(ecoute,(struct sockaddr *)&annuaire , sizeof(annuaire)) < 0){
		perror("bind failed. Error");
		return 1;
	}
	//puts("bind done");
	
	log.fd = open("journal.log", O_WRONLY|O_APPEND|O_CREAT|O_TRUNC, 0660);
	if (log.fd == -1) {
		erreur_IO("open log");
	}
	log.mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

	datasServers.fd = open("servers.datas", O_WRONLY|O_APPEND|O_CREAT|O_TRUNC, 0660);
	if (datasServers.fd == -1) {
		erreur_IO("open datas Servers");
	}
	datasServers.mutex =(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	
	//Listen
	listen(ecoute , NBCLIENTS + NBSERVERS);

	iniCohorte(cohorteServer, NBSERVERS, 1);
	iniCohorte(cohorteClient, NBCLIENTS, 0);

	//Accept an incoming connection
	//puts("Waiting for incoming connections...");
	InfoThread *th;
	c = sizeof(struct sockaddr_in);
	while( (client_sock = accept(ecoute, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		
		fd_set 	rfds;
		struct 	timeval tv = {5,0};

		FD_ZERO(&rfds);
		FD_SET(client_sock, &rfds);

		readSize = select(client_sock+1, &rfds, NULL, NULL, &tv);
		if (readSize == -1)
		       perror("select()");
		else if (readSize){
			readSize = recv(client_sock, &identifier, sizeof(int),0);
			printf("Identifier received : %d\n", identifier);
			if (identifier == ID_SERVER){//0X00F0
				isServer = 1;

				for(j=0;j<NBSERVERS;j++){
					if(cohorteServer[j].isFree==1)
						break;
				}
				
				if (j == NBSERVERS){
					shutdown(ecoute, 2);
					close(ecoute);
					continue;
				}

				pthread_mutex_lock(&(cohorteServer[j].mutex));
					printf("Affectation Server Thread %p = %d\n",&(cohorteServer[j]),cohorteServer[j].isFree);
					cohorteServer[j].logFile.fd = log.fd;
					cohorteServer[j].datasFile.fd = datasServers.fd;
					cohorteServer[j].sock = client_sock;
					printf("Adresse du serveur %s\n",stringIP(ntohl(client.sin_addr.s_addr)));
					strcpy(cohorteServer[j].ip,stringIP(ntohl(client.sin_addr.s_addr)));
				pthread_mutex_unlock(&(cohorteServer[j].mutex));
				
				if (pthread_create(&(cohorteServer[j].thread_id), NULL, connexionHandlerServer, &j) < 0){
					perror("could not create thread server");
				}
				printf("Server joined\n");
			}
			else if (identifier == ID_CLIENT){//0X0F00
				isServer = 0;
				for(j = 0; j < NBCLIENTS; j++){
					if (cohorteClient[j].isFree==1)
						break;
				}
				if (j == NBCLIENTS){
					shutdown(ecoute, 2);
					close(ecoute);
					continue;
				}
				pthread_mutex_lock(&(cohorteClient[j].mutex));
					printf("Affectation Client Thread %p = %d\n",&(cohorteClient[j]),cohorteClient[j].isFree);
					cohorteClient[j].logFile.fd = log.fd;
					printf("Addresse du client %s\n",stringIP(ntohl(client.sin_addr.s_addr)));
					strcpy(cohorteClient[j].ip,stringIP(ntohl(client.sin_addr.s_addr)));
					cohorteClient[j].sock = client_sock;
				pthread_mutex_unlock(&(cohorteClient[j].mutex));
				
				if (pthread_create(&(cohorteClient[j].thread_id), NULL, connexionHandlerClient, &j) < 0){
					perror("could not create thread server");
				}

				printf("Client joined\n");
			}
		}
	}
	
	if (client_sock < 0)
	{
		perror("accept failed");
		return 1;
	}
	
	return 0;
}

void	iniCohorte(InfoThread *cohorte, int size, int type){
	int j;
	for (j = 0; j<size;j++){
		cohorte[j].mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
		cohorte[j].isServer = type;
		cohorte[j].isFree = 1;
		cohorte[j].ip = malloc(sizeof(char)*17);
	}
}

void	connexionHandlerServer(void *tDatas){
	printf("Je suis un server!!!\n");
	fd_set 	rfds;
	struct	timeval tv = {5,0};
	int	retval;

	int j = *(int *) tDatas;
	InfoThread threadData = cohorteServer[j];
	int	log = threadData.logFile.fd;
	int	datasFile = threadData.datasFile.fd;
	int	sock = threadData.sock;

	int	readSize;
	short	*port = malloc(sizeof(short));
	char	pingValue = 1;

	FD_ZERO(&rfds);
	FD_SET(sock, &rfds);

	retval = select(sock+1, &rfds, NULL, NULL, &tv);
	if (retval == -1)
               perror("select()");
	else if (retval){
		readSize = recv(sock, port, sizeof(short),0);
	}
	else{
		printf("No data within five seconds.\n");
		cohorteServer[j].isFree = 1;
		shutdown(cohorteServer[j].sock,2);
		close(cohorteServer[j].sock);
		pthread_exit(NULL);
	}

	pthread_mutex_lock(&(threadData.datasFile.mutex));
	//write adresse et port to datasFile
		char str[8];
		sprintf(str, "%d", *port);
		for (j = 0; j < 17; j++){
			if (!((threadData.ip[j] >= '0' && threadData.ip[j] <='9') || threadData.ip[j] == '.')){
				break;
			}
		}
		threadData.ip[j] = '\0';
		write(threadData.datasFile.fd, threadData.ip, sizeof(char)*j);
		write(threadData.datasFile.fd, " ", sizeof(char));
		write(threadData.datasFile.fd, str, 4*sizeof(char));
		write(threadData.datasFile.fd, "\n", sizeof(char));
	pthread_mutex_unlock(&(threadData.datasFile.mutex));

	while(1){
		retval = select(sock+1, &rfds, NULL, NULL, &tv);
		if (retval == -1)
		       perror("select()");
		else if (retval){
			tv.tv_usec = 0;
			tv.tv_sec = 5;
			readSize = recv(sock, &pingValue, sizeof(char),0);
			if (readSize <=0 || readSize == LIGNE_MAX)
				erreur_IO("lireLigne");
			else if (readSize==0)
				continue;
			else{
				//Traitement des données recues si necessaire mais ici on ne recoit que des pings normalement
			}
		}
		else{
			printf("No data within five seconds : Tiemout.\n");
			cohorteServer[j].isFree=1;
			shutdown(cohorteServer[j].sock,2);
			close(cohorteServer);
			pthread_exit(NULL);
		}
	}
}

void	connexionHandlerClient(void *tDatas)
{
	printf("Je suis un client!\n");
	int j = *(int *) tDatas;
	InfoThread threadData = cohorteClient[j];
	int	log = threadData.logFile.fd;
	int	sock = threadData.sock;
	
	int	readSize, writeSize;
	char	*message, buff[LIGNE_MAX];
	char	*flagStop = malloc(sizeof(char));

	// variable ajoutée 
	int i,ligne,ret;
	i=1;
	ligne=0;
	FILE *fi;
	fi=fopen("servers.datas","r"); //fichié ou sont logé les connexions serveur

	//Demmande au client de se connecter

	message = "[annuaire] : Hello! I'm your connection handler\n";
	write(sock , message , strlen(message));
	//printf("%s",message);
  	
	message="[annuaire] : Vous voulez vous connecter à quel serveur ?\n";
	write(sock , message , strlen(message));
	//printf("%s",message);

	// on compte le nombre de ligne dans le fichier
	while(fgets(buff,LIGNE_MAX,fi)!=NULL)
	{
		ligne++;
	}

	//convertit ligne en char* et l'envoie
	sprintf(buff, "%d", ligne);

	ret = ecrireLigne(sock, buff);
	if (ret == -1) {
		erreur_IO("ecrireLigne");
	}

	//on envoie la listes des serveurs connecté
	fseek(fi,0,SEEK_SET); //on se remet au début du fichier
	for(i=0;i<ligne;i++)
	{
	
//		sprintf(buff,"%d -",i); //envoie le numéro de ligne
//		ret = ecrireLigne(sock, buff);
		
		printf("%d - ",i+1);

		// envoie la ligne corespondante
		if (fgets(buff, LIGNE_MAX,fi) == NULL) {
			erreur("fgets");
		}
		ret = ecrireLigne(sock, buff);
		if (ret == -1) {
			erreur_IO("ecrireLigne");
		}
		printf("%s",buff);
	}

	//on recoit le numéro de serveur choisi
	ret = lireLigne(sock,buff);
	if (ret <=0 || ret == LIGNE_MAX) {
		erreur_IO("lireLigne");
	}

	//on converti le char* en int
	ligne = atoi(buff);

	fseek(fi,0,SEEK_SET); //on se remet au début du fichier

	//on envoie l'adresse choisie
	for(i=0;i<ligne;i++)
	{
	
		// envoie la ligne corespondante
		if (fgets(buff, LIGNE_MAX,fi) == NULL) {
			erreur("fgets");
		}
	}

	ret = ecrireLigne(sock, buff);
	if (ret == -1) {
		erreur_IO("ecrireLigne");
	}

	printf("Le client a choisi le serveur : %s \n",buff);
	
	fclose(fi);
	shutdown(sock,2);
	close(sock);
	cohorteClient[j].isFree=1;

// fin des modifications
	
	pthread_exit(flagStop);
}

int	rAzLog(int fd) {
	int newFd;
	if (close(fd) == -1)
		erreur_IO("close log");
	newFd = open("journal.log", O_WRONLY|O_APPEND|O_CREAT|O_TRUNC, 0660);
	if (newFd == -1)
		erreur_IO("open trunc log");
	return newFd;
}

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
void	iniCohorte(InfoThread *cohorte, int size);

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
	
	if (argc < 2)
		erreur("usage: %s port\n", argv[0]);
	
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
	annuaire.sin_port = htons( atoi(argv[1]) );
	
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
	
	for (j = 0; j<NBSERVERS;j++){
		cohorteServer[j].mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
		cohorteServer[j].isServer = 1;
		cohorteServer[j].isFree = 1;
		cohorteServer[j].ip = malloc(sizeof(char)*17);
	}

	//Accept an incoming connection
	//puts("Waiting for incoming connections...");
	InfoThread *th;
	c = sizeof(struct sockaddr_in);
	while( (client_sock = accept(ecoute, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		
		puts("Connection accepted");
	 	
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
				cohorteClient[j].isFree = 0;
				printf("Client joined\n");
			}
		}
		//Now join the thread , so that we dont terminate before the thread
		//pthread_join( thread_id , NULL);
		puts("Handler assigned");
	}
	
	if (client_sock < 0)
	{
		perror("accept failed");
		return 1;
	}
	
	return 0;
}
/*
void	*connexionHandler(void *tDatas){
	
	int 	retval;
	int 	readSize;

	//int 	identifier = *(int *)tDatas;
	int j = 
	InfoThread *threadData = (InfoThread *) tDatas;//&(cohorteServer[identifier]);
	int sock = threadData->sock;
	//printf("Thread Identifier <%d>\n", identifier);
	//printf("ConnexionHandler <%p>\n", &(threadData));

	pthread_mutex_lock(&(threadData->mutex));
	if (threadData->isServer == 0)
		connexionHandlerClient(tDatas);
	else if (threadData->isServer == 1)
		connexionHandlerServer(tDatas);
	else{
		erreur("Type de thread non reconnu\n");
	}
	pthread_mutex_unlock(&(threadData->mutex));
}*/

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
	else
		printf("No data within five seconds.\n");

	pthread_mutex_lock(&(threadData.datasFile.mutex));
	//write adresse et port to datasFile
		char str[8];
		sprintf(str, "%d", *port);
		write(threadData.datasFile.fd, threadData.ip, sizeof(char)*16);
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
			pthread_exit(NULL);
		}
	}
}

void	connexionHandlerClient(void *tDatas)
{
	InfoThread threadData = *(InfoThread *) tDatas;
	int	log = threadData.logFile.fd;
	int	sock = threadData.sock;
	
	int	readSize, writeSize;
	char	*message , buff[LIGNE_MAX];
	char	*flagStop = malloc(sizeof(char));
	//sending message to client
	message = "[annuaire] : Hello! I'm your connection handler\n";
	write(sock , message , strlen(message));
	//Receive a message from client
	while(1)
	{//*
		readSize = lireLigne(sock , buff);//rcv
		if (readSize <=0 || readSize == LIGNE_MAX) {
			erreur_IO("lireLigne");
		}
		else if (readSize==0)
			continue;
		else{
			printf("[Serveur] : reception %d octets : \"%s\"\n", readSize, buff);
			writeSize = ecrireLigne(log,buff);
			if(writeSize !=-1)
				printf("[Serveur] : ecriture de %d octets\n", writeSize);
		}
		if (strcmp(buff,"fin\n")==0){
			printf("[Serveur] : Arret en cours\n");
			*flagStop = 1;
			break;
		}
		else if(strcmp(buff, "exit\n")==0){
			printf("[Serveur] : Client disconnection\n");
			break;
		}
		else if (strcmp(buff, "init\n")==0){
			printf("[Serveur] : Remise a zero journal\n");
			rAzLog(log);
		}
		//write(sock , buff , strlen(buff));
		//clear the message buffer
		memset(buff, 0, LIGNE_MAX);//*/
	}
	
	if(readSize == 0)
	{
		erreur_IO("Client disconnected");
		fflush(stdout);
	}
	else if(readSize == -1)
	{
		perror("lireLigne failed");
	}
	
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

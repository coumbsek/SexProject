#include "pse.h"
#include <pthread.h> 
#include "InfoThread.h"
#include "LockableFile.h"
#include <sys/time.h>
#define NBCLIENTS 3
#define NBSERVERS 2

//the thread function
void *connexionHandler(void *);
void connexionHandlerClient(void *);
void connexionHandlerServer(void *);
int rAzLog(int fd); 

InfoThread cohorteClient[NBCLIENTS];
InfoThread cohorteServer[NBSERVERS];

int main(int argc , char *argv[])
{
	int ecoute , client_sock , c;
	struct sockaddr_in annuaire, client;
	FileL log, datasServers;
	
	if (argc < 2)
		erreur("usage: %s port\n", argv[0]);
	
	//Create socket
	ecoute = socket(AF_INET , SOCK_STREAM , 0);
	if (ecoute == -1)
	{
		erreur_IO("Could not create socket");
	}
	puts("Socket created");
	
	//Prepare the sockaddr_in structure
	annuaire.sin_family = AF_INET;
	annuaire.sin_addr.s_addr = INADDR_ANY;
	annuaire.sin_port = htons( atoi(argv[1]) );
	
	//Bind
	if( bind(ecoute,(struct sockaddr *)&annuaire , sizeof(annuaire)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");
	
	log.fd = open("journal.log", O_WRONLY|O_APPEND|O_CREAT|O_TRUNC, 0660);
	if (log.fd == -1) {
		erreur_IO("open log");
	}

	datasServers.fd = open("servers.datas", O_WRONLY|O_APPEND|O_CREAT|O_TRUNC, 0660);
	if (datasServers.fd == -1) {
		erreur_IO("open datas Servers");
	}
	
	//Listen
	listen(ecoute , 3);
	
	//Accept an incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	pthread_t thread_id;
	while( (client_sock = accept(ecoute, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		puts("Connection accepted");
	 	
		InfoThread threadData = {0};
		threadData.logFile.fd = log.fd;
		threadData.datasFile.fd = datasServers.fd;
		threadData.sock = client_sock;
		threadData.thread_id = thread_id;
		threadData.isServer = 1;

		if( pthread_create( &thread_id , NULL ,  connexionHandler , (void*) &threadData) < 0)//client_sock
		{
			perror("could not create thread");
			return 1;
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

void *connexionHandler(void *tDatas){
	fd_set rfds;
	struct timeval tv = {5,0};
	int retval;
	int readSize;

	int identifier;
	
	InfoThread threadData = *(InfoThread *) tDatas;
	int sock = threadData.sock;
	
	FD_ZERO(&rfds);
	FD_SET(sock, &rfds);

	retval = select(sock+1, &rfds, NULL, NULL, &tv);
	if (retval == -1)
               perror("select()");
	else if (retval){
		readSize = recv(sock, &identifier, sizeof(int),0);
		if (identifier == 254)//0X00FE
			threadData.isServer = 1;
		else if (identifier == 65280)//0XFF00
			threadData.isServer = 0;
	}
	else
		printf("No data within five seconds.\n");

	if (threadData.isServer == 0)
		connexionHandlerClient(tDatas);
	else if (threadData.isServer == 1)
		connexionHandlerServer(tDatas);
	else{
		erreur("Type de thread non reconnu\n");
	}
}

void connexionHandlerServer(void *tDatas){
	fd_set rfds;
	struct timeval tv = {5,0};
	int retval;

	InfoThread threadData = *(InfoThread *) tDatas;
	int log = threadData.logFile.fd;
	int datasFile = threadData.datasFile.fd;
	int sock = threadData.sock;

	int readSize;
	short *port = malloc(sizeof(short));
	char pingValue = 1;

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
			pthread_exit(NULL);
		}
	}
}

void connexionHandlerClient(void *tDatas)
{
	InfoThread threadData = *(InfoThread *) tDatas;
	int log = threadData.logFile.fd;
	int sock = threadData.sock;
	
	int readSize, writeSize;
	char *message , buff[LIGNE_MAX];
	char *flagStop = malloc(sizeof(char));
	//sending message to client
	message = "[annuaire] : Hello! I'm your connection handler\n";
	write(sock , message , strlen(message));
	//Receive a message from client
	while(1)
	{
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
		memset(buff, 0, LIGNE_MAX);
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

int rAzLog(int fd) {
	int newFd;
	if (close(fd) == -1)
		erreur_IO("close log");
	newFd = open("journal.log", O_WRONLY|O_APPEND|O_CREAT|O_TRUNC, 0660);
	if (newFd == -1)
		erreur_IO("open trunc log");
	return newFd;
}

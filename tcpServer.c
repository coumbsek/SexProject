#include "pse.h"
#include<pthread.h> 

typedef struct InfoThread{
	int logFile;
	int sock;
	int thread_id;
}InfoThread;

//the thread function
void *connexionHandler(void *);
int rAzLog(int fd); 

int main(int argc , char *argv[])
{
	int ecoute , client_sock , c;
	struct sockaddr_in server , client;
	int log;

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
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( atoi(argv[1]) );

	//Bind
	if( bind(ecoute,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");

	log = open("journal.log", O_WRONLY|O_APPEND|O_CREAT|O_TRUNC, 0660);
	if (log == -1) {
		erreur_IO("open log");
	}

	//Listen
	listen(ecoute , 3);

	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);


	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	pthread_t thread_id;
	while( (client_sock = accept(ecoute, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		puts("Connection accepted");
	 	
		InfoThread threadData = {0};
		threadData.logFile = log;
		threadData.sock = client_sock;
		threadData.thread_id = thread_id;

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
 
void *connexionHandler(void *tDatas)
{
	//Get the socket descriptor
	//int sock = *(int*)ecoute;
	
	InfoThread threadData = *(InfoThread *) tDatas;
	int log = threadData.logFile;
	int sock = threadData.sock;

	int readSize, writeSize;
	char *message , buff[LIGNE_MAX];
	char *flagStop = malloc(sizeof(char));
	//Send some messages to the client
	message = "Greetings! I am your connection handler\n";
	write(sock , message , strlen(message));

	message = "Now type something and i shall repeat what you type \n";
	write(sock , message , strlen(message));

	//Receive a message from client
	while( 1)
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

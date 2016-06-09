#include "pse.h"
#include <pthread.h> 
#include "InfoThread.h"
#include "constantes.h"

void 	*connexionHandler(void *);
int 	rAzLog(int fd); 
void 	*connexionHandlerAnnuaire(void *port);

int	main(int argc , char *argv[])
{
	int	ecoute, 
		client_sock, 
		c;
	struct	sockaddr_in	server, 
			  	client;
	FileL 	log;
	
	if (argc < 2)
		erreur("usage: %s port\n", argv[0]);
	
	pthread_t threadAnnuaire;
	c = atoi(argv[1]);
	if( pthread_create( &threadAnnuaire , NULL ,  connexionHandlerAnnuaire , (void*) &c) < 0)//client_sock
	{
		perror("could not create thread");
		return 1;
	}
 	pthread_join( threadAnnuaire , NULL);

//*
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
	
	log.fd = open("journal.log", O_WRONLY|O_APPEND|O_CREAT|O_TRUNC, 0660);
	if (log.fd == -1) {
		erreur_IO("open log");
	}
	
	//Listen
	listen(ecoute , 3);
	
	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	pthread_t thread_id;
	while( (client_sock = accept(ecoute, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		puts("Connection accepted");
	 	
		InfoThread threadData = {0};
		threadData.logFile.fd = log.fd;
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
	//*/
	return 0;
}
//*
void	*connexionHandler(void *tDatas)
{
	//Get the socket descriptor
	//int sock = *(int*)ecoute;
	
	InfoThread 	threadData = *(InfoThread *) tDatas;
	int 	log = threadData.logFile.fd;
	int 	sock = threadData.sock;
	
	int 	readSize, writeSize;
	char 	*message , buff[LIGNE_MAX];
	char 	*flagStop = malloc(sizeof(char));
	//Send some messages to the client
	message = "[Server] : Hello! I'm your connection handler\n";
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
//*/
int	rAzLog(int fd) {
	int newFd;
	if (close(fd) == -1)
		erreur_IO("close log");
	newFd = open("journal.log", O_WRONLY|O_APPEND|O_CREAT|O_TRUNC, 0660);
	if (newFd == -1)
		erreur_IO("open trunc log");
	return newFd;
}

void	*connexionHandlerAnnuaire(void *port){
	char		isRunning = 1;
	int 		sock, 
			ret;
	char 		pingValue = 1;
	struct 		sockaddr_in *address;
	char 		buff[LIGNE_MAX];
	pthread_t 	thread_annuaire;
	int 		writeSize;
	int 		identifier;

	sock = socket(AF_INET, SOCK_STREAM, 0);

	printf("Connection to annuaire : %s, port %s\n", ADDR_ANNUAIRE,PORT_ANNUAIRE);
	address = resolv(ADDR_ANNUAIRE,PORT_ANNUAIRE);
	if (address == NULL)
		erreur("address %s port %s unknow\n",ADDR_ANNUAIRE,PORT_ANNUAIRE);
	printf("Successfully resolved  %s, port %hu\n",
			stringIP(ntohl(address->sin_addr.s_addr)),
			ntohs(address->sin_port));
	/*Connexion a l'annuaire*/
	printf("Connecting the socket\n");
	ret = connect(sock, (struct sockaddr *) address, sizeof(struct sockaddr_in));
	if (ret < 0)
		erreur_IO("Socket connection\n");
	printf("resolv success\n");
	freeResolv();
//*
	//Envoie de l'identifiant à l'annuaire
	identifier = ID_SERVER;
	writeSize = send(sock, (const void *) &identifier, sizeof(identifier), 0);
	if (writeSize == -1)
		erreur_IO("Writing address line");//*/
	//Envoie du port de connexion client à l'annuaire
	writeSize = send(sock, (const void *) port, sizeof(short),0);
	if (writeSize == -1)
		erreur_IO("Writing address line");//*/
	printf("sending address line\nwriting %d bits\n",writeSize);
	//Pining each second to allow annnuaire be sure server is still up
	while(isRunning == 1){
		printf("ping value : %d", pingValue);
		send(sock, (const void *) &pingValue, sizeof(pingValue),0);
		sleep(1);
	}
	return;
}

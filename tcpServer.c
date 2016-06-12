#include "pse.h"
#include <pthread.h> 
#include "InfoThread.h"
#include "constantes.h"

void 	*connexionHandler(void *);
int 	rAzLog(int fd); 
void 	*connexionHandlerAnnuaire(void *port);
void	*connexionHandlerClient(void *tDatas);
void	*commandHandler(void *datas);
char	*nom_du_fichier;

InfoThread cohorteClient[NBCLIENTS_SERVER];

int	main(int argc , char *argv[])
{
	int	ecoute, 
		client_sock, 
		port,
		c,
		j;
	struct	sockaddr_in	server, 
			  	client;
	FileL 	log;
	
	if (argc < 3)
		erreur("usage: %s port file\n", argv[0]);
		
	pthread_t threadAnnuaire;
	port = atoi(argv[1]);
	nom_du_fichier = argv[2];
	if( pthread_create( &threadAnnuaire , NULL ,  connexionHandlerAnnuaire , (void*) &port) < 0)
	{
		perror("could not create thread");
		return 1;
	}

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
	
	iniCohorte(cohorteClient, NBCLIENTS_SERVER);
	
	pthread_t thread_id;

	if( pthread_create( &thread_id , NULL ,  commandHandler , &thread_id) < 0)//client_sock
	{
		perror("could not create thread");
		return 1;
	}


	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	while( (client_sock = accept(ecoute, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		puts("Connection accepted");
	 
		for(j=0;j<NBCLIENTS_SERVER;j++){
			if(cohorteClient[j].isFree==1)
				break;
		}

		if (j==NBCLIENTS_SERVER){
			write(client_sock, "stop", 5*sizeof(char));
			shutdown(client_sock, 2);
			close(client_sock);
			continue;
		}

		cohorteClient[j].isFree = 0;
		cohorteClient[j].sock = client_sock;

		if( pthread_create( &thread_id , NULL ,  connexionHandlerClient , (void*) &j) < 0)//client_sock
		{
			perror("could not create thread");
			return 1;
		}
	 	
		//Now join the thread , so that we dont terminate before the thread
		pthread_join( thread_id , NULL);
		puts("Handler assigned");
	}
	
	if (client_sock < 0)
	{
		perror("accept failed");
		return 1;
	}
	//*/
	
 	pthread_join( threadAnnuaire , NULL);
	return 0;
}
//*

void	iniCohorte(InfoThread *cohorte, int size){
	int j;
	for (j = 0; j<size;j++){
		cohorte[j].mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
		cohorte[j].isServer = 0;
		cohorte[j].isFree = 1;
		cohorte[j].ip = malloc(sizeof(char)*17);
	}
}

void	*commandHandler(void *datas){
	int retval,
	    j;
	char buff[11];

	//infothread *thread = (infothread *) datas;

	while(1){
		fgets(buff, 10, stdin);
		if (strcmp("stop\n",buff) == 0){
			for (j = 0; j < NBCLIENTS_SERVER; j++){
				write(cohorteClient[j].sock, buff, strlen(buff));
				shutdown(cohorteClient[j].sock,2);
				close(cohorteClient[j].sock);
			}
		}	
	}
}

void	*connexionHandlerClient(void *tDatas)
{
	//Get the socket descriptor
	//int sock = *(int*)ecoute;
	int j = *(int *) tDatas;
	InfoThread 	threadData = cohorteClient[j];
	int 	log = threadData.logFile.fd;
	int 	sock = threadData.sock;
	
	int 	readSize, writeSize;
	char 	*message;
	char 	buff[LIGNE_MAX];
	char 	*flagStop = malloc(sizeof(char));

//Modification Nabil, envoie du fichier

// variable ajoutée 
	int i,ligne,ret;
	i=0;
	ligne=0;
	FILE *fi;
	fi=fopen(nom_du_fichier,"r+"); //fichié ouvert en ecriture+lecture

	//Message pour le client

	message = "Envoie du fichier en cours\n";
	write(sock , message , strlen(message));
  	printf("%s",message);

	// on compte le nombre de ligne dans le fichier
	while(fgets(buff,LIGNE_MAX,fi)!=NULL)
	{
		ligne++;
	}

	printf("%d\n",ligne);
	//convertit ligne en char* et l'envoie
	sprintf(buff, "%d", ligne);

	ret = ecrireLigne(sock, buff);
	if (ret == -1) {
		erreur_IO("ecrireLigne");
	}

	printf("%s\n",buff);

	//on envoit le fichier
	fseek(fi,0,SEEK_SET); //on se remet au début du fichier
	for(i=0;i<ligne;i++)
	{
		if (fgets(buff, LIGNE_MAX,fi) == NULL) {
			erreur("fgets");
		}
		ret = ecrireLigne(sock, buff);
		if (ret == -1) {
			erreur_IO("ecrireLigne");
		}
	}


	fseek(fi,0,SEEK_SET); //on se remet au début du fichier

	message ="Fichier recu\n";
	write(sock , message , strlen(message));
	printf("%s",message);	

	fclose(fi);

// fin des modifications
	pthread_exit(flagStop);
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
		printf("address %s port %s unknow\n",ADDR_ANNUAIRE,PORT_ANNUAIRE);
	else{
		printf("Successfully resolved  %s, port %hu\n",
				stringIP(ntohl(address->sin_addr.s_addr)),
				ntohs(address->sin_port));
		/*Connexion a l'annuaire*/
		printf("Connecting the socket\n");
		ret = connect(sock, (struct sockaddr *) address, sizeof(struct sockaddr_in));
	//*
		if (ret >=0){
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
				//printf("%s Ping value : %d\n", SERVER_ANNUAIRE, pingValue);
				send(sock, (const void *) &pingValue, sizeof(pingValue),0);
				sleep(1);
			}
		}
		else{
			printf("Unable to join the annuaire, client can still use direct ip connection\n");
		}
		shutdown(sock,2);
		close(sock);
	}
	pthread_exit(NULL);
}


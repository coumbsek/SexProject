#ifndef INFO_THREAD_H
#define INFO_THREAD_H
#include "LockableFile.h"

typedef struct InfoThread{
	char isServer;
	FileL logFile;
	FileL datasFile;
	int sock;
	int thread_id;
	char isFree;
	char *ip;
	pthread_mutex_t mutex;	
}InfoThread;

#endif

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

	/*union{
		struct{
			int logFile;
			int sock;
			int thread_id;
			char isFree;
		}InfoThreadC;

		struct{
			int logFile;
			short datasServers;
			int sock;
			int thread_id;
			char isFree;
			int connexionPort;
		}InfoThreadS;
	};*/
}InfoThread;
#endif

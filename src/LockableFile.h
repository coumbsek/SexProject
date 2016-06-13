#ifndef LOCKABLE_FILE_H
#define LOCKABLE_FILE_H

#include <pthread.h>

typedef struct FileL{
	int fd;
	pthread_mutex_t mutex;
}FileL;
#endif

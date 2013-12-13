#ifndef IO_H_
#define IO_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <commons/collections/queue.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include "procer_defs.h"

typedef struct
{
	uint32_t ioTime;
	bool flagNonBlocking;
} t_ioInfo;

typedef struct
{
	t_ProcessImage* processImage;
	t_ioInfo* ioInfo;
} t_ioProcessing;

typedef struct
{
	sem_t *LPB_empty, *LFIO_empty, *STS_in_empty;
	t_queue *LPB, *LFIO;
	pthread_mutex_t *LPB_mutex, *LFIO_mutex, *log_mutex;
	sem_t *io_threads;
	int *IO_TIME;
} t_ioThreadsArgs;

void* ioThread(t_ioThreadsArgs*);
pthread_t* ioThreadsPoolCreate(t_ioThreadsArgs* ioThreadsArgs,
		uint32_t ioQuantityThreads);

#endif /* IO_H_ */

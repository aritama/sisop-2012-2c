#ifndef STSCONFIG_H_
#define STSCONFIG_H_

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <commons/config.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <commons/collections/queue.h>

typedef struct
{
	int priority;
	char* name;
	t_queue* queue;
} t_queuesArray;

typedef struct
{
	t_queue *LPN, *LPR, *LFIO, *LFQ, *LPL;
	sem_t *LPN_empty, *LPR_empty, *LFIO_empty, *LFQ_empty, *LPL_empty,
			*STS_in_empty;
	pthread_mutex_t *LPN_mutex, *LPR_mutex, *LFIO_mutex, *LFQ_mutex, *LPL_mutex,
			*ALG_mutex;
	t_queuesArray* queuesArray;
	pthread_mutex_t *log_mutex;
	char** ALG;
} t_STSargs;

void* STS(t_STSargs*);
void ordenarFIFO(t_STSargs* fakeSTSargs);
void ordenarPRI(t_STSargs* fakeSTSargs);
void orderInsertPrioList(t_list* prioList, t_STSargs* fakeSTSargs);
void ordenarSPN(t_STSargs* fakeSTSargs);
void orderInsertSPNList(t_list* prioList, t_STSargs* fakeSTSargs);
int strSwitch(char* alg);

typedef struct
{
	int *MPS, *MMP, *quantum, *PORT, *IO_TIME, *IO_THREADS;
	pthread_mutex_t *ALG_mutex, *QueuePriorities_mutex, *SENT_TIME_mutex;
	t_queuesArray* queuesArray;
	char** ALG;
	int *SENT_TIME;
	bool *flagReady;
	int *TANT;
	double *ALFA;
	t_STSargs* fakeSTSargs;
} t_configure;

#define EVENT_SIZE  ( sizeof (struct inotify_event) + 24 )
#define BUFF_SIZE ( 1024 * EVENT_SIZE )
void handle_error(int error);
void* configure(t_configure* configStruct);
void reorderQueuesArrayPriority(t_queuesArray*);
void configurationReader(t_config* config, t_configure* configStruct);

#endif /* STSCONFIG_H_ */

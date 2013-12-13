#define _GNU_SOURCE
#include "io.h"
#include "logs_def.h"

pthread_t* ioThreadsPoolCreate(t_ioThreadsArgs* ioThreadsArgs,
		uint32_t ioQuantityThreads)
{
	pthread_t* ioPool = NULL;
	uint32_t i;
	ioPool = malloc(sizeof(pthread_t) * ioQuantityThreads);
	for (i = 0; i < ioQuantityThreads; i++)
	{
		pthread_create(&ioPool[i], NULL, (void*) ioThread,
				(void*) ioThreadsArgs);
	}
	return ioPool;
}

void* ioThread(t_ioThreadsArgs* ioThreadsArgs)
{
	t_ioProcessing* ioProcessing;
	uint32_t sleepArg;
	t_logs* log = logCreate("I/O", ioThreadsArgs->log_mutex);
	char* msg = NULL;
	while (true)
	{
		sem_wait(ioThreadsArgs->LPB_empty);
		//para llevar la cuenta segura de los que estan ejecutando nomas:
		sem_wait(ioThreadsArgs->io_threads);
		pthread_mutex_lock(ioThreadsArgs->LPB_mutex);
		ioProcessing = queue_pop(ioThreadsArgs->LPB);
		pthread_mutex_unlock(ioThreadsArgs->LPB_mutex);
		sleepArg = (uint32_t) (ioProcessing->ioInfo->ioTime
				* (uint32_t) (*ioThreadsArgs->IO_TIME));
		if (!ioProcessing->ioInfo->flagNonBlocking)
		{ //si es bloqueante, lo mete a LFIO
			asprintf(&msg, "PID:%u; Realizando E/S, bloqueante",
					ioProcessing->processImage->pid);
			logWriteINFO(log, msg);
			free(msg);
			sleep(sleepArg);
			logWriteLSCH(log, ioProcessing->processImage->pid, "Bloqueados",
					"Fin I/0");
			pthread_mutex_lock(ioThreadsArgs->LFIO_mutex);
			queue_push(ioThreadsArgs->LFIO, ioProcessing->processImage);
			sem_post(ioThreadsArgs->LFIO_empty);
			sem_post(ioThreadsArgs->STS_in_empty);
			pthread_mutex_unlock(ioThreadsArgs->LFIO_mutex);

		} // si era no bloqueante, ademas solo libera, no mete a LFIO
		else
		{
			asprintf(&msg, "PID:%u; Realizando E/S, no bloqueante",
					ioProcessing->processImage->pid);
			logWriteINFO(log, msg);
			free(msg);
			sleep(sleepArg);
		}
		free(ioProcessing->ioInfo);
		free(ioProcessing);
		sem_post(ioThreadsArgs->io_threads); // terminó de ejecutar, un hilo disponible más
		// processImage, no se libera la memoria alocada, esta en LFIO o está siendo utilizada por PROCER
	}
	logDestroy(log);
	return NULL ;
}

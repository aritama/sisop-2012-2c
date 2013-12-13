#include "procer_defs.h"
#include "LTS.h"
#include "logs_def.h"

void* PROCER(t_PROCERargs* procerArgs)
{
	t_logs* log = logCreate("PROCER", procerArgs->log_mutex);
	t_msg *msg = NULL;
	int quantumCount = 0;
	double rafaga = 0;
	bool ioBlockInterrupt = false;
	while (true)
	{
		sem_wait(procerArgs->LPL_empty);
		pthread_mutex_lock(procerArgs->LPL_mutex);
		t_ProcessImage* processImage = queue_pop(procerArgs->LPL);
		pthread_mutex_unlock(procerArgs->LPL_mutex);
		logWriteLSCH(log, processImage->pid, "Listos", "Ejecución");
		char** code = codeSegmentCreator(processImage->code); // este lo tengo que eliminar cada vez
		// porque cada vez que vengo lo hago ademas

		quantumCount = 0;
		rafaga = 0;
		ioBlockInterrupt = false;
		*procerArgs->SUSPEND = false;
		processExecution(code, processImage, procerArgs, &ioBlockInterrupt,
				&quantumCount, &rafaga);
		processImage->Tspn = (((processImage->Tspn) * (*procerArgs->ALFA))
				+ (rafaga * (1 - (*procerArgs->ALFA))));

		if ((strcmp(code[processImage->PC], "fin_programa") == 0)) // salio por fin_programa, y no por suspension
		{
			logWriteLSCH(log, processImage->pid, "Ejecución", "Finalizado");
			(*(procerArgs->SUSPEND)) = false;
			ioBlockInterrupt = false;
			quantumCount = -1;
			t_msg *msg = malloc(sizeof(t_msg));
			msg->type = 'T'; // terminó
			msg->fd = processImage->clientfd;
			msg->msg = prepareMessage(processImage);
			pthread_mutex_lock(procerArgs->sendingList_mutex);
			list_add(procerArgs->sendingList, msg);
			pthread_mutex_unlock(procerArgs->sendingList_mutex);

			list_destroy(processImage->stack);
			free(processImage->data);
			free(processImage->code);
			free(processImage);
			sem_post(procerArgs->MMP_sem);
			sem_post(procerArgs->MPS_sem);
			codeSegmentDestructor(&code);
			continue;
		}

		if (ioBlockInterrupt)
		{
			*procerArgs->SUSPEND = false; // si justo se suspende, tiene prioridad que terminó
			quantumCount = -1;
			codeSegmentDestructor(&code);
			continue;
		}

		if (*procerArgs->SUSPEND
				&& (strcmp(code[processImage->PC], "fin_programa") != 0)
				&& !ioBlockInterrupt) //salió por suspención,
		{
			quantumCount = -1; // tiene prioridad a que justo se termino el quantum
			msg = malloc(sizeof(t_msg));
			msg->fd = processImage->clientfd;
			msg->type = 'S';
			msg->msg = prepareMessage(processImage);

			logWriteLSCH(log, processImage->pid, "Ejecución", "Suspendido");
			pthread_mutex_lock(procerArgs->LPS_mutex);
			list_add(procerArgs->LPS, processImage);
			sem_post(procerArgs->MMP_sem);
			pthread_mutex_unlock(procerArgs->LPS_mutex);
			pthread_mutex_lock(procerArgs->sendingList_mutex);
			list_add(procerArgs->sendingList, msg);
			pthread_mutex_unlock(procerArgs->sendingList_mutex);
			(*(procerArgs->SUSPEND)) = false;
			codeSegmentDestructor(&code);
			continue;
		} // sino puede ser fin_Q , o bloqueado, o que terminó

		if (quantumCount == (*procerArgs->quantum))
		{
			logWriteLSCH(log, processImage->pid, "Ejecución", "Fin De Quantum");
			pthread_mutex_lock(procerArgs->LFQ_mutex);
			queue_push(procerArgs->LFQ, processImage);
			sem_post(procerArgs->LFQ_empty);
			sem_post(procerArgs->STS_in_empty);
			pthread_mutex_unlock(procerArgs->LFQ_mutex);
			codeSegmentDestructor(&code);
			continue;
		}
// si salió por bloqueado no hago nada, ya está en lista de bloqueados
	}
	logDestroy(log);
	return NULL ;
}

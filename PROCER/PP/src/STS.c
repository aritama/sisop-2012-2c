#define _GNU_SOURCE
#include "STS.h"
#include "procer_defs.h"
#include "logs_def.h"

void* STS(t_STSargs* STSargs)
{
	t_logs* log = logCreate("STS", STSargs->log_mutex);
	char * alg = NULL;
	while (true)
	{
		sem_wait(STSargs->STS_in_empty); // espera que haya para consumir, de sale pero saca uno
		sem_post(STSargs->STS_in_empty); // meto ese que saque.. y tendria cantidad original
		pthread_mutex_lock(STSargs->ALG_mutex);
		asprintf(&alg, "%s", *STSargs->ALG);
		pthread_mutex_unlock(STSargs->ALG_mutex);
		switch (strSwitch(alg))
		{
		case 1:
			ordenarFIFO(STSargs);
			break;
		case 2:
			ordenarFIFO(STSargs);
			break;
		case 3:
			ordenarPRI(STSargs);
			break;
		case 4:
			ordenarSPN(STSargs);
			break;
		default:
			logWriteERROR(log, "Algoritmo no reconocido");
			exit(EXIT_FAILURE);
			break;
		} // switch end
		free(alg);
	}
	logDestroy(log);
	return NULL ;
}

int strSwitch(char* alg)
{
	if (!strcmp(alg, "FIFO"))
		return 1;
	if (!strcmp(alg, "RR"))
		return 2;
	if (!strcmp(alg, "PRI"))
		return 3;
	if (!strcmp(alg, "SPN"))
		return 4;
	if (!strcmp(alg, "PLPN"))
		return 5;
	if (!strcmp(alg, "PLPR"))
		return 6;
	if (!strcmp(alg, "PLFQ"))
		return 7;
	if (!strcmp(alg, "PLFIO"))
		return 8;
	return -1;
}

void ordenarSPN(t_STSargs* STSargs)
{
	t_logs* log = logCreate("STS", STSargs->log_mutex);
	t_list* prioList = list_create();
	uint8_t i;
	t_ProcessImage* processImg = NULL;
	for (i = 0; i < 4; i++)
	{
		switch (strSwitch(STSargs->queuesArray[i].name))
		{
		case 5:
		{
			while (sem_trywait(STSargs->LPN_empty) != -1)
			{
				sem_trywait(STSargs->STS_in_empty);
				pthread_mutex_lock(STSargs->LPN_mutex);
				processImg = queue_pop(STSargs->LPN);
				pthread_mutex_unlock(STSargs->LPN_mutex);
				logWriteLSCH(log, processImg->pid, "Nuevos", "Listos");
				list_add(prioList, processImg);
			}
			break;
		}
		case 6:
		{
			while (sem_trywait(STSargs->LPR_empty) != -1)
			{
				sem_trywait(STSargs->STS_in_empty);
				pthread_mutex_lock(STSargs->LPR_mutex);
				processImg = queue_pop(STSargs->LPR);
				pthread_mutex_unlock(STSargs->LPR_mutex);
				logWriteLSCH(log, processImg->pid, "Reanudados", "Listos");
				list_add(prioList, processImg);
			}
			break;
		}
		case 7:
		{
			while (sem_trywait(STSargs->LFQ_empty) != -1)
			{
				sem_trywait(STSargs->STS_in_empty);
				pthread_mutex_lock(STSargs->LFQ_mutex);
				processImg = queue_pop(STSargs->LFQ);
				pthread_mutex_unlock(STSargs->LFQ_mutex);
				logWriteLSCH(log, processImg->pid, "Fin de Quantum", "Listos");
				list_add(prioList, processImg);
			}
			break;
		}
		case 8:
		{
			while (sem_trywait(STSargs->LFIO_empty) != -1)
			{
				sem_trywait(STSargs->STS_in_empty);
				pthread_mutex_lock(STSargs->LFIO_mutex);
				processImg = queue_pop(STSargs->LFIO);
				pthread_mutex_unlock(STSargs->LFIO_mutex);
				logWriteLSCH(log, processImg->pid, "Fin de I/O", "Listos");
				list_add(prioList, processImg);
			}
			break;
		}
		default:
			break;
		}
	}
	processImg = NULL;

	orderInsertSPNList(prioList, STSargs);
	list_destroy(prioList);
	return;

	return;
}

void orderInsertSPNList(t_list* prioList, t_STSargs* STSargs)
{
	pthread_mutex_lock(STSargs->LPL_mutex);
	t_ProcessImage* psImg1, *psImg2;
	int i, queueSize = queue_size(STSargs->LPL);
	int result = 0;
	for (i = 0; (i < queueSize) && (result != -1); i++)
	{
		result = sem_trywait(STSargs->LPL_empty);
		psImg2 = queue_pop(STSargs->LPL);
		list_add_in_index(prioList, 0, psImg2);
	}

	int ilimit, listSize = list_size(prioList);
	bool flag = false;
	ilimit = listSize - 1;
	while (!flag)
	{
		flag = true;
		for (i = 0; i < ilimit; i++)
		{
			psImg1 = list_get(prioList, i);
			psImg2 = list_get(prioList, i + 1);

			if (psImg1->Tspn > psImg2->Tspn)
			{
				if (ilimit - i == 1)
				{
					psImg1 = list_remove(prioList, i);
					list_add(prioList, psImg1);
				}
				else
				{
					psImg1 = list_remove(prioList, i);
					list_add_in_index(prioList, i + 1, psImg1);
				}
				flag = false;
			}
		}
	}
	listSize = list_size(prioList);
	for (i = 0; i < listSize; i++)
	{
		psImg2 = list_remove(prioList, 0);
		queue_push(STSargs->LPL, psImg2);
		sem_post(STSargs->LPL_empty);
	}
	pthread_mutex_unlock(STSargs->LPL_mutex);
	return;
}

void ordenarPRI(t_STSargs* STSargs)
{
	t_logs* log = logCreate("STS", STSargs->log_mutex);
	t_list* prioList = list_create();
	uint8_t i;
	t_ProcessImage* processImg = NULL;
	for (i = 0; i < 4; i++)
	{
		switch (strSwitch(STSargs->queuesArray[i].name))
		{
		case 5:
		{
			while (sem_trywait(STSargs->LPN_empty) != -1)
			{
				sem_trywait(STSargs->STS_in_empty);
				pthread_mutex_lock(STSargs->LPN_mutex);
				processImg = queue_pop(STSargs->LPN);
				pthread_mutex_unlock(STSargs->LPN_mutex);
				logWriteLSCH(log, processImg->pid, "Nuevos", "Listos");
				list_add(prioList, processImg);
			}
			break;
		}
		case 6:
		{
			while (sem_trywait(STSargs->LPR_empty) != -1)
			{
				sem_trywait(STSargs->STS_in_empty);
				pthread_mutex_lock(STSargs->LPR_mutex);
				processImg = queue_pop(STSargs->LPR);
				pthread_mutex_unlock(STSargs->LPR_mutex);
				logWriteLSCH(log, processImg->pid, "Reanudados", "Listos");
				list_add(prioList, processImg);
			}
			break;
		}
		case 7:
		{
			while (sem_trywait(STSargs->LFQ_empty) != -1)
			{
				sem_trywait(STSargs->STS_in_empty);
				pthread_mutex_lock(STSargs->LFQ_mutex);
				processImg = queue_pop(STSargs->LFQ);
				pthread_mutex_unlock(STSargs->LFQ_mutex);
				logWriteLSCH(log, processImg->pid, "Fin de Quantum", "Listos");
				list_add(prioList, processImg);
			}
			break;
		}
		case 8:
		{
			while (sem_trywait(STSargs->LFIO_empty) != -1)
			{
				sem_trywait(STSargs->STS_in_empty);
				pthread_mutex_lock(STSargs->LFIO_mutex);
				processImg = queue_pop(STSargs->LFIO);
				pthread_mutex_unlock(STSargs->LFIO_mutex);
				logWriteLSCH(log, processImg->pid, "Fin de I/O", "Listos");
				list_add(prioList, processImg);
			}
			break;
		}
		default:
			break;
		}
	}
	processImg = NULL;
	logDestroy(log);
	orderInsertPrioList(prioList, STSargs);
	list_destroy(prioList);
	return;
}

void orderInsertPrioList(t_list* prioList, t_STSargs* STSargs)
{
	pthread_mutex_lock(STSargs->LPL_mutex);
	int i, queueSize = queue_size(STSargs->LPL);
	int result = 0;
	t_ProcessImage* psImg1, *psImg2;
	for (i = 0; (i < queueSize) && (result != -1); i++)
	{
		result = sem_trywait(STSargs->LPL_empty);
		psImg2 = queue_pop(STSargs->LPL);
		list_add_in_index(prioList, 0, psImg2);
	}
	int ilimit, listSize = list_size(prioList);
	bool flag = false;
	ilimit = listSize - 1;
	while (!flag)
	{
		flag = true;
		for (i = 0; i < ilimit; i++)
		{
			psImg1 = list_get(prioList, i);
			psImg2 = list_get(prioList, i + 1);
			if (psImg1->priority > psImg2->priority)
			{
				if (ilimit - i == 1)
				{
					psImg1 = list_remove(prioList, i);
					list_add(prioList, psImg1);
				}
				else
				{
					psImg1 = list_remove(prioList, i);
					list_add_in_index(prioList, i + 1, psImg1);
				}
				flag = false;
			}
		}
	}
	listSize = list_size(prioList);
	for (i = 0; i < listSize; i++)
	{
		psImg2 = list_remove(prioList, 0);
		queue_push(STSargs->LPL, psImg2);
		sem_post(STSargs->LPL_empty);
	}
	pthread_mutex_unlock(STSargs->LPL_mutex);
	return;
}

void ordenarFIFO(t_STSargs* STSargs)
{
	t_logs* log = logCreate("STS", STSargs->log_mutex);
	uint8_t i;
	t_ProcessImage* processImg = NULL;
	for (i = 0; i < 4; i++)
	{
		switch (strSwitch(STSargs->queuesArray[i].name))
		{
		case 5:
		{
			while (sem_trywait(STSargs->LPN_empty) != -1)
			{
				sem_trywait(STSargs->STS_in_empty);
				pthread_mutex_lock(STSargs->LPN_mutex);
				processImg = queue_pop(STSargs->LPN);
				pthread_mutex_unlock(STSargs->LPN_mutex);
				logWriteLSCH(log, processImg->pid, "Nuevos", "Listos");
				pthread_mutex_lock(STSargs->LPL_mutex);
				queue_push(STSargs->LPL, processImg);
				sem_post(STSargs->LPL_empty);
				pthread_mutex_unlock(STSargs->LPL_mutex);
			}
			break;
		}
		case 6:
		{
			while (sem_trywait(STSargs->LPR_empty) != -1)
			{
				sem_trywait(STSargs->STS_in_empty);
				pthread_mutex_lock(STSargs->LPR_mutex);
				processImg = queue_pop(STSargs->LPR);
				pthread_mutex_unlock(STSargs->LPR_mutex);
				logWriteLSCH(log, processImg->pid, "Reanudados", "Listos");
				pthread_mutex_lock(STSargs->LPL_mutex);
				queue_push(STSargs->LPL, processImg);
				sem_post(STSargs->LPL_empty);
				pthread_mutex_unlock(STSargs->LPL_mutex);
			}
			break;
		}
		case 7:
		{
			while (sem_trywait(STSargs->LFQ_empty) != -1)
			{
				sem_trywait(STSargs->STS_in_empty);
				pthread_mutex_lock(STSargs->LFQ_mutex);
				processImg = queue_pop(STSargs->LFQ);
				pthread_mutex_unlock(STSargs->LFQ_mutex);
				logWriteLSCH(log, processImg->pid, "Fin de Quantum", "Listos");
				pthread_mutex_lock(STSargs->LPL_mutex);
				queue_push(STSargs->LPL, processImg);
				sem_post(STSargs->LPL_empty);
				pthread_mutex_unlock(STSargs->LPL_mutex);
			}
			break;
		}
		case 8:
		{
			while (sem_trywait(STSargs->LFIO_empty) != -1)
			{
				sem_trywait(STSargs->STS_in_empty);
				pthread_mutex_lock(STSargs->LFIO_mutex);
				processImg = queue_pop(STSargs->LFIO);
				pthread_mutex_unlock(STSargs->LFIO_mutex);
				logWriteLSCH(log, processImg->pid, "Fin de I/O", "Listos");
				pthread_mutex_lock(STSargs->LPL_mutex);
				queue_push(STSargs->LPL, processImg);
				sem_post(STSargs->LPL_empty);
				pthread_mutex_unlock(STSargs->LPL_mutex);
			}
			break;
		}
		default:
			break;
		}
	}
	logDestroy(log);
	processImg = NULL;
	return;
}

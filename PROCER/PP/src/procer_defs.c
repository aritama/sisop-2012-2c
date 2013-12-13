#include "procer_defs.h"
#include "LTS.h"
#include "io.h"
#include "logs_def.h"

void processExecution(char** code, t_ProcessImage* processImage,
		t_PROCERargs* procerArgs, bool* ioBlockInterrupt, int* quantumCount,
		double* rafaga)
{
	t_logs* log = logCreate("PROCER", procerArgs->log_mutex);
	t_data** data = &(processImage->data);
	t_list** stack = &(processImage->stack);
	uint16_t *PC = &(processImage->PC);
	t_ioProcessing* ioProcessing = NULL;
	t_ioInfo *ioInfo = NULL;
	int semValue;

	stackExecution(code, processImage, procerArgs, ioBlockInterrupt,
			quantumCount, rafaga);

	int varValue;
	char varName;
	uint32_t i, ind, ioTime; // auxiliares
	char* auxString = NULL, *ioTimeString = NULL;
	t_msg* printMsg;
	t_stack* newElement = NULL;

	while (strcmp(code[*PC], "fin_programa") != 0 && !(*procerArgs->SUSPEND)
			&& (!(*ioBlockInterrupt))
			&& (*quantumCount < (*procerArgs->quantum)))
	{

		if (!list_is_empty(*stack))
		{
			stackExecution(code, processImage, procerArgs, ioBlockInterrupt,
					quantumCount, rafaga);
			continue;
		}

		(*PC)++;
		if (strcmp(code[*PC], "") == 0 || string_ends_with(code[*PC], ":"))
			continue;

		if (string_starts_with(code[*PC], "variables"))
		{
			addVariables(data, code[*PC] + strlen("variables,"));
			for (ind = 0; code[*PC][ind] != '\0' && code[*PC][ind] != ';';
					ind++)
				;
			pthread_mutex_lock(procerArgs->SENT_TIME_mutex);
			pthread_mutex_unlock(procerArgs->SENT_TIME_mutex);
			pthread_mutex_lock(procerArgs->ALG_mutex);
			pthread_mutex_unlock(procerArgs->ALG_mutex);
			continue;
		}

		if (string_starts_with(code[*PC], "imprimir"))
		{
			varName = code[*PC][strlen("imprimir,")];
			varValue = readData(data, varName);
			printMsg = malloc(sizeof(t_msg));
			printMsg->type = 'N';
			printMsg->fd = processImage->clientfd;
			asprintf(&printMsg->msg, "IMPRIMIENDO VARIABLE %c: %d\n", varName,
					varValue);
			pthread_mutex_lock(procerArgs->sendingList_mutex);
			list_add(procerArgs->sendingList, printMsg);
			pthread_mutex_unlock(procerArgs->sendingList_mutex);
			pthread_mutex_lock(procerArgs->SENT_TIME_mutex);
//			sleep(*procerArgs->SENT_TIME);
			pthread_mutex_unlock(procerArgs->SENT_TIME_mutex);
			pthread_mutex_lock(procerArgs->ALG_mutex);
//			if (!strcmp(*procerArgs->ALG, "RR"))
//				(*quantumCount)++;
			pthread_mutex_unlock(procerArgs->ALG_mutex);
//			(*rafaga)++;
			continue;
		}

		if (string_starts_with(code[*PC], "ssc"))
		{
			varName = code[*PC][strlen("ssc,")];
			varValue = readData(data, varName);
			auxString = code[*PC];
			if (varValue == 0)
				changePC(PC, code, code[*PC] + strlen("ssc,x,"));
			for (i = 0; auxString[i] != ';' && auxString[i] != '\0'; i++)
				;
			pthread_mutex_lock(procerArgs->SENT_TIME_mutex);
			sleep(*procerArgs->SENT_TIME);
			if (auxString[i] == ';')
				sleep(atoi(auxString + i + 1));
			else
				sleep(*procerArgs->SENT_TIME);
			pthread_mutex_unlock(procerArgs->SENT_TIME_mutex);
			pthread_mutex_lock(procerArgs->ALG_mutex);
			if (!strcmp(*procerArgs->ALG, "RR"))
				(*quantumCount)++;
			pthread_mutex_unlock(procerArgs->ALG_mutex);
			(*rafaga)++;
			continue;
		}

		if (string_starts_with(code[*PC], "snc"))
		{
			varName = code[*PC][strlen("snc,")];
			varValue = readData(data, varName);
			auxString = code[*PC];
			if (varValue != 0)
				changePC(PC, code, code[*PC] + strlen("snc,x,"));
			for (i = 0; auxString[i] != ';' && auxString[i] != '\0'; i++)
				;
			pthread_mutex_lock(procerArgs->SENT_TIME_mutex);
			if (auxString[i] == ';')
				sleep(atoi(auxString + i + 1));
			else
				sleep(*procerArgs->SENT_TIME);
			pthread_mutex_unlock(procerArgs->SENT_TIME_mutex);
			pthread_mutex_lock(procerArgs->ALG_mutex);
			if (!strcmp(*procerArgs->ALG, "RR"))
				(*quantumCount)++;
			pthread_mutex_unlock(procerArgs->ALG_mutex);
			(*rafaga)++;
			continue;
		}

		if (code[*PC][1] == '=' && !string_starts_with((code[*PC] + 2), "io("))
		{
			auxString = NULL;
			varValue = 0;
			i = 1;

			while (code[*PC][i] != '\0' && code[*PC][i] != ';')
			{
				varName = code[*PC][i];
				i++;
				ind = 0;
				while (code[*PC][i] != '\0' && code[*PC][i] != '+'
						&& code[*PC][i] != '-' && code[*PC][i] != ';')
				{
					auxString = realloc(auxString, (ind + 1) * sizeof(char));
					auxString[ind] = code[*PC][i];
					i++;
					ind++;
				}
				auxString = realloc(auxString, (ind + 1) * sizeof(char));
				auxString[ind] = '\0';
				if (strlen(auxString) > 1)
				{
					if (varName != '-')
						varValue += atoi(auxString);
					else
						varValue -= atoi(auxString);
				}
				else
				{
					if (varName != '-')
					{
						varValue += readData(data, auxString[0]);
					}
					else
						varValue -= readData(data, auxString[0]);
				}
				free(auxString);
				auxString = NULL;
			}
			writeData(data, code[*PC][0], varValue);
			pthread_mutex_lock(procerArgs->SENT_TIME_mutex);
			if (code[*PC][i] == ';')
				sleep(atoi(code[*PC] + i + 1));
			else
				sleep(*procerArgs->SENT_TIME);
			pthread_mutex_unlock(procerArgs->SENT_TIME_mutex);
			pthread_mutex_lock(procerArgs->ALG_mutex);
			if (!strcmp(*procerArgs->ALG, "RR"))
				(*quantumCount)++;
			pthread_mutex_unlock(procerArgs->ALG_mutex);
			(*rafaga)++;
			continue;
		}

		if (code[*PC][1] == '=' && string_starts_with((code[*PC] + 2), "io("))
		{
			varName = code[*PC][0];
			auxString = code[*PC] + strlen("io(") + 2;
			for (ind = 0; auxString[ind] != ','; ind++)
				;
			ioTimeString = realloc(ioTimeString, (ind + 1));
			memcpy(ioTimeString, auxString, ind * sizeof(char));
			ioTimeString[ind] = '\0';
			ioTime = atoi(ioTimeString);
			free(ioTimeString);
			ioTimeString = NULL;

			if (auxString[ind + 1] == '0')
			{
				pthread_mutex_lock(procerArgs->LPB_mutex);
				sem_getvalue(procerArgs->ioThreads, &semValue); //cantidad de ioT libres
				if (queue_size(procerArgs->LPB) < semValue)
				{ // si es igual o mayor, tardaria en atender
					pthread_mutex_lock(procerArgs->LPB_mutex);
					logWriteLSCH(log, processImage->pid, "Ejecución",
							"Bloqueado");
					ioProcessing = malloc(sizeof(t_ioProcessing));
					ioInfo = malloc(sizeof(t_ioInfo));
					ioInfo->flagNonBlocking = false;
					ioInfo->ioTime = ioTime;
					ioProcessing->ioInfo = ioInfo;
					ioProcessing->processImage = processImage;
					queue_push(procerArgs->LPB, ioProcessing);
					varValue = 1;
					writeData(data, varName, varValue);
					sem_post(procerArgs->LPB_empty);
					pthread_mutex_unlock(procerArgs->LPB_mutex);
					*ioBlockInterrupt = true;
				}
				else
				{
					pthread_mutex_unlock(procerArgs->LPB_mutex);
					printMsg = malloc(sizeof(t_msg));
					asprintf(&printMsg->msg,
							"PID:%d; ERROR I/O no-bloqueante, hilos no disponibles, línea %u\n",
							processImage->pid, (*PC) + 1);
					varValue = 0;
					logWriteERROR(log, printMsg->msg);
					writeData(data, varName, varValue);
					printMsg->fd = processImage->clientfd;
					printMsg->type = 'N';
					pthread_mutex_lock(procerArgs->sendingList_mutex);
					list_add(procerArgs->sendingList, printMsg);
					pthread_mutex_unlock(procerArgs->sendingList_mutex);
					printMsg = NULL;
				}

			}
			else
			{
				pthread_mutex_lock(procerArgs->LPB_mutex);
				logWriteLSCH(log, processImage->pid, "Ejecución", "Bloqueado");
				ioProcessing = malloc(sizeof(t_ioProcessing));
				ioInfo = malloc(sizeof(t_ioInfo));
				ioInfo->flagNonBlocking = false;
				ioInfo->ioTime = ioTime;
				ioProcessing->ioInfo = ioInfo;
				ioProcessing->processImage = processImage;
				queue_push(procerArgs->LPB, ioProcessing);
				varValue = 1;
				writeData(data, varName, varValue);
				sem_post(procerArgs->LPB_empty);
				pthread_mutex_unlock(procerArgs->LPB_mutex);
				*ioBlockInterrupt = true;
			}
			ioProcessing = NULL;
			ioInfo = NULL;
			auxString = NULL;
			pthread_mutex_lock(procerArgs->SENT_TIME_mutex);
			sleep(*procerArgs->SENT_TIME);
			pthread_mutex_unlock(procerArgs->SENT_TIME_mutex);
			pthread_mutex_lock(procerArgs->ALG_mutex);
			if (!strcmp(*procerArgs->ALG, "RR"))
				(*quantumCount)++;
			pthread_mutex_unlock(procerArgs->ALG_mutex);
			(*rafaga)++;
			continue;
		}
		if (string_starts_with(code[*PC], "io("))
		{
			auxString = code[*PC] + strlen("io(");
			for (ind = 0; auxString[ind] != ','; ind++)
				;
			ioTimeString = realloc(ioTimeString, (ind + 1));
			memcpy(ioTimeString, auxString, ind * sizeof(char));
			ioTimeString[ind] = '\0';
			ioTime = atoi(ioTimeString);
			free(ioTimeString);
			ioTimeString = NULL;

			if (auxString[ind + 1] == '0')
			{ //no block
				pthread_mutex_lock(procerArgs->LPB_mutex);
				sem_getvalue(procerArgs->ioThreads, &semValue); //cantidad de ioT libres
				if (queue_size(procerArgs->LPB) < semValue)
				{ // si es igual o mayor, tardaria en atender
					ioProcessing = malloc(sizeof(t_ioProcessing));
					ioInfo = malloc(sizeof(t_ioInfo));
					ioInfo->flagNonBlocking = true;
					ioInfo->ioTime = ioTime;
					ioProcessing->ioInfo = ioInfo;
					ioProcessing->processImage = processImage;
					queue_push(procerArgs->LPB, ioProcessing);
					sem_post(procerArgs->LPB_empty);
					pthread_mutex_unlock(procerArgs->LPB_mutex);
				}
				else
				{
					pthread_mutex_unlock(procerArgs->LPB_mutex);
					printMsg = malloc(sizeof(t_msg));
					asprintf(&printMsg->msg,
							"ERROR I/O no-bloqueante, hilos no disponibles, línea %u\n",
							(*PC) + 1);
					logWriteERROR(log, printMsg->msg);
					printMsg->fd = processImage->clientfd;
					printMsg->type = 'N';
					pthread_mutex_lock(procerArgs->sendingList_mutex);
					list_add(procerArgs->sendingList, printMsg);
					pthread_mutex_unlock(procerArgs->sendingList_mutex);
					printMsg = NULL;
				}

			}
			else // block io
			{
				pthread_mutex_lock(procerArgs->LPB_mutex);
				logWriteLSCH(log, processImage->pid, "Ejecución", "Bloqueado");
				ioProcessing = malloc(sizeof(t_ioProcessing));
				ioInfo = malloc(sizeof(t_ioInfo));
				ioInfo->flagNonBlocking = false;
				ioInfo->ioTime = ioTime;
				ioProcessing->ioInfo = ioInfo;
				ioProcessing->processImage = processImage;
				queue_push(procerArgs->LPB, ioProcessing);
				sem_post(procerArgs->LPB_empty);
				pthread_mutex_unlock(procerArgs->LPB_mutex);
				*ioBlockInterrupt = true;
			}
			ioProcessing = NULL;
			ioInfo = NULL;
			auxString = NULL;
			pthread_mutex_lock(procerArgs->SENT_TIME_mutex);
//			sleep(*procerArgs->SENT_TIME);
			pthread_mutex_unlock(procerArgs->SENT_TIME_mutex);
			pthread_mutex_lock(procerArgs->ALG_mutex);
//			if (!strcmp(*procerArgs->ALG, "RR"))
//				(*quantumCount)++;
			pthread_mutex_unlock(procerArgs->ALG_mutex);
//			(*rafaga)++;
			continue;
		}

		if (string_ends_with(code[*PC], "()"))
		{
			varValue = 0;
			for (ind = 0; code[*PC][ind] != '('; ind++)
				;
			auxString = malloc((ind + 1) * sizeof(char));
			memcpy(auxString, code[*PC], ind);
			auxString[ind] = '\0';

			newElement = malloc(sizeof(t_stack));
			newElement->linecall = (*PC);
			newElement->functionName = auxString;
			auxString = NULL;
			newElement->PC = findFunctionLine(code, newElement->functionName);
			list_add(*stack, newElement);
			stackExecution(code, processImage, procerArgs, ioBlockInterrupt,
					quantumCount, rafaga);
			continue;
		}
	}
	logDestroy(log);
	return;
}

void functionExecution(char** code, t_ProcessImage* processImage,
		t_stack** element, t_PROCERargs* procerArgs, bool* ioBlockInterrupt,
		int* quantumCount, double *rafaga)
{
	t_logs* log = logCreate("PROCER", procerArgs->log_mutex);
	t_list** stack = &processImage->stack;
	t_data** data = &processImage->data;
	t_ioProcessing* ioProcessing = NULL;
	t_ioInfo* ioInfo = NULL;
	int semValue;

	t_msg *printMsg;
	int varValue;
	char varName;
	uint16_t i, ind, ioTime;
	char* auxString = NULL, *ioTimeString = NULL;
	t_stack* newElement = NULL;
	uint16_t *PC = &((*element)->PC);

	while (((!string_starts_with(code[*PC], "fin_funcion"))
			|| (!string_ends_with(code[*PC], (*element)->functionName)))
			&& !(*procerArgs->SUSPEND) && (!(*ioBlockInterrupt))
			&& ((*quantumCount) < (*procerArgs->quantum)))
	{
		(*PC)++;
		if (strcmp(code[*PC], "") == 0 || string_ends_with(code[*PC], ":"))
			continue;

		if (string_starts_with(code[*PC], "imprimir"))
		{
			varName = code[*PC][strlen("imprimir,")];
			varValue = readData(data, varName);
			printMsg = malloc(sizeof(t_msg));
			printMsg->fd = processImage->clientfd;
			printMsg->type = 'N';
			asprintf(&printMsg->msg, "IMPRIMIENDO VARIABLE %c: %d\n", varName,
					varValue);
			pthread_mutex_lock(procerArgs->sendingList_mutex);
			list_add(procerArgs->sendingList, printMsg);
			pthread_mutex_unlock(procerArgs->sendingList_mutex);
			pthread_mutex_lock(procerArgs->SENT_TIME_mutex);
//			sleep(*procerArgs->SENT_TIME);
			pthread_mutex_unlock(procerArgs->SENT_TIME_mutex);
			pthread_mutex_lock(procerArgs->ALG_mutex);
//			if (!strcmp(*procerArgs->ALG, "RR"))
//				(*quantumCount)++;
			pthread_mutex_unlock(procerArgs->ALG_mutex);
//			(*rafaga)++;
			continue;
		}

		if (code[*PC][1] == '=' && !string_starts_with((code[*PC] + 2), "io("))
		{
			auxString = NULL;
			varValue = 0;
			i = 1;

			while (code[*PC][i] != '\0' && code[*PC][i] != ';')
			{
				varName = code[*PC][i];
				i++;
				ind = 0;
				while (code[*PC][i] != '\0' && code[*PC][i] != '+'
						&& code[*PC][i] != '-' && code[*PC][i] != ';')
				{
					auxString = realloc(auxString, (ind + 1) * sizeof(char));
					auxString[ind] = code[*PC][i];
					i++;
					ind++;
				}
				auxString = realloc(auxString, (ind + 1) * sizeof(char));
				auxString[ind] = '\0';
				if (strlen(auxString) > 1)
				{
					if (varName != '-')
						varValue += atoi(auxString);
					else
						varValue -= atoi(auxString);
				}
				else
				{
					if (varName != '-')
					{
						varValue += readData(data, auxString[0]);
					}
					else
						varValue -= readData(data, auxString[0]);
				}
				free(auxString);
				auxString = NULL;
			}
			writeData(data, code[*PC][0], varValue);
			pthread_mutex_lock(procerArgs->SENT_TIME_mutex);
			if (code[*PC][i] == ';')
				sleep(atoi(code[*PC] + i + 1));
			else
				sleep(*procerArgs->SENT_TIME);
			pthread_mutex_unlock(procerArgs->SENT_TIME_mutex);
			pthread_mutex_lock(procerArgs->ALG_mutex);
			if (!strcmp(*procerArgs->ALG, "RR"))
				(*quantumCount)++;
			pthread_mutex_unlock(procerArgs->ALG_mutex);
			(*rafaga)++;
			continue;
		}

		if (code[*PC][1] == '=' && string_starts_with((code[*PC] + 2), "io("))
		{
			varName = code[*PC][0];
			auxString = code[*PC] + strlen("io(") + 2;
			for (ind = 0; auxString[ind] != ','; ind++)
				;
			ioTimeString = realloc(ioTimeString, (ind + 1));
			memcpy(ioTimeString, auxString, ind * sizeof(char));
			ioTimeString[ind] = '\0';
			ioTime = atoi(ioTimeString);
			free(ioTimeString);
			ioTimeString = NULL;

			if (auxString[ind + 1] == '0')
			{
				pthread_mutex_lock(procerArgs->LPB_mutex);
				sem_getvalue(procerArgs->ioThreads, &semValue); //cantidad de ioT libres
				if (queue_size(procerArgs->LPB) < semValue)
				{ // si es igual o mayor, tardaria en atender
					ioProcessing = malloc(sizeof(t_ioProcessing));
					ioInfo = malloc(sizeof(t_ioInfo));
					ioInfo->flagNonBlocking = true;
					ioInfo->ioTime = ioTime;
					ioProcessing->ioInfo = ioInfo;
					ioProcessing->processImage = processImage;
					queue_push(procerArgs->LPB, ioProcessing);
					sem_post(procerArgs->LPB_empty);
					pthread_mutex_unlock(procerArgs->LPB_mutex);
					varValue = 1;
					writeData(data, varName, varValue);
				}
				else
				{
					pthread_mutex_unlock(procerArgs->LPB_mutex);
					printMsg = malloc(sizeof(t_msg));
					asprintf(&printMsg->msg,
							"ERROR I/O no-bloqueante, hilos no disponibles, línea %u\n",
							(*PC) + 1);
					varValue = 0;
					logWriteERROR(log, printMsg->msg);
					writeData(data, varName, varValue);
					printMsg->fd = processImage->clientfd;
					printMsg->type = 'N';
					pthread_mutex_lock(procerArgs->sendingList_mutex);
					list_add(procerArgs->sendingList, printMsg);
					pthread_mutex_unlock(procerArgs->sendingList_mutex);
					printMsg = NULL;
				}

			}
			else
			{
				pthread_mutex_lock(procerArgs->LPB_mutex);
				logWriteLSCH(log, processImage->pid, "Ejecución", "Bloqueado");
				ioProcessing = malloc(sizeof(t_ioProcessing));
				ioInfo = malloc(sizeof(t_ioInfo));
				ioInfo->flagNonBlocking = false;
				ioInfo->ioTime = ioTime;
				ioProcessing->ioInfo = ioInfo;
				ioProcessing->processImage = processImage;
				queue_push(procerArgs->LPB, ioProcessing);
				varValue = 1;
				writeData(data, varName, varValue);
				sem_post(procerArgs->LPB_empty);
				pthread_mutex_unlock(procerArgs->LPB_mutex);
				*ioBlockInterrupt = true;
			}
			ioProcessing = NULL;
			ioInfo = NULL;
			auxString = NULL;
			pthread_mutex_lock(procerArgs->SENT_TIME_mutex);
			sleep(*procerArgs->SENT_TIME);
			pthread_mutex_unlock(procerArgs->SENT_TIME_mutex);
			pthread_mutex_lock(procerArgs->ALG_mutex);
			if (!strcmp(*procerArgs->ALG, "RR"))
				(*quantumCount)++;
			pthread_mutex_unlock(procerArgs->ALG_mutex);
			(*rafaga)++;
			continue;
		}

		if (string_starts_with(code[*PC], "io("))
		{
			auxString = code[*PC] + strlen("io(");
			for (ind = 0; auxString[ind] != ','; ind++)
				;
			ioTimeString = realloc(ioTimeString, (ind + 1));
			memcpy(ioTimeString, auxString, ind * sizeof(char));
			ioTimeString[ind] = '\0';
			ioTime = atoi(ioTimeString);
			free(ioTimeString);
			ioTimeString = NULL;

			if (auxString[ind + 1] == '0')
			{
				pthread_mutex_lock(procerArgs->LPB_mutex);
				sem_getvalue(procerArgs->ioThreads, &semValue); //cantidad de ioT libres
				if (queue_size(procerArgs->LPB) < semValue)
				{ // si es igual o mayor, tardaria en atender
					ioProcessing = malloc(sizeof(t_ioProcessing));
					ioInfo = malloc(sizeof(t_ioInfo));
					ioInfo->flagNonBlocking = true;
					ioInfo->ioTime = ioTime;
					ioProcessing->ioInfo = ioInfo;
					ioProcessing->processImage = processImage;
					queue_push(procerArgs->LPB, ioProcessing);
					sem_post(procerArgs->LPB_empty);
					pthread_mutex_unlock(procerArgs->LPB_mutex);
				}
				else
				{
					pthread_mutex_unlock(procerArgs->LPB_mutex);
					printMsg = malloc(sizeof(t_msg));
					asprintf(&printMsg->msg,
							"ERROR I/O no-bloqueante, hilos no disponibles, línea %u\n",
							(*PC) + 1);
					logWriteERROR(log, printMsg->msg);
					printMsg->fd = processImage->clientfd;
					printMsg->type = 'N';
					pthread_mutex_lock(procerArgs->sendingList_mutex);
					list_add(procerArgs->sendingList, printMsg);
					pthread_mutex_unlock(procerArgs->sendingList_mutex);
					printMsg = NULL;
				}

			}
			else
			{
				pthread_mutex_lock(procerArgs->LPB_mutex);
				logWriteLSCH(log, processImage->pid, "Ejecución", "Bloqueado");
				ioProcessing = malloc(sizeof(t_ioProcessing));
				ioInfo = malloc(sizeof(t_ioInfo));
				ioInfo->flagNonBlocking = false;
				ioInfo->ioTime = ioTime;
				ioProcessing->ioInfo = ioInfo;
				ioProcessing->processImage = processImage;
				queue_push(procerArgs->LPB, ioProcessing);
				sem_post(procerArgs->LPB_empty);
				pthread_mutex_unlock(procerArgs->LPB_mutex);
				(*ioBlockInterrupt) = true;
			}
			ioProcessing = NULL;
			ioInfo = NULL;
			auxString = NULL;
			pthread_mutex_lock(procerArgs->SENT_TIME_mutex);
//			sleep(*procerArgs->SENT_TIME);
			pthread_mutex_unlock(procerArgs->SENT_TIME_mutex);
			pthread_mutex_lock(procerArgs->ALG_mutex);
//			if (!strcmp(*procerArgs->ALG, "RR"))
//				(*quantumCount)++;
			pthread_mutex_unlock(procerArgs->ALG_mutex);
//			(*rafaga)++;
			continue;
		}

		if (string_ends_with(code[*PC], "()"))
		{
			varValue = 0;
			for (ind = 0; code[*PC][ind] != '('; ind++)
				;
			auxString = malloc((ind + 1) * sizeof(char));
			memcpy(auxString, code[*PC], ind);
			auxString[ind] = '\0';

			newElement = malloc(sizeof(t_stack));
			newElement->linecall = (*PC);
			newElement->functionName = auxString;
			auxString = NULL;
			newElement->PC = findFunctionLine(code, newElement->functionName);
			list_add(*stack, newElement);
			stackExecution(code, processImage, procerArgs, ioBlockInterrupt,
					quantumCount, rafaga);
			if (!list_is_empty(*stack))
			{
				reorderStack(stack, element);
				PC = &((*element)->PC);
			}

			continue;
		}
	}

	if (string_starts_with(code[*PC], "fin_funcion")
			&& string_ends_with(code[*PC], (*element)->functionName))
	{
		freeStackElement(element);
	}
	else
		list_add(*stack, (*element));
	logDestroy(log);
	return;
}
void reorderStack(t_list** stack, t_stack** element)
{
	t_stack* newElement = stack_get(*stack);
	t_list* newStack = list_create();
	list_add(newStack, *element);
	*element = newElement;

	int size = list_size(*stack);
	for (size = list_size(*stack); size > 0; size--)
		list_add(newStack, list_remove(*stack, 0));
	list_destroy(*stack);
	*stack = newStack;
}

void freeStackElement(t_stack** element)
{
	free((*element)->functionName);
	free(*element);
	(*element) = NULL;
}

void stackExecution(char** code, t_ProcessImage* processImage,
		t_PROCERargs* procerArgs, bool* ioBlockInterrupt, int* quantumCount,
		double *rafaga)
{
	t_list** stack = &processImage->stack;
	t_stack* element;
	if (list_is_empty(*stack) != true)
	{
		element = stack_get(*stack);
		functionExecution(code, processImage, &element, procerArgs,
				ioBlockInterrupt, quantumCount, rafaga);
	}
	return;
}

uint16_t findFunctionLine(char** code, char* fname)
{
	char* line = malloc(
			(strlen("comienzo_funcion,") + strlen(fname) + 1) * sizeof(char));
	uint16_t offset = 0, tmpsize = 0;
	memcpy(line + offset, "comienzo_funcion ",
			tmpsize = strlen("comienzo_funcion,"));
	offset += tmpsize;
	memcpy(line + offset, fname, tmpsize = strlen(fname));
	offset += tmpsize;
	line[offset] = '\0';
	for (offset = 0; strcmp(code[offset], line) != 0; offset++)
		;
	free(line);
	return offset;
}

t_stack* stack_get(t_list* stack)
{
	return list_remove(stack, list_size(stack) - 1);
}

void writeData(t_data** data, char varName, int varValue)
{
	uint16_t i;
	for (i = 0; (*data)[i].varName != '\0' && (*data)[i].varName != varName;
			i++)
		;
	if ((*data)[i].varName != '\0')
		(*data)[i].varValue = varValue;
	return;
}

void changePC(uint16_t* PC, char** code, char* tagName)
{
	uint16_t i;
	for (i = 0; tagName[i] != ';' && tagName[i] != '\0'; i++)
		;
	char* realTagName = malloc((i + 1) * sizeof(char));

	memcpy(realTagName, tagName, i);
	realTagName[i] = '\0';

	char* tag = malloc((strlen(realTagName) + 2) * sizeof(char));
	memcpy(tag, realTagName, strlen(realTagName));
	tag[strlen(realTagName)] = ':';
	tag[strlen(realTagName) + 1] = '\0';
	for (i = 0; strcmp(tag, code[i]) != 0; i++)
		;
	free(tag);
	free(realTagName);
	(*PC) = i;
	return;
}

int readData(t_data** data, int varName)
{
	uint16_t i;
	for (i = 0; (*data)[i].varName != '\0' && (*data)[i].varName != varName;
			i++)
		;
	if ((*data)[i].varName == '\0')
	{
		char* varNameString = malloc(2 * sizeof(char));
		varNameString[0] = varName;
		varNameString[1] = '\0';
		int value = atoi(varNameString);
		free(varNameString);
		return value;
	}
	return (*data)[i].varValue;
}

uint16_t dataCount(t_data** data)
{
	if ((*data) == NULL )
		return 0;
	uint16_t i;
	for (i = 0; (*data)[i].varName != '\0'; i++)
		;
	return i;
}

void addVariables(t_data** data, char* varList)
{
	uint16_t varCount = dataCount(data);
	uint16_t i = 0;
	while (true)
	{
		varCount++;
		(*data) = realloc((*data), (varCount) * sizeof(t_data));
		(*data)[varCount - 1].varName = varList[i];
		(*data)[varCount - 1].varValue = 0;
		if (varList[i + 1] != '\0')
			i += 2;
		else
			break;
	}
	varCount++;
	(*data) = realloc((*data), (varCount) * sizeof(t_data));
	(*data)[varCount - 1].varName = '\0';
	return;
}

char** codeSegmentCreator(char* script)
{
	uint16_t lineLength, linesCount = 0;
	char* code = script;
	char** codeSegment = NULL;
	while (true)
	{
		linesCount++;
		for (lineLength = 0;
				code[lineLength] != '\n' && code[lineLength] != '\0'
						&& code[lineLength] != '#'; lineLength++)
			;
		codeSegment = realloc(codeSegment, linesCount * sizeof(char*));
		codeSegment[linesCount - 1] = malloc((lineLength + 1) * sizeof(char));
		memcpy(codeSegment[linesCount - 1], code, lineLength);
		codeSegment[linesCount - 1][lineLength] = '\0';
		string_trim(&codeSegment[linesCount - 1]);
		if (code[lineLength] == '\0')
			break;
		else
			code = code + strlenCS(code) + 1;
	}
	linesCount++;
	codeSegment = realloc(codeSegment, linesCount * sizeof(char*));
	codeSegment[linesCount - 1] = NULL;
	return codeSegment;
}

void codeSegmentDestructor(char*** code)
{
	uint16_t i;
	for (i = 0; code[0][i] != NULL ; i++)
		free(code[0][i]);
	free(*code);
	*code = NULL;
	return;
}

size_t strlenCS(char* line)
{
	size_t lineLength;
	for (lineLength = 0; line[lineLength] != '\0' && line[lineLength] != '\n';
			lineLength++)
		;
	return lineLength;
}

char* prepareMessage(t_ProcessImage* processImage)
{
	char* message = NULL;
	char* dataStructure = dataStructureMsg(processImage->data);
	char* stackStructure = stackStructureMsg(processImage->stack);

	asprintf(&message,
			"\n------------------------------------------\n\nID=%d\nPC=%d\n\n- Estructura de código ----\n%s\n-------------------------\n\n- Estructura de Datos ----\n%s\n-------------------------\n\n- Estructura de Stack ----\n%s\n------------------------------------------\n\n",
			processImage->pid, processImage->PC + 1, processImage->code,
			dataStructure, stackStructure);
	free(dataStructure);
	free(stackStructure);

	return message;
}

char* dataStructureMsg(t_data* data)
{
	uint16_t i, offset = 0, tmp_size = 0;
	char *msg = NULL, *aux = NULL;
	if (data != NULL )
	{
		for (i = 0; data[i].varName != '\0'; i++)
		{
			asprintf(&aux, "%c=%d\n", data[i].varName, data[i].varValue);
			tmp_size += strlen(aux) + 1;
			msg = realloc(msg, offset + tmp_size);
			if (offset == 0)
				msg[0] = '\0';
			strcat(msg, aux);
			offset += tmp_size - 1;
			free(aux);
			aux = NULL;
		}
	}
	else
	{
		msg = malloc(sizeof(char));
		*msg = '\0';
	}
	return msg;
}

char* stackStructureMsg(t_list* stack)
{
	uint16_t i, listSize = list_size(stack), offset = 0, tmp_size = 0;
	t_stack* element = NULL;
	char* msg = NULL, *aux = NULL;
	if (listSize > 0)
	{
		for (i = 0; i < listSize; i++)
		{
			element = list_get(stack, i);
			asprintf(&aux, "%u,%s\n", element->linecall + 1,
					element->functionName);
			tmp_size += strlen(aux) + 1;
			msg = realloc(msg, offset + tmp_size);
			memcpy(msg + offset, aux, tmp_size);
			offset += tmp_size - 1;
			free(aux);
			aux = NULL;
		}
	}
	else
	{
		msg = malloc(sizeof(char));
		*msg = '\0';
	}

	return msg;
}


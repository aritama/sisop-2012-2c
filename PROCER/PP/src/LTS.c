#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <arpa/inet.h>
#include "defSerialization.h"
#include "LTS.h"
#include <commons/collections/queue.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include "server_defs.h"
#include "logs_def.h"

//#define PORT 5000

void* LTSconex(t_LTSconexArgs* LTSconexArgs)
{
	t_logs* log = logCreate("LTS_COM", LTSconexArgs->log_mutex);
	int serverfd = createBindListen((uint16_t) *LTSconexArgs->PORT, NULL );
	if (serverfd == ERROR)
	{
		logWriteERROR(log, "Error creando descriptor de socket server");
		exit(EXIT_FAILURE);
	}
	int epfd = epoll_create1(EPOLL_CLOEXEC);
	int eventsCount, i, clientfd;
	if (epfd < 0)
	{
		logWriteERROR(log, "Error creando instancia epoll");
		perror("epoll_create error");
		exit(EXIT_FAILURE);
	}
	struct epoll_event events[MAX_EVENTS];
	struct epoll_event event;
	bzero(&event, sizeof(struct epoll_event));

	event.data.fd = serverfd;
	event.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
	epoll_ctl(epfd, EPOLL_CTL_ADD, serverfd, &event);
	t_data_stream *dataIn = NULL;
	t_data_stream *dataOut = NULL;
	t_ProcessImage *processImage = NULL;
	int pid = 11;
	t_msg *msg = NULL;
	int32_t priority;
	uint16_t len = 0;
	int listSize;

	logWriteINFO(log, "Esperando AnSISOP scripts");

	while (1)
	{
		eventsCount = epoll_wait(epfd, events, MAX_EVENTS, -1);
		if (eventsCount < 0)
		{
			perror("epoll_wait");
		}

		for (i = 0; i < eventsCount; i++)
		{
			clientfd = events[i].data.fd;

			if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)
					|| (events[i].events & EPOLLRDHUP))
			{ //eventos error o se cerró conexion
				close(clientfd);
				continue;
			}

			if (events[i].events & EPOLLIN)
			{ // ve si e EPOLLIN
				if (clientfd == serverfd) // CONEXION INICIAL,accept,hand,recv ansisop
				{
					clientfd = accept_connection(serverfd); // accept y hand
					if (makeHandshake(clientfd) != ERROR)
					{
						event.data.fd = clientfd;
						event.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
						epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &event);
					}
					else
					{
						close(clientfd);
						continue;
					}

					// Despues del handshake, me va a mandar el script,
					if (readStreamPri(clientfd, &dataIn, &priority) == ERROR)
					{ // si recibio mal o no recibe mas, cierro
						close(clientfd);
						continue;
					}
					else
					{ // si salió bien, miro de que tipo es el mensaje
						if ((dataIn->type) == 'N')
						{ // me envia script para ejecutar
						  // si supera MPS aviso y cierro conexion;
							if (sem_trywait(LTSconexArgs->MPS_sem) == ERROR)//FIXME
							{
								free(dataIn->data);
								dataIn->data = NULL;
								dataIn->data = malloc(
										strlen("MPS superado") + 1);
								strcpy(dataIn->data, "MPS superado");
								dataIn->type = 'F';				//reFused//XXX
								writeStream(clientfd, dataIn);
								free(dataIn->data);
								dataIn->data = NULL;
								free(dataIn);
								dataIn = NULL;
								close(clientfd);
								continue;
							}
							else
							{ // si supera MMP, enviar mensaje de ejecución demorada
								processImage = malloc(sizeof(t_ProcessImage));
								processImage->PC = 0;
								processImage->clientfd = clientfd;
								len = strlen(dataIn->data) + 1;
								processImage->code = malloc(len * sizeof(char));
								memmove(processImage->code, dataIn->data, len);
								processImage->data = NULL;
								processImage->pid = pid++;
								processImage->priority = priority;
								processImage->Tspn =
										(double) *LTSconexArgs->TANT;
								processImage->stack = list_create();
								free(dataIn->data);
								dataIn->data = NULL;
								free(dataIn);
								dataIn = NULL;
								if (sem_trywait(LTSconexArgs->MMP_sem) == ERROR) //MMP
								{
									logWriteLSCH(log, processImage->pid,
											"Ingresa al PP, demorado", "FIFO");
									dataOut = malloc(sizeof(t_data_stream));
									dataOut->data =
											malloc(
													strlen(
															"MMP superado, ejecución demorada")
															+ 1);
									strcpy(dataOut->data,
											"MMP superado, ejecución demorada");
									dataOut->type = 'N';
									writeStream(clientfd, dataOut);
									free(dataOut->data);
									dataOut->data = NULL;
									free(dataOut);
									dataOut = NULL;
									// pone en FIFO, poner en fifo solo consumió MPS
									pthread_mutex_lock(
											LTSconexArgs->LFIFO_mutex);
									queue_push(LTSconexArgs->LFIFO,
											processImage);
									sem_post(LTSconexArgs->LFIFO_empty);
									pthread_mutex_unlock(
											LTSconexArgs->LFIFO_mutex);
									// pone EN FIFO
								}
								else //MPS y MMP eran validos
								{
									logWriteLSCH(log, processImage->pid,
											"Ingresa al PP", "Nuevos");
									dataOut = malloc(sizeof(t_data_stream));
									dataOut->data = malloc(
											strlen("Script AnSISOP cargado")
													+ 1);
									strcpy(dataOut->data,
											"Script AnSISOP cargado");
									dataOut->type = 0;
									writeStream(clientfd, dataOut);
									free(dataOut->data);
									dataOut->data = NULL;
									free(dataOut);
									dataOut = NULL;

									// pone en nuevos
									pthread_mutex_lock(LTSconexArgs->LPN_mutex);
									queue_push(LTSconexArgs->LPN, processImage);
									sem_post(LTSconexArgs->LPN_empty);
									sem_post(LTSconexArgs->STS_in_empty);
									pthread_mutex_unlock(
											LTSconexArgs->LPN_mutex);
								}
							}
						}
					}
				}
				else // recibo respuestas, reanudar, la unica creo, no haria falta ni tipo
				{ 	// si MMP es superado, la solicitud deberá esperar, continue
					if (sem_trywait(LTSconexArgs->MMP_sem) != ERROR) // si es mayor a cero atiendo la solicitud, sino la postergo
					{
						if (readStream(clientfd, &dataIn) != ERROR)
						{
							if (dataIn->type == 'R') // quiere Reanudar dice el cliente, se pone la gorra
							{
								free(dataIn->data);
								free(dataIn);
								pthread_mutex_lock(LTSconexArgs->LPS_mutex);
								processImage = restoreSuspended(
										LTSconexArgs->LPS, clientfd);
								pthread_mutex_unlock(LTSconexArgs->LPS_mutex);
								if (processImage != NULL )
								{
									logWriteLSCH(log, processImage->pid,
											"Suspendidos", "Reanudados");
									pthread_mutex_lock(LTSconexArgs->LPR_mutex);
									queue_push(LTSconexArgs->LPR, processImage);
									sem_post(LTSconexArgs->LPR_empty);
									sem_post(LTSconexArgs->STS_in_empty);
									pthread_mutex_unlock(
											LTSconexArgs->LPR_mutex);
								}
								else
								{
									logWriteERROR(log,
											"Error restaurando un proceso");
									perror("error restoring process");
									//no va a pasar nunca se supone, pero por si las dudas
									sem_post(LTSconexArgs->MMP_sem);
									close(clientfd);
								}
							}
						}
						else // si dio error para recibir,, libero un recurso de MMP_sem y cierro
						{
							sem_post(LTSconexArgs->MMP_sem);
							close(clientfd);
						}
					}
				}
			}
			else
			{ // EPOLLOUT //  mando solo lo de la lista
				pthread_mutex_lock(LTSconexArgs->sendingList_mutex);
				listSize = list_size(LTSconexArgs->sendingList);
				pthread_mutex_unlock(LTSconexArgs->sendingList_mutex);
				if (listSize > 0)
				{
					pthread_mutex_lock(LTSconexArgs->sendingList_mutex);
					msg = list_fd_msg_get(LTSconexArgs->sendingList, clientfd);
					pthread_mutex_unlock(LTSconexArgs->sendingList_mutex);
					if (msg != NULL )
					{
						dataOut = malloc(sizeof(t_data_stream));
						dataOut->type = msg->type;
						dataOut->data = msg->msg;
						if (writeStream(clientfd, dataOut) == ERROR)
						{
							close(clientfd);
						}
						free(dataOut->data);
						free(dataOut);
						free(msg);
						msg = NULL;
					}
				}

			}
		}
	}

	logDestroy(log);
	close(epfd);
	close(serverfd);
	return NULL ;
}

t_ProcessImage* restoreSuspended(t_list* LPS, int clientfd)
{
	int listSize = list_size(LPS), i;
	t_ProcessImage* processImage;
	for (i = 0; i < listSize; i++)
	{
		processImage = list_get(LPS, i);
		if (processImage->clientfd == clientfd)
			break;
	}
	if (processImage->clientfd == clientfd)
	{
		processImage = list_remove(LPS, i);
		return processImage;
	}
	return NULL ;
}

t_msg* list_fd_msg_get(t_list* lista, int clientfd)
{
	int i, listSize = list_size(lista);
	t_msg* msg = NULL;
	for (i = 0; i < listSize; i++)
	{
		msg = list_get(lista, i);
		if (msg->fd == clientfd)
			break;
	}
	if (msg->fd == clientfd)
	{
		return list_remove(lista, i);
	}
	return NULL ;
}

void* LTSnew(t_LTSnewArgs* LTSnewArgs)
{
	t_logs* log = logCreate("LTS_NEW", LTSnewArgs->log_mutex);
	t_ProcessImage* processImage = NULL;
	while (true)
	{
		sem_wait(LTSnewArgs->LFIFO_empty);
		sem_wait(LTSnewArgs->MMP_sem);
		pthread_mutex_lock(LTSnewArgs->LFIFO_mutex);
		processImage = queue_pop(LTSnewArgs->LFIFO);
		pthread_mutex_unlock(LTSnewArgs->LFIFO_mutex);
		logWriteLSCH(log, processImage->pid, "FIFO", "Nuevos");
		pthread_mutex_lock(LTSnewArgs->LPN_mutex);
		queue_push(LTSnewArgs->LPN, processImage);
		sem_post(LTSnewArgs->LPN_empty);
		sem_post(LTSnewArgs->STS_in_empty);
		pthread_mutex_unlock(LTSnewArgs->LPN_mutex);
	}
	logDestroy(log);
	return NULL ;
}


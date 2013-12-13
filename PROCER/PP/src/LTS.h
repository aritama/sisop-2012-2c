#ifndef LTS_H_
#define LTS_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/queue.h>
#include <semaphore.h>
#include <pthread.h>
#include "procer_defs.h"

/* Headers
 * 0, envia script		; recibe LTS
 * 1, aviso reanudar	; recibe LTS
 *
 * 1, termina, informa al PI resultado, cierra			; AL PI
 * 2, aviso de suspendido								; AL PI
 */

typedef struct
{
	int fd;
	char type;
	char* msg;
} t_msg;

typedef struct
{
	t_list* sendingList, *LPS;
	t_queue *LFIFO, *LPR, *LPN;
	sem_t *MPS_sem, *MMP_sem, *LFIFO_empty, *LPR_empty, *LPN_empty,
			*STS_in_empty;
	pthread_mutex_t *sendingList_mutex, *LPS_mutex, *LFIFO_mutex, *LPR_mutex,
			*LPN_mutex;
	int *PORT;
	int *TANT; // spn
	pthread_mutex_t *log_mutex;
} t_LTSconexArgs;
void* LTSconex(t_LTSconexArgs*);

typedef struct
{
	t_queue *LFIFO, *LPN;
	sem_t *MMP_sem, *LFIFO_empty, *LPN_empty, *STS_in_empty;
	pthread_mutex_t *LFIFO_mutex, *LPN_mutex, *log_mutex;
} t_LTSnewArgs;
void* LTSnew(t_LTSnewArgs*);

/*typedef struct
 {
 int clientfd;
 int pid;
 int priority;
 uint16_t PC;
 char* code;
 t_data* data;
 t_list* stack;
 } t_ProcessImage;*/

t_msg* list_fd_msg_get(t_list* lista, int clientfd);
int setNonBlocking(int sockfd);
t_ProcessImage* restoreSuspended(t_list* LPS, int clientfd);

#endif /* LTS_H_ */

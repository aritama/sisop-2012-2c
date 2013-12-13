#ifndef PROCER_DEFS_H_
#define PROCER_DEFS_H_

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <commons/collections/list.h>
#include <stdint.h>
#include <stdbool.h>
#include <commons/string.h>
#include <string.h>
#include <commons/collections/queue.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct
{
	char varName;
	int varValue;
} t_data;

typedef struct
{
	char* functionName;
	uint16_t PC;
	uint16_t linecall;
} t_stack;

typedef struct
{
	int clientfd;
	int pid;
	int32_t priority;
	uint16_t PC;
	char* code;
	t_data* data;
	t_list* stack;
	double Tspn;
	double* ALFA;
} t_ProcessImage;

size_t strlenCS(char* line);
void codeSegmentDestructor(char*** code);
char** codeSegmentCreator(char* script);

typedef struct
{
	t_list* sendingList, *LPS;
	t_queue *LPL, *LPB, *LFQ;
	sem_t *LPL_empty, *LPB_empty, *LFQ_empty;
	sem_t *MMP_sem, *MPS_sem;
	sem_t *ioThreads, *STS_in_empty;
	bool *SUSPEND;
	pthread_mutex_t *sendingList_mutex, *LPS_mutex, *LPB_mutex, *LPL_mutex,
			*LFQ_mutex;
	int *quantum, *SENT_TIME;
	char** ALG;
	pthread_mutex_t* SENT_TIME_mutex, *ALG_mutex, *log_mutex;
	double *ALFA;
} t_PROCERargs;

void processExecution(char** code, t_ProcessImage* processImage,
		t_PROCERargs* procerArgs, bool*, int*, double*);
uint16_t dataCount(t_data** data);
void addVariables(t_data** data, char* varList);
int readData(t_data** data, int varName);
void changePC(uint16_t* PC, char** code, char* tagName);
void writeData(t_data** data, char varName, int varValue);
uint16_t findFunctionLine(char** code, char* fname);
t_stack* stack_get(t_list* stack);

void stackExecution(char** code, t_ProcessImage*, t_PROCERargs*, bool*, int*,
		double*);
void functionExecution(char** code, t_ProcessImage* processImage,
		t_stack** element, t_PROCERargs*, bool*, int*, double*);
void freeStackElement(t_stack** element);
void reorderStack(t_list** stack, t_stack** element);

void* PROCER(t_PROCERargs* procerArgs);

char* prepareMessage(t_ProcessImage* processImage);
char* stackStructureMsg(t_list* stack);
char* dataStructureMsg(t_data* data);

#endif /* PROCER_DEFS_H_ */

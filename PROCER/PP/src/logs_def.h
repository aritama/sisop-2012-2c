#ifndef LOGS_DEF_H_
#define LOGS_DEF_H_

#define _GNU_SOURCE
#include <stdio.h>
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

typedef struct
{
	FILE* logFile;
	char* programName;
	char* threadName;
	pid_t threadPid;
	pid_t programPid;
	pthread_mutex_t* log_mutex;
} t_logs;

char* getDateString(void);
int logWriteLSCH(t_logs* log, int scriptPID, char* origen, char* destino);
int logWriteINFO(t_logs* log, char* message);
int logWriteERROR(t_logs* log, char* message);
t_logs* logCreate(char* threadName, pthread_mutex_t*);
void logDestroy(t_logs* log);

#endif /* LOGS_DEF_H_ */

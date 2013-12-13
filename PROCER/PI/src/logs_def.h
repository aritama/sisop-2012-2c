#ifndef LOGS_DEF_H_
#define LOGS_DEF_H_

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

typedef struct
{
	FILE* logFile;
	char* programName;
	pid_t programPid;
} t_logs;

char* getDateString(void);
int logWriteINFO(t_logs* log, char* message);
int logWriteERROR(t_logs* log, char* message);
t_logs* logCreate();
void logDestroy(t_logs* log);

#endif /* LOGS_DEF_H_ */

#include "logs_def.h"

t_logs* logCreate(char* threadName, pthread_mutex_t* log_mutex)
{
	t_logs* log = malloc(sizeof(t_logs));
	log->log_mutex = log_mutex;

	if (threadName != NULL )
	{
		asprintf(&log->threadName, "%s", threadName);
		log->threadPid = pthread_self();
	}
	else
	{
		log->threadName = NULL;
	}
	asprintf(&log->programName, "%s", "PP");
	log->programPid = getpid();

	char* filePath = NULL;
	asprintf(&filePath, "%s/PP.[%u].log", getenv("PWD"), log->programPid);
	log->logFile = fopen(filePath, "a+");
	free(filePath);

	if (log->logFile == NULL )
	{
		perror("Cannot create/open log file");
		return NULL ;
	}

	return log;
}

char* getDateString(void)
{
	struct tm *OurT = NULL;
	time_t Tval = 0;
	Tval = time(NULL );
	OurT = localtime(&Tval);

	char* messageExt = NULL;
	asprintf(&messageExt, "%d/%d/%d %d:%d:%d", 1900 + OurT->tm_year,
			1 + OurT->tm_mon, OurT->tm_mday, OurT->tm_hour, OurT->tm_min,
			OurT->tm_sec);
	return messageExt;
}

int logWriteLSCH(t_logs* log, int scriptPID, char* origen, char* destino)
{
	char* msgLog = NULL;
	char* date = getDateString();
	char* message = NULL;
	asprintf(&message, "PID:%d; Origen:%s; Destino:%s", scriptPID, origen,
			destino);
	if (log->threadName != NULL )
		asprintf(&msgLog, "[%s] - [%s/%u] [%s/%u] [LSCH]:\n    %s\n\n", date,
				log->programName, log->programPid, log->threadName,
				log->threadPid, message);
	else
		asprintf(&msgLog, "[%s] - [%s/%u] [LSCH]:\n    %s\n\n", date,
				log->programName, log->programPid, message);
	pthread_mutex_lock(log->log_mutex);
	int result = fwrite(msgLog, sizeof(char), strlen(msgLog), log->logFile);
	fflush(log->logFile);
	pthread_mutex_unlock(log->log_mutex);
	free(message);
	free(date);
	free(msgLog);
	return result;
}

int logWriteINFO(t_logs* log, char* message)
{
	char* msgLog = NULL;
	char* date = getDateString();
	if (log->threadName != NULL )
		asprintf(&msgLog, "[%s] - [%s/%u] [%s/%u] [INFO]:\n    %s\n\n", date,
				log->programName, log->programPid, log->threadName,
				log->threadPid, message);
	else
		asprintf(&msgLog, "[%s] - [%s/%u] [INFO]:\n    %s\n\n", date,
				log->programName, log->programPid, message);
	pthread_mutex_lock(log->log_mutex);
	int result = fwrite(msgLog, sizeof(char), strlen(msgLog), log->logFile);
	fflush(log->logFile);
	pthread_mutex_unlock(log->log_mutex);

	free(date);
	free(msgLog);
	return result;
}

int logWriteERROR(t_logs* log, char* message)
{
	char* msgLog = NULL;
	char* date = getDateString();
	if (log->threadName != NULL )
		asprintf(&msgLog, "[%s] - [%s/%u] [%s/%u] [ERROR]:\n    %s\n\n", date,
				log->programName, log->programPid, log->threadName,
				log->threadPid, message);
	else
		asprintf(&msgLog, "[%s] - [%s/%u] [ERROR]:\n    %s\n\n", date,
				log->programName, log->programPid, message);
	pthread_mutex_lock(log->log_mutex);
	int result = fwrite(msgLog, sizeof(char), strlen(msgLog), log->logFile);
	fflush(log->logFile);
	pthread_mutex_unlock(log->log_mutex);

	free(date);
	free(msgLog);
	return result;
}

void logDestroy(t_logs* log)
{
	free(log->programName);
	free(log->threadName);
	fclose(log->logFile);
	free(log);
}

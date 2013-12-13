#include "logs_def.h"

t_logs* logCreate()
{
	t_logs* log = malloc(sizeof(t_logs));
	asprintf(&log->programName, "%s", "PI");
	log->programPid = getpid();
	char* filePath = NULL;
	asprintf(&filePath, "%s/%s.[%u].log", getenv("PWD"), log->programName,
			log->programPid);

	log->logFile = fopen(filePath, "a");
	free(filePath);

	if (log->logFile == NULL )
	{
		perror("Cannot create/open log file");
		exit(EXIT_FAILURE);
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

int logWriteINFO(t_logs* log, char* message)
{
	char* msgLog = NULL;
	char* date = getDateString();
	asprintf(&msgLog, "[%s] - [%s/%u] [INFO]:\n    %s\n\n", date,
			log->programName, log->programPid, message);
	int result = fwrite(msgLog, sizeof(char), strlen(msgLog), log->logFile);
	fflush(log->logFile);

	free(date);
	free(msgLog);
	return result;
}

int logWriteERROR(t_logs* log, char* message)
{
	char* msgLog = NULL;
	char* date = getDateString();
	asprintf(&msgLog, "[%s] - [%s/%u] [ERROR]:\n    %s\n\n", date,
			log->programName, log->programPid, message);
	int result = fwrite(msgLog, sizeof(char), strlen(msgLog), log->logFile);
	fflush(log->logFile);

	free(date);
	free(msgLog);
	return result;
}

void logDestroy(t_logs* log)
{
	free(log->programName);
	fclose(log->logFile);
	free(log);
}

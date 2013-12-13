#define _GNU_SOURCE
#include "PI_defs.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "defSerialization.h"
#include "client_defs.h"
#include <commons/config.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include "logs_def.h"
#include <stdio.h>

typedef struct
{
	bool* finished;
	pid_t threadID;
} t_sigHandStruct;

void *signal_handler(t_sigHandStruct*);

int main(int argc, char* argv[], char* env[])
{
	t_logs* log = logCreate();
	logWriteINFO(log, "Proceso Interprete iniciado");
	t_sigHandStruct sigHandStruct;
	sigset_t SigSet;
	sigfillset(&SigSet);
	bool finished = false;
	sigHandStruct.finished = &finished;
	pthread_t SigHandler_thread;
	pthread_sigmask(SIG_SETMASK, &SigSet, NULL );
	pthread_create(&SigHandler_thread, NULL, (void*) signal_handler,
			(void*) &sigHandStruct);

	t_config* config = config_create("./pi.config");

	char* path = obtenerPathDelScript(argv, env);
	int32_t priority = (int32_t) config_get_int_value(config, "prioridad_PRI");
	if (argc == 3)
		priority = atoi(argv[2]);
	if (argc > 3)
	{
		logWriteERROR(log, "Mal ingreso de parametros al ejecutar el script");
		logDestroy(log);
		exit(EXIT_FAILURE);
	}

	int fd = open(path, O_RDONLY);
	char aux;
	free(path);
	struct stat fileInfo;
	fstat(fd, &fileInfo);
	char* mapedScript = (char*) mmap(NULL, fileInfo.st_size, PROT_READ,
			MAP_PRIVATE, fd, 0);
	close(fd);
	if (*mapedScript == -1)
	{
		logWriteERROR(log, "Error al mapear el script a memoria");
		exit(EXIT_FAILURE);
	}

	char* IP = config_get_string_value(config, "PP_IP");
	uint16_t PORT = (uint16_t) config_get_int_value(config, "PP_Puerto");
	char* logMsg = NULL;
	asprintf(&logMsg, "Configuración Leída: PP_IP:%s PP_Puerto:%u Prioridad:%d",
			IP, PORT, priority);
	logWriteINFO(log, logMsg);
	free(logMsg);

	int sockfd = create_and_connect(PORT, IP);
	if (sockfd == ERROR)
	{
		logWriteERROR(log,
				"Error creando descriptor de socket o conectado al servidor");
		exit(EXIT_FAILURE);
	}

	if (makeHandshake(sockfd) == ERROR)
	{
		logWriteERROR(log, "Error en handshake");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	logWriteINFO(log, "Handshake OK, conectado con el servidor");

	t_data_stream datos;
	datos.type = 'N';
	datos.data = mapedScript;

	if (writeStreamPri(sockfd, &datos, &priority) == ERROR)
	{
		//envia script
		logWriteERROR(log, "Error enviando datos y script al servidor");
		exit(EXIT_FAILURE);
	}

	logWriteINFO(log, "Script enviado al servidor");
	munmap(mapedScript, fileInfo.st_size);

	//toDO, a espera de respuestas o etc

	t_data_stream *data;
	if (readStream(sockfd, &data) == ERROR)
	{ // script recibido, o otra cosa
		logWriteERROR(log,
				"Servidor no ha enviado respuesta luego del envio del script");
		exit(EXIT_FAILURE);
	}
	logWriteINFO(log, data->data);
	aux = data->type;
	puts(data->data);
	free(data->data);
	free(data);
	if (aux == 'F')
	{
		close(sockfd);
		logDestroy(log);
		config_destroy(config);
		finished = true;
		kill(sigHandStruct.threadID, SIGINT);
		pthread_join(SigHandler_thread, NULL );
		exit(EXIT_SUCCESS);
	}

	// fixme falta lo de MMP,MPS, script aceptado

	while (true)
	{ // suspendido, info de finalizacion, info de "imprimir"
		if (readStream(sockfd, &data) == ERROR)
		{
			logWriteERROR(log,
					"Error recibiendo datos, o el servidor nos ha desconectado");
			close(sockfd);
			break;
		}

		if (data->type == 'N')
		{
			logWriteINFO(log, data->data);
			puts(data->data);
			free(data->data);
			free(data);
			continue;
		}
		if (data->type == 'S')
		{
			logWriteINFO(log, "Proceso Suspendido");
			logWriteINFO(log, data->data);
			puts(data->data);
			free(data->data);
			free(data);
			puts(
					"Proceso Suspendido, presione *enter para reanudarlo."
							"Deberá esperar a que disminuya MMP en caso de estar al máximo");
			scanf("%c", &aux);
			data = malloc(sizeof(t_data_stream));
			data->data = malloc(strlen("Reanudar") + 1);
			strcpy(data->data, "Reanudar");
			data->type = 'R';
			if (writeStream(sockfd, data) == ERROR)
			{
				logWriteERROR(log, "No se ha podido reanudar el proceso");
				exit(EXIT_FAILURE);
			}
			logWriteINFO(log, "Proceso reanudado");
			free(data->data);
			free(data);
			puts("Reanudado");
			continue;
		}
		if (data->type == 'T')
		{
			logWriteINFO(log, data->data);
			puts(data->data);
			free(data->data);
			free(data);
			puts("Finalizó ejecución");
			logWriteINFO(log, "Ha finalizado la ejecución");
			break;
		}
	}
	logDestroy(log);
	close(sockfd);
	config_destroy(config);
	finished = true;
	kill(sigHandStruct.threadID, SIGINT);
	pthread_join(SigHandler_thread, NULL );

	return EXIT_SUCCESS;
}

void *signal_handler(t_sigHandStruct *sigHandStruct)
{
	sigHandStruct->threadID = getpid();
	sigset_t SigSet;
	int senial;

	sigfillset(&SigSet);

	while (1)
	{
		sigwait(&SigSet, &senial);
		switch (senial)
		{
		case SIGINT:
		{
			if (*sigHandStruct->finished)
			{
				return NULL ;
			}
			else
			{
				puts("    Debe esperar a que finalice la ejecución del script");

			}
			break;
		}
		default:
//	Ignorar las demas menos SIGUSR1
		{
			puts("    Debe esperar a que finalice la ejecución del script");
			break;
		}
		} //fin switch
	}
	return NULL ;
}


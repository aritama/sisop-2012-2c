#define _GNU_SOURCE
#include "LTS.h"
#include "procer_defs.h"
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include "io.h"
#include "configure.h"
#include "STS.h"

typedef struct
{
	bool* SUSPEND;
	pid_t sigHandPid;
	int MPS_max_val;
	sem_t *MPS;
} t_sigHandStruct;
void *signal_handler(t_sigHandStruct*);

int main()
{
	uint32_t i;

	t_sigHandStruct sigHandStruct;
	sigset_t SigSet;
	sigfillset(&SigSet);
	bool SUSPEND = false;
	sigHandStruct.SUSPEND = &SUSPEND;
	pthread_t SigHandler_thread;
	pthread_sigmask(SIG_SETMASK, &SigSet, NULL );

	t_list *sendingList = list_create(), *LPS = list_create();
	t_queue *LPN = queue_create(), *LFIFO = queue_create(), *LPB =
			queue_create(), *LFIO = queue_create(), *LPR = queue_create(),
			*LFQ = queue_create(), *LPL = queue_create();
	sem_t LPN_empty, LFIFO_empty, MMP_sem, MPS_sem, ioThreadsCount, LPB_empty,
			LFIO_empty, LPR_empty, LFQ_empty, LPL_empty, STS_in_empty;
	pthread_t LTSconex_thread, PROCER_thread, LTSnew_thread, *ioThreadsPool;
	pthread_mutex_t sendingList_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t LPS_mutex = PTHREAD_MUTEX_INITIALIZER,
	LPB_mutex = PTHREAD_MUTEX_INITIALIZER, LFIO_mutex =
			PTHREAD_MUTEX_INITIALIZER;
			pthread_mutex_t LPN_mutex=PTHREAD_MUTEX_INITIALIZER,
	LPR_mutex = PTHREAD_MUTEX_INITIALIZER, LFIFO_mutex =
			PTHREAD_MUTEX_INITIALIZER, LPL_mutex = PTHREAD_MUTEX_INITIALIZER,
			LFQ_mutex = PTHREAD_MUTEX_INITIALIZER;

			//XXX, configs
					int MPS,
	MMP, quantum, PORT, IO_TIME, IO_THREADS;
	int TANT; // XXX, este es el de para SPN, el tiempo ese anterior estimado
	double ALFA;
	char** ALG = malloc(sizeof(char*));
	int SENT_TIME;
	bool readyFlag = false;
	pthread_t configTh;

	pthread_mutex_t ALG_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t QueuePriorities_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t SENT_TIME_mutex = PTHREAD_MUTEX_INITIALIZER;

	t_queuesArray queuesArray[4];
	queuesArray[0].name = NULL;
	queuesArray[0].priority = 0;
	queuesArray[0].queue = LPN;
	asprintf(&queuesArray[0].name, "%s", "PLPN"); // XXX, free names;
	queuesArray[1].name = NULL;
	queuesArray[1].priority = 0;
	queuesArray[1].queue = LPR;
	asprintf(&queuesArray[1].name, "%s", "PLPR");
	queuesArray[2].name = NULL;
	queuesArray[2].priority = 0;
	queuesArray[2].queue = LFQ;
	asprintf(&queuesArray[2].name, "%s", "PLFQ");
	queuesArray[3].name = NULL;
	queuesArray[3].priority = 0;
	queuesArray[3].queue = LFIO;
	asprintf(&queuesArray[3].name, "%s", "PLFIO");

	pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER; //XXX

	//XXX STS
	t_STSargs STSargs;
	STSargs.LFIO = LFIO;
	STSargs.LFIO_empty = &LFIO_empty;
	STSargs.LFIO_mutex = &LFIO_mutex;
	STSargs.LFQ = LFQ;
	STSargs.LFQ_empty = &LFQ_empty;
	STSargs.LFQ_mutex = &LFQ_mutex;
	STSargs.LPL = LPL;
	STSargs.LPL_empty = &LPL_empty;
	STSargs.LPL_mutex = &LPL_mutex;
	STSargs.LPN = LPN;
	STSargs.LPN_empty = &LPN_empty;
	STSargs.LPN_mutex = &LPN_mutex;
	STSargs.LPR = LPR;
	STSargs.LPR_empty = &LPR_empty;
	STSargs.LPR_mutex = &LPR_mutex;
	STSargs.ALG = ALG;
	STSargs.STS_in_empty = &STS_in_empty;
	STSargs.queuesArray = queuesArray;
	STSargs.ALG_mutex = &ALG_mutex;
	STSargs.log_mutex = &log_mutex;

	t_configure configurations;
	configurations.fakeSTSargs = &STSargs;
	configurations.ALG = ALG;
	configurations.ALG_mutex = &ALG_mutex;
	configurations.IO_THREADS = &IO_THREADS;
	configurations.IO_TIME = &IO_TIME;
	configurations.MMP = &MMP;
	configurations.MPS = &MPS;
	configurations.PORT = &PORT;
	configurations.quantum = &quantum;
	configurations.QueuePriorities_mutex = &QueuePriorities_mutex;
	configurations.SENT_TIME = &SENT_TIME;
	configurations.flagReady = &readyFlag;
	configurations.queuesArray = queuesArray;
	configurations.SENT_TIME_mutex = &SENT_TIME_mutex;
	configurations.TANT = &TANT;
	configurations.ALFA = &ALFA;

	pthread_create(&configTh, NULL, (void*) configure, (void*) &configurations);

	while (!readyFlag)
		;
	sigHandStruct.MPS = &MPS_sem;
	sigHandStruct.MPS_max_val = MPS;

	pthread_create(&SigHandler_thread, NULL, (void*) signal_handler,
			(void*) &sigHandStruct);

// xxx, end configs
	sem_init(&LPN_empty, 0, 0);
	sem_init(&LFIFO_empty, 0, 0);
	sem_init(&MMP_sem, 0, MMP);
	sem_init(&MPS_sem, 0, MPS);
	sem_init(&ioThreadsCount, 0, IO_THREADS);
	sem_init(&LPB_empty, 0, 0);
	sem_init(&LFIO_empty, 0, 0);
	sem_init(&LPR_empty, 0, 0);
	sem_init(&LFQ_empty, 0, 0);
	sem_init(&LPL_empty, 0, 0);
	sem_init(&STS_in_empty, 0, 0);

	t_LTSconexArgs LTSconexArgs;
	LTSconexArgs.MPS_sem = &MPS_sem;
	LTSconexArgs.MMP_sem = &MMP_sem;
	LTSconexArgs.LFIFO = LFIFO;
	LTSconexArgs.LFIFO_empty = &LFIFO_empty;
	LTSconexArgs.LFIFO_mutex = &LFIFO_mutex;
	LTSconexArgs.sendingList = sendingList;
	LTSconexArgs.sendingList_mutex = &sendingList_mutex;
	LTSconexArgs.LPS = LPS;
	LTSconexArgs.LPS_mutex = &LPS_mutex;
	LTSconexArgs.LPR = LPR;
	LTSconexArgs.LPR_empty = &LPR_empty;
	LTSconexArgs.LPR_mutex = &LPR_mutex;
	LTSconexArgs.PORT = &PORT;
	LTSconexArgs.TANT = &TANT;
	LTSconexArgs.LPN = LPN;
	LTSconexArgs.LPN_empty = &LPN_empty;
	LTSconexArgs.LPN_mutex = &LPN_mutex;
	LTSconexArgs.STS_in_empty = &STS_in_empty;
	LTSconexArgs.log_mutex = &log_mutex;

	t_LTSnewArgs LTSnewArgs;
	LTSnewArgs.LFIFO = LFIFO;
	LTSnewArgs.LFIFO_empty = &LFIFO_empty;
	LTSnewArgs.LFIFO_mutex = &LFIFO_mutex;
	LTSnewArgs.LPN = LPN;
	LTSnewArgs.LPN_empty = &LPN_empty;
	LTSnewArgs.LPN_mutex = &LPN_mutex;
	LTSnewArgs.MMP_sem = &MMP_sem;
	LTSnewArgs.STS_in_empty = &STS_in_empty;
	LTSnewArgs.log_mutex = &log_mutex;

	t_PROCERargs PROCERargs;
	PROCERargs.LPL = LPL;
	PROCERargs.LPL_empty = &LPL_empty;
	PROCERargs.LPL_mutex = &LPL_mutex;
	PROCERargs.MMP_sem = &MMP_sem;
	PROCERargs.MPS_sem = &MPS_sem;
	PROCERargs.sendingList = sendingList;
	PROCERargs.sendingList_mutex = &sendingList_mutex;
	PROCERargs.SUSPEND = &SUSPEND;
	PROCERargs.LPS = LPS;
	PROCERargs.LPS_mutex = &LPS_mutex;
	PROCERargs.LPS_mutex = &LPS_mutex;
	PROCERargs.LPB = LPB;
	PROCERargs.LPB_empty = &LPB_empty;
	PROCERargs.LPB_mutex = &LPB_mutex;
	PROCERargs.ioThreads = &ioThreadsCount;
	PROCERargs.LFQ = LFQ;
	PROCERargs.LFQ_empty = &LFQ_empty;
	PROCERargs.LFQ_mutex = &LFQ_mutex;
	PROCERargs.quantum = &quantum;
	PROCERargs.ALG = ALG;
	PROCERargs.ALG_mutex = &ALG_mutex;
	PROCERargs.SENT_TIME = &SENT_TIME;
	PROCERargs.SENT_TIME_mutex = &SENT_TIME_mutex;
	PROCERargs.ALFA = &ALFA;
	PROCERargs.STS_in_empty = &STS_in_empty;
	PROCERargs.log_mutex = &log_mutex;

	t_ioThreadsArgs ioThreadsArgs;
	ioThreadsArgs.LFIO = LFIO;
	ioThreadsArgs.LFIO_empty = &LFIO_empty;
	ioThreadsArgs.LFIO_mutex = &LFIO_mutex;
	ioThreadsArgs.LPB = LPB;
	ioThreadsArgs.LPB_empty = &LPB_empty;
	ioThreadsArgs.LPB_mutex = &LPB_mutex;
	ioThreadsArgs.io_threads = &ioThreadsCount;
	ioThreadsArgs.IO_TIME = &IO_TIME;
	ioThreadsArgs.STS_in_empty = &STS_in_empty;
	ioThreadsArgs.log_mutex = &log_mutex;

	pthread_t STSth;
	pthread_create(&STSth, NULL, (void*) STS, (void*) &STSargs);

	pthread_create(&LTSnew_thread, NULL, (void*) LTSnew, (void*) &LTSnewArgs);
	pthread_create(&LTSconex_thread, NULL, (void*) LTSconex,
			(void*) &LTSconexArgs);
	pthread_create(&PROCER_thread, NULL, (void*) PROCER, (void*) &PROCERargs);
	ioThreadsPool = ioThreadsPoolCreate(&ioThreadsArgs, IO_THREADS);

	puts("\nEspere aviso de PP listo..");
	sleep(3);
	puts("PP listo");

	pthread_join(SigHandler_thread, NULL ); //XXX, a partir de este, empiezo a cancelar los demás

	for (i = 0; i < IO_THREADS; i++)
	{
		pthread_cancel(ioThreadsPool[i]);
	}

	pthread_cancel(LTSconex_thread);
	pthread_cancel(LTSnew_thread);
	pthread_cancel(PROCER_thread);
	pthread_cancel(STSth);
	pthread_cancel(configTh);

	pthread_detach(SigHandler_thread);
	pthread_detach(configTh);
	pthread_detach(STSth);
	pthread_detach(LTSnew_thread);
	pthread_detach(LTSconex_thread);
	pthread_detach(PROCER_thread);
	for (i = 0; i < IO_THREADS; i++)
	{
		pthread_detach(ioThreadsPool[i]);
	}

	list_destroy(sendingList);
	list_destroy(LPS);
	queue_destroy(LPN);
	queue_destroy(LFIFO);
	queue_destroy(LPB);
	queue_destroy(LFIO);
	queue_destroy(LPR);
	queue_destroy(LFQ);
	queue_destroy(LPL);

	exit(EXIT_SUCCESS);
}

void *signal_handler(t_sigHandStruct *sigHandStruct)
{
	sigHandStruct->sigHandPid = getpid();
	sigset_t SigSet;
	int senial, semValue;

	sigfillset(&SigSet);

	while (1)
	{
		sigwait(&SigSet, &senial);
		switch (senial)
		{
		case SIGUSR1:
		{
			(*sigHandStruct->SUSPEND) = true;
			break;
		}
		case SIGINT:
		{
			sem_getvalue(sigHandStruct->MPS, &semValue);
			if (semValue == sigHandStruct->MPS_max_val)
			{
				pthread_exit(NULL );
			}
			else
			{
				puts("    Todavía se encuentran procesos en el sistema");
			}
			break;
		}
		default:
		{
			puts("    SIGINT para finalizar correctamente");
			break;
		}
		} //fin switch
	}
	return NULL ;
}

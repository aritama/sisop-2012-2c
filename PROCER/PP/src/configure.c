#include "configure.h"

void* configure(t_configure* configStruct)
{
	ssize_t len, i = 0;
	char buff[BUFF_SIZE] =
	{ 0 };
	char* target = "./";
	char* config_path = malloc(strlen(target) + strlen("pp.config") + 1);
	*config_path = '\0';
	strcat(config_path, target);
	strcat(config_path, "pp.config");
	t_config* config = config_create(config_path);

	configurationReader(config, configStruct);

	int fd;
	int wd;

	fd = inotify_init();
	if (fd < 0)
	{
		handle_error(errno);
		abort();
	}

	wd = inotify_add_watch(fd, target, IN_MODIFY);
	if (wd < 0)
	{
		handle_error(errno);
		abort();
	}

	while (true)
	{
		i = 0;

		len = read(fd, (void*) buff, BUFF_SIZE);

		while (i < len)
		{
			struct inotify_event *pevent = (struct inotify_event *) &buff[i];
			char action[81 + FILENAME_MAX] =
			{ 0 };

			if (pevent->len)
				strcpy(action, pevent->name);
			else
				strcpy(action, target);

			if (pevent->mask & IN_MODIFY)
			{
				if (!strcmp(action, "pp.config"))
				{
					puts("PP.config modificado");
					config_destroy(config);
					config = config_create(config_path);
					configurationReader(config, configStruct);
				}
			}

			i += sizeof(struct inotify_event) + pevent->len;

		}
	}

	config_destroy(config);
	return NULL ;
}

void handle_error(int error)
{
	fprintf(stderr, "Error: %s\n", strerror(error));

}

void configurationReader(t_config* config, t_configure* configStruct)
{
	t_queuesArray* qArray = configStruct->queuesArray;
	int i;
	t_list* blankList = list_create();

	pthread_mutex_lock(configStruct->ALG_mutex);
	*configStruct->ALG = config_get_string_value(config, "ALG");
	if (!strcmp(*configStruct->ALG, "PRI"))
	{
		orderInsertPrioList(blankList, configStruct->fakeSTSargs);
	}
	if (!strcmp(*configStruct->ALG, "SPN"))
	{
		orderInsertSPNList(blankList, configStruct->fakeSTSargs);
	}
	list_destroy(blankList);
	pthread_mutex_unlock(configStruct->ALG_mutex);

	pthread_mutex_lock(configStruct->QueuePriorities_mutex);
	for (i = 0; i < 4; i++)
		qArray[i].priority = config_get_int_value(config, qArray[i].name);
	reorderQueuesArrayPriority(configStruct->queuesArray);
	pthread_mutex_unlock(configStruct->QueuePriorities_mutex);

	pthread_mutex_lock(configStruct->SENT_TIME_mutex);
	*configStruct->SENT_TIME = config_get_int_value(config, "SENT_TIME");
	pthread_mutex_unlock(configStruct->SENT_TIME_mutex);

	if (!(*configStruct->flagReady))
	{
		*configStruct->PORT = config_get_int_value(config, "PORT");
		*configStruct->quantum = config_get_int_value(config, "QUANTUM");
		*configStruct->IO_THREADS = config_get_int_value(config, "IO_THREADS");
		*configStruct->IO_TIME = config_get_int_value(config, "IO_TIME");
		*configStruct->MMP = config_get_int_value(config, "MMP");
		*configStruct->MPS = config_get_int_value(config, "MPS");
		*configStruct->TANT = config_get_int_value(config, "TANT");
		*configStruct->ALFA = atof(config_get_string_value(config, "ALFA"));

		*configStruct->flagReady = true;
	}

	return;
}

void reorderQueuesArrayPriority(t_queuesArray* queuesArray)
{
	int i;
	bool flag = false;
	t_queuesArray aux;
	while (!flag)
	{
		flag = true;
		for (i = 0; i < 3; i++)
		{
			if (queuesArray[i].priority > queuesArray[i + 1].priority)
			{
				aux = queuesArray[i + 1];
				queuesArray[i + 1] = queuesArray[i];
				queuesArray[i] = aux;
				flag = false;
			}
		}
	}
	return;
}

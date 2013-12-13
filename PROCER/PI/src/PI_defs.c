#include "PI_defs.h"

char* obtenerPathDelScript(char** argv, char** env)
{
	int i;

	for (i = 0; (env[i] != NULL )
	&& ((env[i][0] != 'P') || (env[i][1] != 'W')
			|| (env[i][2] != 'D')); i++);

	char* path = malloc(strlen(env[i]) + strlen(argv[1]));
	strcpy(path, &env[i][4]);
	char* sep = "/";
	strcat(path, sep);
	strcat(path, argv[1]);

	return path;
}

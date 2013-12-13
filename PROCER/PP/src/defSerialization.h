#ifndef SERIALIZATION_H_
#define SERIALIZATION_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct
{
	char type;
	char *data;
}__attribute__((__packed__)) t_data_stream;

typedef struct
{
	uint16_t length;
	char *data;
}__attribute__((__packed__)) t_stream;

t_stream* serialization(t_data_stream* data);

t_data_stream* deserialization(char* stream);

t_data_stream* deserializationPri(char* stream, int32_t *priority);

typedef struct
{
	char type;
	uint16_t length;
}__attribute__((__packed__)) t_header;

#endif /* SERIALIZATION_H_ */

#include "defSerialization.h"

//t_data* deserialization(char* stream); //type,length,data
t_data_stream* deserialization(char* stream)
{
	t_data_stream* data = malloc(sizeof(t_data_stream));
	uint16_t length = 0;
	uint32_t offset = 0, tmp_size = 0;

	memcpy(&data->type, stream, tmp_size = sizeof(char));

	offset += tmp_size;
	memcpy(&length, stream + offset, tmp_size = sizeof(uint16_t));

	length = length - sizeof(char) - sizeof(uint16_t);
	data->data = malloc(length * sizeof(char));

	offset += tmp_size;
	memcpy(data->data, stream + offset, length);

	return data;
}

//char* serialization(t_data* data);
t_stream* serialization(t_data_stream* datos)
{
	uint16_t length = sizeof(char) + sizeof(uint16_t) + strlen(datos->data) + 1;
	char* data = malloc(length * sizeof(char));
	t_stream* stream = malloc(sizeof(t_stream));
	uint32_t offset = 0, tmp_size = 0;

	memcpy(data, &datos->type, tmp_size = sizeof(char));

	offset += tmp_size;
	memcpy(data + offset, &length, tmp_size = sizeof(uint16_t));

	offset += tmp_size;
	memcpy(data + offset, datos->data, tmp_size = (strlen(datos->data) + 1));

	stream->length = offset + tmp_size;
	stream->data = data;

	return stream;
}

t_stream* serializationPri(t_data_stream* datos, int32_t *priority)
{
	uint16_t length = sizeof(char) + sizeof(uint16_t) + sizeof(int32_t)
			+ strlen(datos->data) + 1;
	char* data = malloc(length * sizeof(char));
	t_stream* stream = malloc(sizeof(t_stream));
	uint32_t offset = 0, tmp_size = 0;

	offset += tmp_size;
	memcpy(data + offset, &datos->type, tmp_size = sizeof(char));

	offset += tmp_size;
	memcpy(data + offset, &length, tmp_size = sizeof(uint16_t));

	offset += tmp_size;
	memcpy(data + offset, priority, tmp_size = sizeof(int32_t));

	offset += tmp_size;
	memcpy(data + offset, datos->data, tmp_size = (strlen(datos->data) + 1));

	stream->length = offset + tmp_size;
	stream->data = data;

	return stream;
}


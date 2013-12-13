#include "client_defs.h"

int create_and_connect(uint16_t PORT, char* IP_SERVER)
{
	int sockfd;
	struct sockaddr_in dest_addr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
		return ERROR;

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(PORT);
	if (IP_SERVER == NULL )
		dest_addr.sin_addr.s_addr = htonl(INADDR_ANY );
	else
		dest_addr.sin_addr.s_addr = inet_addr(IP_SERVER);
	bzero(&(dest_addr.sin_zero), sizeof(dest_addr.sin_zero));

	if ((connect(sockfd, (struct sockaddr *) &dest_addr,
			sizeof(struct sockaddr))) == ERROR)
		return ERROR;

	return sockfd;
}

int makeHandshake(int sockfd)
{
	char code = '#';
	if (send(sockfd, &code, sizeof(char), 0) == ERROR)
	{
		return ERROR;
	}
	if (recv(sockfd, &code, sizeof(char), 0) == ERROR)
	{
		return ERROR;
	}
	if (code != '!')
	{
		return ERROR;
	}
	return true;
}

int readStream(int sockfd, t_data_stream** data)
{
	t_header header;
	if (recv(sockfd, &header, sizeof(t_header), MSG_PEEK) <= 0)
		return ERROR;
	char* stream_in = malloc(header.length);
	if (recv(sockfd, stream_in, header.length, 0) <= 0)
	{
		free(stream_in);
		return ERROR;
	}
	*data = deserialization(stream_in);
	free(stream_in);
	return 1;
}

int writeStream(int sockfd, t_data_stream* data)
{
	t_stream* stream = serialization(data);
	if (send(sockfd, stream->data, stream->length, 0) <= 0)
		return ERROR;
	free(stream->data);
	free(stream);
	return 1;
}

int writeStreamPri(int sockfd, t_data_stream* data, int32_t *priority)
{
	t_stream* stream = serializationPri(data, priority);
	if (send(sockfd, stream->data, stream->length, 0) <= 0)
		return ERROR;
	free(stream->data);
	free(stream);
	return 1;
}

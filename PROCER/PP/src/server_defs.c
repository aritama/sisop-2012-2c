#include "server_defs.h"
#include "defSerialization.h"

int createBindListen(uint16_t PORT, char* IP_SERVER)
{

	struct sockaddr_in sockaddr;
	int serverfd;

	if ((serverfd = socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
		return ERROR;

	if (IP_SERVER == NULL )
		sockaddr.sin_addr.s_addr = htonl(INADDR_ANY );
	else
		sockaddr.sin_addr.s_addr = inet_addr(IP_SERVER);

	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(PORT);
	bzero(&sockaddr.sin_zero, sizeof(sockaddr.sin_zero));

	int yes = 1;
	if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &yes,
			sizeof(int)) == ERROR)
		return ERROR;

	if (bind(serverfd, (struct sockaddr*) &sockaddr,
			sizeof(struct sockaddr)) == ERROR)
		return ERROR;

	if (listen(serverfd, SOMAXCONN) == ERROR)
		return ERROR;

	return serverfd;
}

int makeHandshake(int sockfd)
{
	char code;
	if (recv(sockfd, &code, sizeof(char), 0) == ERROR)
	{
		perror("handshake send error");
		return ERROR;
	}
	if (code != '#')
	{
		perror("handshake wrong");
		return ERROR;
	}
	code = '!';
	if (send(sockfd, &code, sizeof(char), 0) == ERROR)
	{
		perror("handshake recv error");
		return ERROR;
	}
	return true;
}

int accept_connection(int serverfd)
{
	int clientfd;
	socklen_t sin_size;
	struct sockaddr_in clientAddr;
	sin_size = (socklen_t) sizeof(struct sockaddr_in);
	if ((clientfd = accept(serverfd, (struct sockaddr*) &clientAddr, &sin_size))
			== ERROR)
		perror("accept error");
	return clientfd;
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

int readStreamPri(int sockfd, t_data_stream** data, int32_t* priority)
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
	*data = deserializationPri(stream_in, priority);
	free(stream_in);
	return 1;
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


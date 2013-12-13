#ifndef SERVER_DEFS_H_
#define SERVER_DEFS_H_

#define ERROR -1
#define MAX_EVENTS 64

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "defSerialization.h"

int createBindListen(uint16_t PORT, char* IP_SERVER);

int makeHandshake(int sockfd);

int accept_connection(int serverfd);

int readStream(int clientfd, t_data_stream** stream);

int writeStream(int sockfd, t_data_stream* data);

int readStreamPri(int sockfd, t_data_stream** data, int32_t* priority);

#endif /* SERVER_DEFS_H_ */

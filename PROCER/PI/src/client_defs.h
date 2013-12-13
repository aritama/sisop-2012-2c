#ifndef CLIENT_DEFS_H_
#define CLIENT_DEFS_H_

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "defSerialization.h"

#define ERROR -1

int create_and_connect(uint16_t PORT, char* IP_SERVER);

int makeHandshake(int sockfd);

int readStream(int clientfd, t_data_stream** stream);

int writeStream(int sockfd, t_data_stream* data);

int writeStreamPri(int sockfd, t_data_stream* data, int32_t *priority);

#endif /* CLIENT_DEFS_H_ */

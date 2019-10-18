#ifndef __SOCKET_SIMPLE_H__
#define __SOCKET_SIMPLE_H__

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include <time.h>
#include <sys/queue.h>

#include "utils.h"

#define MODULE_NAME "Socket: "
#define DEBUG(fmt, args...) printf(MODULE_NAME fmt, ##args);

#define BACKLOG     10
#define SERVER_PORT 8888
#define CLIENT_DEFAULT_IP "127.0.0.1"
#define BUFFER_MAX  1024

struct SocketCommon {
   	int fd;
	int max_fd;
	fd_set fd_list;
	
	struct sockaddr_in tSocketAddr_in;	
};

struct SocketClient {
	char *ip;
	char *name;

	int message_len;
	struct SocketCommon desc;
	SLIST_ENTRY(entry) entries;
};

struct SocketServer {
	struct SocketCommon desc;
};

struct entry {
	int fd;
	char *ip;
	char *name;
	
	SLIST_ENTRY(entry) entries;     /* Singly-linked List. */
 };


#endif /* __SOCKET_SIMPLE_H__ */

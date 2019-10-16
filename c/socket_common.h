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

#define SERVER_PORT 8888
#define BACKLOG     10

#define N_SUFFIX(x, len) \
    x[len] = '\n'

struct SocketServer {
   	int iSocket_fd;
	struct sockaddr_in tSocketAddr_in;
};

struct SocketClient {
	
};

typedef enum {
	TPYE_IP = 0,
	TPYE_NAME = 1,
	TPYE_MESSAGE = 2,
}SocketType;	
int incode(const char *msg);
int encode(SocketType type, const char *msg);

#endif /* __SOCKET_SIMPLE_H__ */

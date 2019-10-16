#include <signal.h>
#include "socket_common.h"

#define MODULE_NAME "Server: "
#define DEBUG(fmt, args...) printf(MODULE_NAME fmt, ##args);

int init_socket(struct SocketServer *Socket)
{	
	int iRet;
	
	Socket->iSocket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (Socket->iSocket_fd < 0) {
		printf("socket error!\n");
		return -ENAVAIL;
	}
	
	Socket->tSocketAddr_in.sin_family	     = AF_INET;
	Socket->tSocketAddr_in.sin_port		 = htons(SERVER_PORT);  /* host to net, short */
	Socket->tSocketAddr_in.sin_addr.s_addr = INADDR_ANY;
	memset(Socket->tSocketAddr_in.sin_zero, 0, 8);
	
	if (bind(Socket->iSocket_fd, (const struct sockaddr *)&Socket->tSocketAddr_in, sizeof(struct sockaddr)) < 0) {
		DEBUG("bind error!\n");
		return -ENAVAIL;
	}
	
	if (listen(Socket->iSocket_fd, BACKLOG) < 0) {
		DEBUG("listen error!\n");
		return -ENAVAIL;
	}

	DEBUG("#-----Socket Service online-----#\n");
	return 0;
}

int loop_server(struct SocketServer *Socket)
{
	int iRecvLen;
	int iAddrLen = sizeof(struct sockaddr);
	unsigned char ucRecvBuf[1000];	
	
	int iSocketClient;	
	struct sockaddr_in tSocketClientAddr;
	
	while (1) {
		iSocketClient = accept(Socket->iSocket_fd, (struct sockaddr *)&tSocketClientAddr, &iAddrLen);
		if (iSocketClient != -1) {
			// first message is name
			iRecvLen = recv(iSocketClient, ucRecvBuf, 999, 0);
			if (iRecvLen < 0) {
				goto recv_err;
			}
			N_SUFFIX(ucRecvBuf, iRecvLen);
			
			DEBUG("connect from client : %s(%s)\n", inet_ntoa(tSocketClientAddr.sin_addr), ucRecvBuf);
			if (!fork()) {
				while (1) {
					iRecvLen = recv(iSocketClient, ucRecvBuf, 999, 0);
					if (iRecvLen <= 0) {
						goto recv_err;
					} else {
						N_SUFFIX(ucRecvBuf, iRecvLen);
						DEBUG("Get Msg From Client %s\n", ucRecvBuf);
					}
				}
			}
		}
	}

recv_err:
    close(iSocketClient);
	return -ENAVAIL;
}

int main(int argc, char **argv)
{	
    struct SocketServer Socket;

	signal(SIGCHLD, SIG_IGN);
    init_socket(&Socket);	
	loop_server(&Socket);
	
kill:
	close(Socket.iSocket_fd);
	
	return 0;
}


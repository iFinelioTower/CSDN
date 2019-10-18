#include <signal.h>
#include "socket_common.h"

#ifdef MODULE_NAME
#undef MODULE_NAME
#define MODULE_NAME "Server: "
#endif

struct entry * wait_online(int fd, char *ip) {
    unsigned char ucRecvBuf[BUFFER_MAX];
	
	int recvlen = recv(fd, ucRecvBuf, BUFFER_MAX - 1, 0);
	if (recvlen > 0) {
		struct entry *client = malloc(sizeof(struct entry));	
		client->fd = fd;
		
		client->ip = malloc(sizeof(char) * strlen(ip));
		strcpy(client->ip, ucRecvBuf);
		
		client->name = malloc(sizeof(char) * strlen(ucRecvBuf));
		strcpy(client->name, ucRecvBuf);

		return client;
	}

	return NULL;
}

int init_socket(struct SocketServer *Socket)
{	
	Socket->desc.fd = socket(AF_INET, SOCK_STREAM, 0);
	if (Socket->desc.fd < 0) {
		printf("socket error!\n");
		return -ENAVAIL;
	}
	
	Socket->desc.tSocketAddr_in.sin_family	     = AF_INET;
	Socket->desc.tSocketAddr_in.sin_port		 = htons(SERVER_PORT);  /* host to net, short */
	Socket->desc.tSocketAddr_in.sin_addr.s_addr = INADDR_ANY;
	memset(Socket->desc.tSocketAddr_in.sin_zero, 0, 8);
	
	if (bind(Socket->desc.fd, (const struct sockaddr *)&Socket->desc.tSocketAddr_in, sizeof(struct sockaddr)) < 0) {
		DEBUG("bind error!\n");
		return -ENAVAIL;
	}
	
	if (listen(Socket->desc.fd, BACKLOG) < 0) {
		DEBUG("listen error!\n");
		return -ENAVAIL;
	}
	
	DEBUG("#-----Socket Service online-----#\n");
	return 0;
}

int loop_server(struct SocketServer *Socket)
{
	int iAddrLen = sizeof(struct sockaddr);
	unsigned char ucSendBuf[BUFFER_MAX];
	unsigned char ucRecvBuf[BUFFER_MAX];
	
	int iSocketClient;	
	struct sockaddr_in tSocketClientAddr;

	SLIST_HEAD(slisthead, entry) head = SLIST_HEAD_INITIALIZER(&head);
	SLIST_INIT(&head);
	
	struct timeval tv = {20, 0};
	while (1) {
		LISTENER_INIT(Socket->desc.fd_list);
		LISTENER_ADD(STDIN_FILENO, Socket->desc.fd_list);
	    LISTENER_ADD(Socket->desc.fd, Socket->desc.fd_list);
		
		Socket->desc.max_fd = CHECK_MAX(STDIN_FILENO, Socket->desc.fd);
        int ret = select(Socket->desc.max_fd + 1, &Socket->desc.fd_list, NULL, NULL, &tv);
		if(ret <= 0) {
            continue;
        } else {
            if (LISTENER_INQUIRE(STDIN_FILENO, Socket->desc.fd_list)) {
				int len = -1;
				bzero(ucSendBuf, BUFFER_MAX);
				len = getMessage(ucSendBuf, BUFFER_MAX);
                if (len > 0) {
					DEBUG("broadcast: %s \n", ucSendBuf);
					struct entry *np;
				    if (!SLIST_EMPTY(&head)) {
						SLIST_FOREACH(np, &head, entries)
							send(np->fd, ucSendBuf, len, 0);
					}
				}
            }
			
            if (LISTENER_INQUIRE(Socket->desc.fd, Socket->desc.fd_list)) {  
				iSocketClient = accept(Socket->desc.fd, (struct sockaddr *)&tSocketClientAddr, &iAddrLen);
				if (iSocketClient != -1) {
					struct entry *client = wait_online(iSocketClient, inet_ntoa(tSocketClientAddr.sin_addr));
				    if (client) {
				        DEBUG("ADD fd:%d, ip:%s, name:%s \n", client->fd, client->ip, client->name);
						SLIST_INSERT_HEAD(&head, client, entries);
					}

					// broadcast all, user online
					if (!SLIST_EMPTY(&head)) {
						struct entry *np;
						SLIST_FOREACH(np, &head, entries)
						    if (np->fd != iSocketClient) {
								send(np->fd, client->name, strlen(client->name), 0);
							}
					}
					
					DEBUG("connect from client : %s\n", inet_ntoa(tSocketClientAddr.sin_addr));
				    if (!fork()) {
						int message_len = 0;
						while (1) {
						    bzero(ucRecvBuf, message_len);
							int recvlen = recv(iSocketClient, ucRecvBuf, BUFFER_MAX - 1, 0);
							if (recvlen > 0) {
								struct entry *np;
								message_len = recvlen;
							    SLIST_FOREACH(np, &head, entries)
									if (np->fd != iSocketClient) {
										send(np->fd, ucRecvBuf, recvlen, 0);
									}

									DEBUG("recv: %s\n", ucRecvBuf);
							} else {
								struct entry *np;
								DEBUG("有客户端退出了\n");
								SLIST_FOREACH(np, &head, entries)
									if (np->fd == iSocketClient) {
										SLIST_REMOVE(&head, np, entry, entries);/* Deletion. */
										free(np);
									}
									
								goto client_exit;
							}
						}
					}
				}
            }  
        }
	}
	
client_exit:
    close(iSocketClient);
	return 0;
}

int main(int argc, char **argv)
{	
    struct SocketServer server;
	
	signal(SIGCHLD, SIG_IGN);
    init_socket(&server);
	loop_server(&server);
	
kill:
	close(server.desc.fd);
	
	return 0;
}


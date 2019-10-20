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
		strcpy(client->ip, ip);
		
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

inline void boardcastMessageAll(SLIST_HEAD(slisthead, entry) *list, char *msg, int len)
{
    struct entry *bnp;
    if (!SLIST_EMPTY(list)) {
		SLIST_FOREACH(bnp, list, entries)
			send(bnp->fd, msg, len, 0);
	}
}

inline void boardcastMessageToOtherUser(SLIST_HEAD(slisthead, entry) *list, int self_fd, char *msg, int len)
{
    struct entry *bnp;
    if (!SLIST_EMPTY(list)) {
		SLIST_FOREACH(bnp, list, entries) {
			if (self_fd != bnp->fd)
			send(bnp->fd, msg, len, 0);
		}
	}
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
		
		struct entry *np;
	    if (!SLIST_EMPTY(&head)) {
			SLIST_FOREACH(np, &head, entries) {
				LISTENER_ADD(np->fd, Socket->desc.fd_list);
				Socket->desc.max_fd = CHECK_MAX(np->fd, Socket->desc.max_fd);
			}
		}
		
        int ret = select(Socket->desc.max_fd + 1, &Socket->desc.fd_list, NULL, NULL, &tv);
		if(ret <= 0) {
            continue;
        } else {
            if (LISTENER_INQUIRE(STDIN_FILENO, Socket->desc.fd_list)) {
				int len = -1;
				unsigned char ucBoardcastBuf[BUFFER_MAX];
				bzero(ucBoardcastBuf, BUFFER_MAX);
				len = waitInputMessage(ucBoardcastBuf, BUFFER_MAX);
                if (len > 0) {
					DEBUG("broadcast: %s \n", ucBoardcastBuf);
					bzero(ucSendBuf, BUFFER_MAX);  
					strcpy(ucSendBuf, "'Service Broadcast': ");
					strcat(ucSendBuf, ucBoardcastBuf);
					boardcastMessageAll(&head, ucSendBuf, strlen(ucSendBuf));
				}
            }
			
            if (LISTENER_INQUIRE(Socket->desc.fd, Socket->desc.fd_list)) {  
				iSocketClient = accept(Socket->desc.fd, (struct sockaddr *)&tSocketClientAddr, &iAddrLen);
				if (iSocketClient != -1) {
					struct entry *client = wait_online(iSocketClient, inet_ntoa(tSocketClientAddr.sin_addr));
				    if (client) {
				        DEBUG("connect fd:%d, ip:%s, name:%s \n", client->fd, client->ip, client->name);
						SLIST_INSERT_HEAD(&head, client, entries);
					
						// broadcast all, have user online!.
						bzero(ucSendBuf, BUFFER_MAX);  
						strcpy(ucSendBuf, client->name);
						strcat(ucSendBuf, " online! \n");
						boardcastMessageToOtherUser(&head, iSocketClient, ucSendBuf, strlen(ucSendBuf));
					}
				}
            }  

			if (!SLIST_EMPTY(&head)) {
				SLIST_FOREACH(np, &head, entries)
					if (LISTENER_INQUIRE(np->fd, Socket->desc.fd_list)) {
						 bzero(ucRecvBuf, BUFFER_MAX);
						 int recvlen = recv(np->fd, ucRecvBuf, BUFFER_MAX - 1, 0);
					     if (recvlen > 0) {
						 	// boardcase message
						 	bzero(ucSendBuf, BUFFER_MAX); 
							strcpy(ucSendBuf, "'");
							strcat(ucSendBuf, np->name);
							strcat(ucSendBuf, "': ");
							strcat(ucSendBuf, ucRecvBuf);
						 	boardcastMessageToOtherUser(&head, np->fd, ucSendBuf, strlen(ucSendBuf));
							DEBUG("%s\n", ucSendBuf);
							
						} else {
							DEBUG("disconnet fd: %d, ip: %s, name: %s\n", np->fd, np->ip, np->name);
							
							// broadcast other user, have guy online!.
							bzero(ucSendBuf, BUFFER_MAX);  
							strcpy(ucSendBuf, np->name);
							strcat(ucSendBuf, " offline! \n");
							boardcastMessageToOtherUser(&head, np->fd, ucSendBuf, strlen(ucSendBuf));
							
							SLIST_REMOVE(&head, np, entry, entries);
							LISTENER_DEL(np->fd, Socket->desc.fd_list);
							close(np->fd);
							free(np);
						}
					}
			}
        }
	}

exit:
	return 0;
}

int main(int argc, char **argv)
{	
    struct SocketServer server;
	
	signal(SIGCHLD, SIG_IGN);
    if (init_socket(&server)) 
		goto kill;

	loop_server(&server);
	
kill:
	close(server.desc.fd);
	return 0;
}


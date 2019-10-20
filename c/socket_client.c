#include "socket_common.h"

#ifdef MODULE_NAME
#undef MODULE_NAME
#define MODULE_NAME "Client: "
#endif

#define Tips(x, y) \
	DEBUG("Usage default config, ip: %s, name: %s\n", x, y)

void online(struct SocketClient *Socket) {
	send(Socket->desc.fd, Socket->name, strlen(Socket->name), 0);
	free(Socket->name);
}

int init_socket(struct SocketClient *Socket)
{	
	Socket->desc.fd = socket(AF_INET, SOCK_STREAM, 0);
	Socket->desc.tSocketAddr_in.sin_family      = AF_INET;
	Socket->desc.tSocketAddr_in.sin_port        = htons(SERVER_PORT);  /* host to net, short */
 	if (inet_aton(Socket->ip, &Socket->desc.tSocketAddr_in.sin_addr) <= 0) {
		DEBUG("invalid server_ip\n");
		return -ENAVAIL;
	}
	
	memset(Socket->desc.tSocketAddr_in.sin_zero, 0, 8);
	return 0;
}

int loop_client(struct SocketClient *Socket)
{
	int message_len;
	struct timeval tv = {20, 0};
	unsigned char ucSendBuf[BUFFER_MAX];
	unsigned char ucRecvBuf[BUFFER_MAX];
	
	if (connect(Socket->desc.fd, (const struct sockaddr *)&Socket->desc.tSocketAddr_in, sizeof(struct sockaddr)) < 0) {
		DEBUG("connect error!\n");
		return -ENAVAIL;
	}

	// Tell the server your name
	online(Socket);
	
	while (1) {
		LISTENER_INIT(Socket->desc.fd_list);
		LISTENER_ADD(STDIN_FILENO, Socket->desc.fd_list);
        LISTENER_ADD(Socket->desc.fd, Socket->desc.fd_list);  
		
		select(Socket->desc.fd + 1, &Socket->desc.fd_list, NULL, NULL, &tv);  
        if (LISTENER_INQUIRE(STDIN_FILENO, Socket->desc.fd_list)) {  
            int len = waitInputMessage(ucSendBuf, BUFFER_MAX - 1);
			if (len > 0) {
				int start = send(Socket->desc.fd, ucSendBuf, len, 0);
				if (start <= 0) {
					close(Socket->desc.fd);
					return -ENAVAIL;
				}
			}
        }
		
		if (LISTENER_INQUIRE(Socket->desc.fd, Socket->desc.fd_list)) {
			 bzero(ucRecvBuf, BUFFER_MAX);
			 int recvlen = recv(Socket->desc.fd, ucRecvBuf, BUFFER_MAX - 1, 0);
		     if (recvlen > 0) {
				DEBUG("%s\n", ucRecvBuf);
			 } else if (recvlen < 0) {
				DEBUG("recv error!\n");
			 } else {
			    DEBUG("Server offline, Abort!\n");
				exit(0);
			 }
		}
	}
	
	return 0;
}

char *createName() {
	time_t nSeconds;
    struct tm * pTM;
    char *tmpName = malloc(sizeof(char) * 24);
	
    time(&nSeconds);
    pTM = localtime(&nSeconds);
	sprintf(tmpName, "user_%02d:%02d:%02d", pTM->tm_hour, pTM->tm_min, pTM->tm_sec);	
	
	return tmpName;	
}

int main(int argc, char **argv)
{
	struct SocketClient client;
    if (argc != 2) {
		client.ip = CLIENT_DEFAULT_IP;
	} else {
	    client.ip = argv[1];
	}
	client.name = createName();
	Tips(client.ip, client.name);

	init_socket(&client);
	loop_client(&client);
	
	return 0;
}

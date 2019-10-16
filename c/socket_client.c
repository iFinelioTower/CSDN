#include "socket_common.h"

#define MODULE_NAME "Client"

int main(int argc, char **argv)
{
	int iSocketClient;
	struct sockaddr_in tSocketServerAddr;
	
	int ret;
	unsigned char ucSendBuf[1000];
	int iSendLen;

	if (argc != 2) {
		printf("Usage:\n");
		printf("%s <server_ip> [nickname]\n", argv[0]);
		return -ENAVAIL;
	}
	
	iSocketClient = socket(AF_INET, SOCK_STREAM, 0);
	tSocketServerAddr.sin_family      = AF_INET;
	tSocketServerAddr.sin_port        = htons(SERVER_PORT);  /* host to net, short */
	
 	if (inet_aton(argv[1], &tSocketServerAddr.sin_addr) <= 0) {
		printf("invalid server_ip\n");
		return -ENAVAIL;
	}
	
	memset(tSocketServerAddr.sin_zero, 0, 8);
	if (connect(iSocketClient, (const struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr)) < 0) {
		printf("connect error!\n");
		return -ENAVAIL;
	}

	while (1) {
		if (fgets(ucSendBuf, 999, stdin)) {
			iSendLen = send(iSocketClient, ucSendBuf, strlen(ucSendBuf), 0);
			if (iSendLen <= 0) {
				close(iSocketClient);
				return -ENAVAIL;
			}
		}
	}
	
	return 0;
}


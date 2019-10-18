#ifndef __SOCKET_UTILS_H__
#define __SOCKET_UTILS_H__ 

#include <sys/types.h>          /* See NOTES */
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#define CHECK_MAX(x, y) (x > y ? x : y)
#define LISTENER_INIT(list) FD_ZERO(&list)
#define LISTENER_ADD(fd, list) FD_SET(fd, &list)
#define LISTENER_INQUIRE(fd, list) FD_ISSET(fd, &list)
#define LISTENER_DEL(fd, list) FD_CLR(fd, &list)

int getMessage(char *buf, int size);

#endif /* __SOCKET_UTILS_H__ */


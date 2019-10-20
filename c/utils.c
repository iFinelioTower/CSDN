#include "utils.h"

#define MESSAGE_FROM stdin
inline int waitInputMessage(char *buf, int size)
{
	if (fgets(buf, size, MESSAGE_FROM))
		if (strlen(buf) != 1) 
			return strlen(buf) - 1;
		
    return 0;
}

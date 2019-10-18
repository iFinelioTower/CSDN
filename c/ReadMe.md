/*
 * Copyright (C) 2012 The iFinelio Tower
 * Of course, you can contact me through email if you have any questions
 *
 *      E-mail: <iFinelio Tower>308662170@qq.com
 * 
 * You can obtain the source code in the following ways:
 *      git clone https://github.com/iFinelioTower/CSDN -b socket
 * 
 * The source code only with learning, Thank you for watching.
 */

编译如下:
gcc -o client utils.c socket_client.c
gcc -o server utils.c socket_server.c

./server &
./client [local_ip] [pickname]
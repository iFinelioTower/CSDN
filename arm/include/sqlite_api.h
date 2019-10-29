/*
 * Copyright (C) 2012 The iFinelio Tower
 * Of course, you can contact me through email if you have any questions
 *
 *      E-mail: <iFinelio Tower>308662170@qq.com
 * 
 * You can obtain the source code in the following ways:
 *      git clone https://github.com/iFinelioTower/CSDN -b sqlite
 * 
 * The source code only with learning, Thank you for watching.
 */

#ifndef __SQLITE_API_H__
#define __SQLITE_API_H__

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include <sqlite3.h>

#define SQLITE_OPEN(path, db) sqlite3_open_v2(path, db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)
#define SQLITE_CLOSE(db) sqlite3_close_v2(db)

#endif //__SQLITE_API_H__
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include <sqlite_api.h>

#define DATABASE_PATH "/data/database.sqlite3"

sqlite3 *open(char *path, int mode)
{
	sqlite3 *db = NULL;

	// initialize
	sqlite3_initialize();

	// create
	if (SQLITE_OPEN(path, &db) != SQLITE_OK) {
		SQLITE_CLOSE(db);
		return NULL;
	}
	
	return db;
}


#define GENERATE_CREATETABLE(table, parmlist)

int createTable(sqlite3 *db) 
{
	char *log = 0;
	char *sql;
	
	sql = "CREATE TABLE SensorData(\
		ID INTEGER PRIMARY KEY, \
		SensorID INTEGER, \
		SiteNum INTEGER, \
		Time VARCHAR(12), \
		SensorParameter REAL \
		);";

	sqlite3_exec(db, sql, 0, 0, &log);

	return 0;
}

#define GENERATE_INSERT(table, value)
int insert(sqlite3 *db, char *origin)
{
	char *log = 0;
	char *sql;
	
	sql = "INSERT INTO \"SensorData\" VALUES(NULL , 1 , 1 , '200605011206', 18.9 );" ;
	sqlite3_exec( db , sql , 0 , 0 , &log );
 
	sql = "INSERT INTO \"SensorData\" VALUES(NULL , 23 , 45 , '200605011306', 16.4 );" ;
	sqlite3_exec( db , sql , 0 , 0 , &log );
 
	sql = "INSERT INTO \"SensorData\" VALUES(NULL , 34 , 45 , '200605011306', 15.4 );" ;
	sqlite3_exec( db , sql , 0 , 0 , &log );
	
	//sqlite3_exec(db , origin , 0 , 0 , &log);
	return 0;
}

int delete(sqlite3 *db, char *origin)
{
	//删除数据
	char *log = 0;
	char *sql;
	
	sql = "DELETE FROM SensorData WHERE SensorID = 1 ;" ;
	sqlite3_exec( db , sql , 0 , 0 , &log);
}

int save(sqlite3 *db)
{
	char *log = 0;
	sqlite3_exec(db, NULL , 0 , 0 , &log);
}

int show(sqlite3 *db)
{
	char *log = 0;
	int nrow = 0, ncolumn = 0;
	char *sql;
	char **azResult;

	sql = "SELECT * FROM SensorData ";
	sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &log);

	int i = 0 ;
	printf("row:%d column=%d \n", nrow, ncolumn);
	//printf("\nThe result of querying is : \n" );
	//for(i = 0; i < (nrow + 1) * ncolumn; i++)
	//	printf("azResult[%d] = %s\n", i , azResult[i]);
	
	//释放掉 azResult 的内存空间
	sqlite3_free_table( azResult );
}

int main(int argc, char **argv)
{
	sqlite3 *pDB = NULL;
	int ret = 0;
	int i = 0;
	
	pDB = open(DATABASE_PATH, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
	if (!pDB) {
		goto exit;
	}

	createTable(pDB);
	insert(pDB, "NULL");
    show(pDB);
	
exit:
	/*	perform database operations  */
    if (pDB) {
		SQLITE_CLOSE(pDB);
	}
	
    return 0;
}

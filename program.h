#ifndef PROGRAM_H
#define PROGRAM_H
#include <sqlite3.h>

int createTables(sqlite3 *db,const char *createTable);
int insert_data(sqlite3 *db, const char *sql, const char *value1, int value2);
void syncProduct(sqlite3 *db);
void prgmProcess(sqlite3 *db);

#endif
#ifndef REPORT_H
#define REPORT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sqlite3.h>

void menuReport(sqlite3 *db);
void reptProcess(sqlite3 *db);
void reportSummary(sqlite3 *db);
void reportDetail(sqlite3 *db);

#endif
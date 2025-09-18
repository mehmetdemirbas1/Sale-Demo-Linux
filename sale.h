#ifndef SALE_H
#define SALE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>   
#include <sqlite3.h>
#include "program.h"

void saleProcess(sqlite3 *db);
void saleProduct(sqlite3 *db);
void payment(sqlite3 *db);
int getLastReceiptNo(sqlite3 *db);
int getReceiptTotal(sqlite3 *db, int receiptNo);
void closeReceipt(sqlite3 *db);

#endif
#ifndef SALE_H
#define SALE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>   
#include <sqlite3.h>
#include "program.h"


void saleProcess(sqlite3 *db);

/**
 * @brief Kullanıcıdan PLU alır, ürünün varlığını kontrol eder ve
 * varsa, ürünü belirtilen fiş numarasıyla ReceiptDetails tablosuna ekler.
 *
 * @param db Aktif SQLite veritabanı bağlantısı.
 * @param currentReceiptNo_ptr İşlem yapılan mevcut fişin numarası.
 */
void saleProduct(sqlite3 *db, int *currentReceiptNo_ptr);

void payment(sqlite3 *db);

int getSingleIntValue(sqlite3 *db, const char *sql, int pluNo);

#endif
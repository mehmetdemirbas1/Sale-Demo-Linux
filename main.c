#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "program.h"
#include "sale.h"
#include "report.h"

sqlite3 *db;
char *zErrMsg = 0;
int rc;

sqlite3 *create_conn(const char* dbName);
void menu();
int DB_Init();



int main()
{

    DB_Init();
    while (1)
    {
        menu();
    }
    
    

    return 0;
}

sqlite3 *create_conn(const char* dbName)
{
    
    int rc = sqlite3_open(dbName, &db);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr,"\tDB cannot open!! : %s\n", sqlite3_errmsg(db));
        return NULL;
    }
    else
    {
        fprintf(stdout, "\tDB is open\n");
    }
    return db;
    
}

void menu()
{

    
    char choice;
    printf("============ MAIN MENU ============\n");
    printf("\t1 - PROGRAM\n");
    printf("\t2 - SALE\n");
    printf("\t3 - REPORT\n");
    printf("\t4 - EXIT\n");
    printf("\t>");
    scanf("%s",&choice);
    
    switch (choice)
    {
    case '1':
        prgmProcess(db);
        break;
    case '2':
        saleProcess(db);
        break;
    case '3':
        reptProcess(db);
        break;
    case '4':
        printf("\tExitting...\n");
        exit(0);
        break;
    default:
        printf("\tInvalid choice pls try again !!!\n");
        break;
    }
}

int DB_Init()
{
        db = create_conn("cashRegisterr.db");
    if (db == NULL)
    {
        return 1;
    }
    const char  *sqlCrTable_Product = "CREATE TABLE IF NOT EXISTS Product ("
                                    "PluNo INTEGER PRIMARY KEY, "
                                    "Name TEXT, "
                                    "Price INTEGER, "
                                    "VatRate INTEGER, "
                                    "Barcode TEXT);";
    const char  *sqlCrTable_Receipt = "CREATE TABLE IF NOT EXISTS Receipt ("
                                    "ReceiptNo INTEGER, "
                                    "ReceiptTotal INTEGER, "
                                    "CashPayment INTEGER, "
                                    "CreditPayment INTEGER);";
    
    const char  *sqlCrTable_ReceiptDetails = "CREATE TABLE IF NOT EXISTS ReceiptDetails ("
                                        "ReceiptNo INTEGER, "
                                        "PluNo INTEGER PRIMARY KEY, "
                                        "PluName TEXT, "
                                        "Price INTEGER);";

    rc = createTables(db,sqlCrTable_Product);
    if (rc != SQLITE_OK){ sqlite3_close(db); return 1;}      
    rc = createTables(db,sqlCrTable_Receipt);
    if (rc != SQLITE_OK){ sqlite3_close(db); return 1;}  
    rc = createTables(db,sqlCrTable_ReceiptDetails);
    if (rc != SQLITE_OK){ sqlite3_close(db); return 1;}  
    

}
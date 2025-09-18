#include "report.h"


void reptProcess(sqlite3 *db)
{
    menuReport(db);
}


void menuReport(sqlite3 *db)
{
    char choice = 0;
    
    printf("============ REPORT MENU ============\n");
    printf("1 - Summary\n");
    printf("2 - Detail\n");
    printf("3 - Back to Main\n");
    printf(">");
    scanf(" %c",&choice);

        switch (choice)
        {
        case '1':
            reportSummary(db);
            break;
        case '2':
            reportDetail(db);
            break;
        case '3':
            return;
        default:
            printf("Invalid Number pls try again!\n");
            break;
        }
    menuReport(db);
}

void reportSummary(sqlite3 *db)
{
    sqlite3_stmt *stmt;
    
    const char *sql = "SELECT ReceiptNo, ReceiptTotal, CashPayment, CreditPayment FROM Receipt;";
    
    FILE *fp = fopen("report/summary.txt", "w");
    if(fp == NULL)
    {
        perror("File creation error!");
        return;
    }
    
    fprintf(fp,"[\n");
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK)
    {
        int firstRecord = 1;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int receiptNo = sqlite3_column_int(stmt, 0);
            int receiptTotal = sqlite3_column_int(stmt, 1);
            int cashPayment = sqlite3_column_int(stmt, 2);
            int creditPayment = sqlite3_column_int(stmt, 3);
            
            if (!firstRecord)
            {
                fprintf(fp, ",\n");
            }
            
            fprintf(fp, "  {\n");
            fprintf(fp, "    \"ReceiptNo\": %d,\n", receiptNo);
            fprintf(fp, "    \"ReceiptTotal\": %d,\n", receiptTotal);
            fprintf(fp, "    \"CashPayment\": %d,\n", cashPayment);
            fprintf(fp, "    \"CreditPayment\": %d\n", creditPayment);
            fprintf(fp, "  }");
            
            firstRecord = 0;
        }
        sqlite3_finalize(stmt);
    }
    else
    {
        fprintf(stderr, "SQL Error :%s\n",sqlite3_errmsg(db));
    }
    
    fprintf(fp, "\n]\n");
    fclose(fp);
    printf("Summary file has succesfully created.\n");
}


void reportDetail(sqlite3 *db)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT T1.ReceiptNo, T1.ReceiptTotal, T1.CashPayment, T1.CreditPayment, T2.PluNo, T2.PluName, T2.Price "
                      "FROM Receipt T1 JOIN ReceiptDetails T2 ON T1.ReceiptNo = T2.ReceiptNo ORDER BY T1.ReceiptNo;";
    
    FILE *fp = fopen("report/details.txt", "w");
    if (fp == NULL)
    {
        perror("File creation error!\n");
        return;
    }
    
    fprintf(fp, "[\n");
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK)
    {
        int currentReceiptNo = -1;
        int firstReceipt = 1;
        
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int receiptNo = sqlite3_column_int(stmt, 0);
            int receiptTotal = sqlite3_column_int(stmt, 1);
            int cashPayment = sqlite3_column_int(stmt, 2);
            int creditPayment = sqlite3_column_int(stmt, 3);
            int pluNo = sqlite3_column_int(stmt, 4);
            const unsigned char *pluName = sqlite3_column_text(stmt, 5);
            int price = sqlite3_column_int(stmt, 6);
            
            if (receiptNo != currentReceiptNo)
            {
                if (!firstReceipt)
                {
                    fprintf(fp, "\n      ]\n  },\n");
                }
                
                fprintf(fp, "  {\n");
                fprintf(fp, "    \"ReceiptNo\": %d,\n", receiptNo);
                fprintf(fp, "    \"ReceiptTotal\": %d,\n", receiptTotal);
                fprintf(fp, "    \"CashPayment\": %d,\n", cashPayment);
                fprintf(fp, "    \"CreditPayment\": %d,\n", creditPayment);
                fprintf(fp, "    \"ReceiptDetails\": [\n");
            }
            else
            {
                fprintf(fp, ",\n");
            }
            
            fprintf(fp, "      {\n");
            fprintf(fp, "        \"PluNo\": %d,\n", pluNo);
            fprintf(fp, "        \"PluName\": \"%s\",\n", pluName);
            fprintf(fp, "        \"Price\": %d\n", price);
            fprintf(fp, "      }");
            
            currentReceiptNo = receiptNo;
            firstReceipt = 0;
        }
        
        if (!firstReceipt)
        {
            fprintf(fp, "\n      ]\n  }");
        }
        
        sqlite3_finalize(stmt);
    }
    else
    {
        fprintf(stderr, "SQL Error: %s\n", sqlite3_errmsg(db));
    }
    
    fprintf(fp, "\n]\n");
    fclose(fp);
    printf("Details file has succesfully created.\n");
}
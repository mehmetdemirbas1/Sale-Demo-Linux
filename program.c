#include "program.h"

int rc;
sqlite3_stmt *stmt;
char line[256];

int createTables(sqlite3 *db, const char *createTable) 
{
    

    char *err_msg = 0;
    int rc = sqlite3_exec(db,createTable,0,0,&err_msg);

    if(rc != SQLITE_OK)
    {

        fprintf(stderr, "SQL Error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return -1;
    }
    else
    {
        fprintf(stdout, "Tables succesfuly created\n");
    }

    return 0;
}
int insert_data(sqlite3 *db, const char *sql, const char *value1, int value2)
{
    
     int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

     if (rc != SQLITE_OK)
     {
        fprintf(stderr, "Ready Error: %s\n", sqlite3_errmsg(db));
        return -1;
     }


    sqlite3_bind_text(stmt, 1, value1, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, value2);

    rc = sqlite3_step(stmt);


    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "Insertion Error: %s\n ",sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    printf("Insertion successful\n");
    return 0;
    
}
void syncProduct(sqlite3 *db)
{
    int count = 0;
    FILE *fp = fopen("program/product.txt", "r");
    if (fp == NULL)
    {
        fprintf(stderr, "product.txt file is not opened\n");
        sqlite3_close(db);
    }
    const char *sql_upsert = "INSERT OR REPLACE INTO Product (PluNo, Name, Price, VatRate, Barcode) VALUES (?,?,?,?,?);";
    rc = sqlite3_prepare_v2(db, sql_upsert, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr,"product.txt file cannot opened.\n");
        sqlite3_close(db);
    }
    while (fgets(line, sizeof(line), fp))
    {
        int pluNo, price, vatRate;
        char name[50];
        char barcode[50];

        char *token = strtok(line, ",");
        int fieldIndex = 0;

        while (token) 
        {
            while (*token == ' ') token++; 
            switch (fieldIndex) 
            {
                case 0: pluNo = atoi(token); break;
                case 1: strcpy(name, token); break;
                case 2: price = atoi(token); break;
                case 3: vatRate = atoi(token); break;
                case 4: strcpy(barcode, token); barcode[strcspn(barcode, "\n")] = 0; break;
            }
            fieldIndex++;
            token = strtok(NULL, ",");
        }
        printf(" %10d | %10s | %10d | %10d | %10s\n", pluNo, name, price, vatRate, barcode);

        if (sqlite3_prepare_v2(db, sql_upsert, -1, &stmt, NULL) == SQLITE_OK) 
        {
            sqlite3_bind_int(stmt, 1, pluNo);
            sqlite3_bind_text(stmt, 2, name, -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 3, price);
            sqlite3_bind_int(stmt, 4, vatRate);
            sqlite3_bind_text(stmt, 5, barcode, -1, SQLITE_TRANSIENT);

            if (sqlite3_step(stmt) == SQLITE_DONE) 
            {
                count++;
            } 
            else 
            {
                printf("Error inserting data: %s\n", sqlite3_errmsg(db));
            }

            sqlite3_finalize(stmt);
        } 
        else 
        {
            printf("Error preparing statement: %s\n", sqlite3_errmsg(db));
        }

    }
    fclose(fp);
    printf("\t%d Record Success\n", count);
}
void prgmProcess(sqlite3 *db)
{
    
    
    char choice = 0;
    printf("============ PROGRAM MENU ============\n");
    printf("1 - Sync Product\n");
    printf("2 - Back to Main\n");
    printf(">");
    scanf("%s",&choice);

    switch (choice)
    {
        case '1':
            syncProduct(db);
            break;
        case '2':
            return;
            break;
        default:
            printf("Invalid choice\n");
            break;
    }
    
}
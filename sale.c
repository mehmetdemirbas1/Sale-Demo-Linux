#include "sale.h"
#include <string.h>
int receipt = 1;

void saleProcess(sqlite3 *db)
{
    char choice;
    printf("============ SALE MENU ============\n");
    printf("\t1 - Sale Product\n");
    printf("\t2 - Pay\n");
    printf("\t3 - Close Receipt\n");
    printf("\t4 - Back to Main\n");
    printf("\t>");
    scanf("%s",&choice);

    while (1)
    {
        switch (choice)
        {
        case '1':
            saleProduct(db,&receipt);          
            break;
        case '2':
            payment(db);
            break;
        case '3':
            printf("\tClose receipt bl bla bla bla\n");
            break;
        case '4':
            printf("\tReturning to the Main Menu");
            return;
            break;        
        default:
        printf("\tInvalid Choice!!!\n");
            break;
        }
    }
    
}

int getSingleIntValue(sqlite3 *db, const char *sql, int pluNo) 
{
    sqlite3_stmt *stmt;
    int result = -1;

 if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, pluNo);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int count = sqlite3_column_int(stmt, 0);
            if (count > 0) {
                result = 1; 
            }
        }
        sqlite3_finalize(stmt);
    } else {
        fprintf(stderr, "\tSQL Error: %s\n", sqlite3_errmsg(db));
    }

    return result;
}

void saleProduct(sqlite3 *db, int *currentReceiptNo_ptr)
{
    sqlite3_stmt *stmt = NULL;
    int userEnteredPluNo;
    int productCount;
    int success = 0; 
    int currentReceiptNo = *currentReceiptNo_ptr; 

    const char *sql_insert = "INSERT INTO ReceiptDetails (ReceiptNo, PluNo, PluName, Price) "
                             "SELECT ?, PluNo, Name, Price "
                             "FROM Product WHERE PluNo = ?;";

    printf("\t Enter Plu No:\n\t> ");
    scanf("%d", &userEnteredPluNo);

    productCount = getSingleIntValue(db, "SELECT COUNT(*) FROM Product WHERE PluNo = ?", userEnteredPluNo);

    if (1 == productCount)
    {
        printf("\tProduct is Found!!!!\n");
        do
        {
            if (sqlite3_prepare_v2(db, sql_insert, -1, &stmt, 0) != SQLITE_OK)
            {
                fprintf(stderr, "\tSQL Error: %s\n", sqlite3_errmsg(db));
                break;
            }

            sqlite3_bind_int(stmt, 1, currentReceiptNo);
            sqlite3_bind_int(stmt, 2, userEnteredPluNo);

            if (sqlite3_step(stmt) != SQLITE_DONE)
            {
                
                if (sqlite3_extended_errcode(db) == SQLITE_CONSTRAINT_UNIQUE)
                {
                    printf("\tCreating a new receipt\n");
                    
                    
                    currentReceiptNo = currentReceiptNo + 1; 
                    printf("\tNew Receipt number: %d\n", currentReceiptNo);
                    sqlite3_finalize(stmt); 
                    stmt =NULL;
                }
                else
                {
                    fprintf(stderr, "\tSQl adding error: %s\n", sqlite3_errmsg(db));
                    return saleProcess(db);
                }
            }
            else
            {
                printf("\tReceipt is succesfuly added\n");
                success = 1;
                return saleProcess(db);
            }

        } while (!success);

        *currentReceiptNo_ptr = currentReceiptNo; 
        if (stmt)
        {
            sqlite3_finalize(stmt);
        }
        
        
    }
    else
    {
        printf("\tError: No product with the entered PLU number was found.\n");
        return saleProcess(db);
    }

    
}

void payment(sqlite3 *db)
{
    char choice = 0;
    int amount = 0;

    printf("\t============ Enter Payment Type ============\n");
    printf("\t1 - Cash\n");
    printf("\t2 - Credit\n");
    printf("\t>");
    scanf("%s", &choice);

    if (choice == '1')
    {
        printf("\tEnter Amount\n");
        printf("\t>");
        scanf("%d",&amount);


        return saleProcess(db);
    }
    else if(choice == '2')
    {
        printf("\tEnter Amount\n");
        printf("\t>");
        scanf("%d",&amount);


        return saleProcess(db);
    }
    else
    {
        printf("\tInvalid Number!! pls try again\n");
    }
    
    
}




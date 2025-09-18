#include "sale.h"

static int currentReceiptNo = 0;
static int receiptTotal = 0;

int getLastReceiptNo(sqlite3 *db) 
{
    sqlite3_stmt *stmt;
    int lastReceiptNo = 0;
    const char *sql = "SELECT MAX(ReceiptNo) FROM Receipt;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) 
    {
        if (sqlite3_step(stmt) == SQLITE_ROW) 
        {
            lastReceiptNo = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    return lastReceiptNo;
}

int getReceiptTotal(sqlite3 *db, int receiptNo) 
{
    sqlite3_stmt *stmt;
    int total = 0;
    const char *sql = "SELECT SUM(Price) FROM ReceiptDetails WHERE ReceiptNo = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) 
    {
        sqlite3_bind_int(stmt, 1, receiptNo);
        if (sqlite3_step(stmt) == SQLITE_ROW) 
        {
            total = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    } 
    else 
    {
        fprintf(stderr, "SQL Error: %s\n", sqlite3_errmsg(db));
    }
    return total;
}


void saleProcess(sqlite3 *db) 
{
    char choice = 0;

    if (currentReceiptNo == 0) 
    {
        currentReceiptNo = getLastReceiptNo(db) + 1;
        printf("Starting new receipt. Receipt No: %d\n", currentReceiptNo);
    }

    printf("============ SALE MENU ============\n");
    printf("1 - Sale Product\n");
    printf("2 - Pay\n");
    printf("3 - Close Receipt\n");
    printf("4 - Back to Main\n");
    printf(">");
    scanf(" %c", &choice);

    switch (choice) 
    {
        case '1':
            saleProduct(db);
            break;
        case '2':
            payment(db);
            break;
        case '3':
            closeReceipt(db);
            break;
        case '4':
            return;
        default:
            printf("Invalid Choice!!!\n");
            break;
    }
    saleProcess(db); 
}

void saleProduct(sqlite3 *db) 
{
    sqlite3_stmt *stmt = NULL;
    int userEnteredPluNo = 0;
    int productPrice = 0;
    char *productName = NULL;

    printf("Enter Plu Number:\n>");
    scanf("%d", &userEnteredPluNo);

    
    const char *sql_select_product = "SELECT Name, Price FROM Product WHERE PluNo = ?;";
    if (sqlite3_prepare_v2(db, sql_select_product, -1, &stmt, 0) == SQLITE_OK) 
    {
        sqlite3_bind_int(stmt, 1, userEnteredPluNo);

        if (sqlite3_step(stmt) == SQLITE_ROW) 
        {
            productName = strdup((const char *)sqlite3_column_text(stmt, 0));
            productPrice = sqlite3_column_int(stmt, 1);
            printf("Product is Found!!!\n");
            sqlite3_finalize(stmt);

           
            const char *sql_insert = "INSERT INTO ReceiptDetails (ReceiptNo, PluNo, PluName, Price) VALUES (?, ?, ?, ?);";
            sqlite3_stmt *insert_stmt;
            if (sqlite3_prepare_v2(db, sql_insert, -1, &insert_stmt, 0) == SQLITE_OK) 
            {
                sqlite3_bind_int(insert_stmt, 1, currentReceiptNo);
                sqlite3_bind_int(insert_stmt, 2, userEnteredPluNo);
                sqlite3_bind_text(insert_stmt, 3, productName, -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(insert_stmt, 4, productPrice);

                if (sqlite3_step(insert_stmt) == SQLITE_DONE) 
                {
                    printf("Product added successfully!\n");
                } 
                else
                {
                    fprintf(stderr, "SQL adding error: %s\n", sqlite3_errmsg(db));
                }
                sqlite3_finalize(insert_stmt);
            } 
            else 
            {
                fprintf(stderr, "SQL Error: %s\n", sqlite3_errmsg(db));
            }
        } 
        else 
        {
            printf("Product has not been found.\n");
            sqlite3_finalize(stmt);
        }
    } 
    else 
    {
        fprintf(stderr, "SQL Error: %s\n", sqlite3_errmsg(db));
    }
    if (productName) free(productName);
}

void payment(sqlite3 *db) 
{
    char choice;
    int amount;
    sqlite3_stmt *stmt = NULL;
    int hasReceiptRecord = 0;
    int currentCashPaid = 0;
    int currentCreditPaid = 0;

    receiptTotal = getReceiptTotal(db, currentReceiptNo);

    if (receiptTotal == 0) 
    {
        printf("No products added to this receipt.\n");
        return;
    }

    
    const char *sql_check = "SELECT COUNT(*) FROM Receipt WHERE ReceiptNo = ?;";
    if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, 0) == SQLITE_OK) 
    {
        sqlite3_bind_int(stmt, 1, currentReceiptNo);
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_int(stmt, 0) > 0) 
        {
            hasReceiptRecord = 1;
        }
        sqlite3_finalize(stmt);
    } 
    else 
    {
        fprintf(stderr, "SQL Error: %s\n", sqlite3_errmsg(db));
        return;
    }

    if (hasReceiptRecord) 
    {
        
        const char *sql_get_paid = "SELECT CashPayment, CreditPayment FROM Receipt WHERE ReceiptNo = ?;";
        if (sqlite3_prepare_v2(db, sql_get_paid, -1, &stmt, 0) == SQLITE_OK) 
        {
            sqlite3_bind_int(stmt, 1, currentReceiptNo);
            if (sqlite3_step(stmt) == SQLITE_ROW) 
            {
                currentCashPaid = sqlite3_column_int(stmt, 0);
                currentCreditPaid = sqlite3_column_int(stmt, 1);
            }
            sqlite3_finalize(stmt);
        } 
        else
        {
            fprintf(stderr, "SQL Error: %s\n", sqlite3_errmsg(db));
            return;
        }
    }
    
    int totalPaid = currentCashPaid + currentCreditPaid;
    int remainingAmount = receiptTotal - totalPaid;

    while (remainingAmount > 0) {
        printf("\n============ Payment Selection ============\n");
        printf("Current Receipt Total: %d\n", receiptTotal);
        printf("Total Paid: %d\n", totalPaid);
        printf("Remaining Amount: %d\n", remainingAmount);
        printf("Enter Payment Type: 1-Cash 2-Credit 3-Cancel\n>");
        scanf(" %c", &choice);
        
        if (choice == '3') 
        {
            printf("Payment canceled.\n");
            return;
        }

        if (choice != '1' && choice != '2') 
        {
            printf("Invalid selection! Please try again.\n");
            continue;
        }

        printf("Enter Amount:\n>");
        scanf("%d", &amount);

        if (amount > remainingAmount) 
        {
            printf("Entered amount is more than remaining amount. Please try again.\n");
            continue;
        }
        if (amount < 0)
        {
            printf("you canot pay negaive value .\n");
            continue;
        }

        if (choice == '1') 
        {
            currentCashPaid += amount;
        } 
        else 
        {
            currentCreditPaid += amount;
        }

        totalPaid += amount;
        remainingAmount = receiptTotal - totalPaid;

        printf("Payment successful! Remaining: %d\n", remainingAmount);
    }

    
    if (hasReceiptRecord) 
    {
        const char *sql_update_final = "UPDATE Receipt SET ReceiptTotal = ?, CashPayment = ?, CreditPayment = ? WHERE ReceiptNo = ?;";
        if (sqlite3_prepare_v2(db, sql_update_final, -1, &stmt, 0) == SQLITE_OK) 
        {
            sqlite3_bind_int(stmt, 1, receiptTotal);
            sqlite3_bind_int(stmt, 2, currentCashPaid);
            sqlite3_bind_int(stmt, 3, currentCreditPaid);
            sqlite3_bind_int(stmt, 4, currentReceiptNo);

            if (sqlite3_step(stmt) != SQLITE_DONE) 
            {
                fprintf(stderr, "Final update failed: %s\n", sqlite3_errmsg(db));
            }
            sqlite3_finalize(stmt);
        } 
        else 
        {
            fprintf(stderr, "SQL Error: %s\n", sqlite3_errmsg(db));
        }
    } 
    else 
    {

        const char *sql_insert = "INSERT INTO Receipt (ReceiptNo, ReceiptTotal, CashPayment, CreditPayment) VALUES (?, ?, ?, ?);";
        if (sqlite3_prepare_v2(db, sql_insert, -1, &stmt, 0) == SQLITE_OK) 
        {
            sqlite3_bind_int(stmt, 1, currentReceiptNo);
            sqlite3_bind_int(stmt, 2, receiptTotal);
            sqlite3_bind_int(stmt, 3, currentCashPaid);
            sqlite3_bind_int(stmt, 4, currentCreditPaid);

            if (sqlite3_step(stmt) != SQLITE_DONE) 
            {
                fprintf(stderr, "Initial insert failed: %s\n", sqlite3_errmsg(db));
            }
            sqlite3_finalize(stmt);
        } 
        else 
        {
            fprintf(stderr, "SQL Error: %s\n", sqlite3_errmsg(db));
        }
    }
    
    printf("Payment for this receipt is completed.\n");
    
}

void closeReceipt(sqlite3 *db) 
{
    printf("Receipt Successfully Closed.\n");
    currentReceiptNo++;
    receiptTotal = 0;
    
}
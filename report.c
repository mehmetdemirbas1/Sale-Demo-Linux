#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "report.h"

void reptProcess(sqlite3 *db)
{

    char choice = 0;
    printf("\t============ REPORT MENU ============\n");
    printf("\t1 - Summary\n");
    printf("\t2 - Detail\n");
    printf("\t3 - Back to Main\n");
    printf("\t>");
    scanf("%s",&choice);

    while (1)
    {
        switch (choice)
        {
            case '1':
                printf("Summary bla bla bla\n");
                break;
            case '2':
                printf("Detail bla bla bla\n");
                break;
            case '3':
                return;
                break;    
            default:
                printf("\tInvalid Number pls try again!\n");
                break;
        }
    }
    
    
}
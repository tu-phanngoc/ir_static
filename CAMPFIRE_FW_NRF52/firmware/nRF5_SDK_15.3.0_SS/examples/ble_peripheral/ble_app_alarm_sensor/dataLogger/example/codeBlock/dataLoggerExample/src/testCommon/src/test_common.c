/*****************************************************************************
* Copyright 2016 NXP
*****************************************************************************/

/**
*
* @file testMenu.h
*
* @author
*
* @date
*
* @brief
*
*
******************************************************************************/

/*****************************************************************************
* INCLUDES
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "test_common.h"

/*****************************************************************************
* DEFINES
*****************************************************************************/

/*****************************************************************************
* TYPE DEFINITIONS
*****************************************************************************/

/*****************************************************************************
* INTERNAL TYPE DEFINITIONS
*****************************************************************************/

/***************** ************************************************************
* EXPORTED VARIABLES
*****************************************************************************/


/*****************************************************************************
* INTERNAL VARIABLES
*****************************************************************************/
bool exitMenu = false;
bool goBack = false;

void exitMenuFunction(void)
{
    exitMenu = true;
    goBack = true;
}

void test_menu_display(menu_item_t *menu, uint8_t size)
{
    uint8_t choice, ch;
    uint8_t i;

    do
    {
        choice = 'a';
        for (i=0; i< size; i++)
        {
            PRINTF("\n%c. %s", choice++, menu[i].menu_description);
        }
        PRINTF("\n  Select option: ");
        do
        {
            ch = GETCHAR();
            choice = ch - 'a';
            if(choice >= size)
            {
                PRINTF("\n This opt not supported!.Select again: ");
            }
        } while(choice >= size);

        do
        {
            PRINTF("\n\n  %s is selected\r\n", menu[choice].menu_description);
            if(menu[choice].menu_function != NULL)
            {
                menu[choice].menu_function();
            }
        } while (exitMenu == false);
    } while (goBack == false);
}

void test_end_handler(void)
{
    char ch;
    PRINTF("\n  Test case finished");
    PRINTF("\n  Press 'r': again, 'q':exit, 'b':back : ");
    do
    {
        ch = GETCHAR();
        switch (ch)
        {
            case 'r':
            {
                exitMenu = false;
                break;
            }
            case 'q':
            {
                exitMenu = true;
                goBack = true;
                break;
            }
            case 'b':
            {
                exitMenu = true;
                goBack = false;
                break;
            }
            default:
                break;
        }
    } while ((ch != 'r') && (ch != 'q') && (ch != 'b'));
    PRINTF("\n");
}
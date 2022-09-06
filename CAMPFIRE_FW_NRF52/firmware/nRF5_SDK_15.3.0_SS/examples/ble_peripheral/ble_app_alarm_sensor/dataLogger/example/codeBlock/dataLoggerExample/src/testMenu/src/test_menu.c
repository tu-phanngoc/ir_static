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
#include "test_menu.h"
#include "test_common.h"
#include "dataLogger_Config.h"
#include "dataLogger_Interface.h"

/*****************************************************************************
* DEFINES
*****************************************************************************/

/*****************************************************************************
* TYPE DEFINITIONS
*****************************************************************************/

/*****************************************************************************
* INTERNAL TYPE DEFINITIONS
*****************************************************************************/

/*****************************************************************************
* EXPORTED VARIABLES
*****************************************************************************/

/*****************************************************************************
* INTERNAL VARIABLES
*****************************************************************************/
void initDataLog(void);
void saveDataLog(void);
void getDataLog(void);
void popDataLog(void);
void getRingIndexInfo(void);
void closeExternalFlash(void);
void openExternalFlash(void);

/* Sender menu */
menu_item_t mainMenu[] =
{
    {initDataLog, "Init Data Log"},
    {saveDataLog, "Save data log"},
    {getDataLog, "Get Data Log"},
    {popDataLog, "Pop Data Log"},
    {getRingIndexInfo, "Get ring index info"},
    {openExternalFlash, "Open external Flash"},
    {closeExternalFlash, "Close external Flash"},
    {exitMenuFunction,"Exit"}
};

dataExample_t exampleData;

/*****************************************************************************
* INTERNAL FUNCTION DECLARATIONS
*****************************************************************************/

/*****************************************************************************
* FUNCTIONS
*****************************************************************************/

/************************************* Common functions ****************************/
void initDataLog(void)
{
    dataLogger_Init();
    test_end_handler();
}

void saveDataLog(void)
{
    dataLoggerStatus_t retVal = DATA_LOGGER_STT_SUCCESS;
    static uint32_t saveNo = 0;

    exampleData.head = 0xAAAAAAAA;
    exampleData.count = saveNo++;
    exampleData.data = 0x31323334;
    exampleData.tail = 0x55555555;

    dataLogger_Save(EXAMPLE1_TYPE, &exampleData);

    if(DATA_LOGGER_STT_SUCCESS != retVal)
    {
        printf("Save data log FAIL\n");
    }

    test_end_handler();
}

void getDataLog(void)
{
    dataExample_t exampleDataTmp;
    dataLoggerStatus_t retVal = DATA_LOGGER_STT_SUCCESS;

    retVal = dataLogger_Get(EXAMPLE1_TYPE, &exampleDataTmp);

    if(DATA_LOGGER_STT_SUCCESS != retVal)
    {
        printf("Get data log FAIL\n");
    }
    else
    {
        printf("Read data: \n");
        printf("    Header: 0x%x\n", exampleDataTmp.head);
        printf("    Count:    %d\n", exampleDataTmp.count);
        printf("    Data:   0x%x\n", exampleDataTmp.data);
        printf("    Tail:   0x%x\n", exampleDataTmp.tail);
    }
    test_end_handler();
}

void popDataLog(void)
{
    dataExample_t exampleDataTmp;
    dataLoggerStatus_t retVal = DATA_LOGGER_STT_SUCCESS;

    retVal = dataLogger_Pop(EXAMPLE1_TYPE, &exampleDataTmp);

    if(DATA_LOGGER_STT_SUCCESS != retVal)
    {
        printf("Pop data log FAIL\n");
    }
    else
    {
        printf("Read data: \n");
        printf("    Header: 0x%x\n", exampleDataTmp.head);
        printf("    Count:    %d\n", exampleDataTmp.count);
        printf("    Data:   0x%x\n", exampleDataTmp.data);
        printf("    Tail:   0x%x\n", exampleDataTmp.tail);
    }

    test_end_handler();
}

void getRingIndexInfo(void)
{
    test_end_handler();
}

extern FILE* pExternalFlash;

void openExternalFlash(void)
{
    pExternalFlash = fopen ("ExternalFlash.bin", "r+b");
    test_end_handler();
}

void closeExternalFlash(void)
{
    fclose(pExternalFlash);
    test_end_handler();
}

/************************************* CANTP retransmit sender menu ****************************/

void main_menu_display(void)
{
    test_menu_display(mainMenu, sizeof(mainMenu)/sizeof(menu_item_t));
}

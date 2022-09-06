/**************************************************************************************************
 *
 * Copyright (C)
 *
 *************************************************************************************************/
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "dataLogger_Config.h"
#include "dataLogger_UserPort.h"
#include "dataLogger_IT.h"
#include "dataLogger_IT_TC.h"
/*******************************************************************************
 * This file to test functions
 ******************************************************************************/
/*******************************************************************************
 * Variables
 ******************************************************************************/
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void DL_IT_001_SaveGetVerify_DataStr8(void)
{
    DL_IT_001_SaveGetVerify(DATA_LOG_TYPE_8BYTE);
}
void DL_IT_001_SaveGetVerify_DataStr9(void)
{
    DL_IT_001_SaveGetVerify(DATA_LOG_TYPE_9BYTE);
}
void DL_IT_001_SaveGetVerify_DataStr10(void)
{
    DL_IT_001_SaveGetVerify(DATA_LOG_TYPE_10BYTE);
}
void DL_IT_001_SaveGetVerify_DataStr11(void)
{
    DL_IT_001_SaveGetVerify(DATA_LOG_TYPE_11BYTE);
}

void DL_IT_002_SavePopVerify_Str8(void)
{
    DL_IT_002_SavePopVerify(DATA_LOG_TYPE_8BYTE);
}
void DL_IT_002_SavePopVerify_Str9(void)
{
    DL_IT_002_SavePopVerify(DATA_LOG_TYPE_9BYTE);
}
void DL_IT_002_SavePopVerify_Str10(void)
{
    DL_IT_002_SavePopVerify(DATA_LOG_TYPE_10BYTE);
}
void DL_IT_002_SavePopVerify_Str11(void)
{
    DL_IT_002_SavePopVerify(DATA_LOG_TYPE_11BYTE);
}

void DL_IT_003_SaveOneRoundPopOneRound_Str8(void)
{
    DL_IT_003_SaveOneRoundPopOneRound(DATA_LOG_TYPE_8BYTE);
}
void DL_IT_003_SaveOneRoundPopOneRound_Str9(void)
{
    DL_IT_003_SaveOneRoundPopOneRound(DATA_LOG_TYPE_9BYTE);
}
void DL_IT_003_SaveOneRoundPopOneRound_Str10(void)
{
    DL_IT_003_SaveOneRoundPopOneRound(DATA_LOG_TYPE_10BYTE);
}
void DL_IT_003_SaveOneRoundPopOneRound_Str11(void)
{
    DL_IT_003_SaveOneRoundPopOneRound(DATA_LOG_TYPE_11BYTE);
}

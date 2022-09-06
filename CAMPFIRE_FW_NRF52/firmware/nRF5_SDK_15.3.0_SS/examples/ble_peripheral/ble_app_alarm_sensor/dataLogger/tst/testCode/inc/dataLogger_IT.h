/**************************************************************************************************
 *
 * Copyright (C) 2019
 *
 *************************************************************************************************/
#include <stdbool.h>
#include <unity.h>
#ifndef __DATALOGGER_IT_H__
#define __DATALOGGER_IT_H__

/**
 * @brief This function is for testing saveDataLog, getDataLog and verify if data is correct
 * Repeat this iteration to make sure wrap around the maximum record 2 times
 */
void DL_IT_001_SaveGetVerify(dbDataType_t dataType);

/**
 * @brief This function is for testing saveDataLog, popDataLog and verify if data is correct
 * Repeat this iteration to make sure wrap around the maximum record 2 times
 */
void DL_IT_002_SavePopVerify(dbDataType_t dataType);

/**
 * @brief This function is for testing saveDataLog one round (max data records on pool)
 * Then pop all datalogs and verify data
 */
void DL_IT_003_SaveOneRoundPopOneRound(dbDataType_t dataType);
#endif /* __DATALOGGER_IT_H__ */

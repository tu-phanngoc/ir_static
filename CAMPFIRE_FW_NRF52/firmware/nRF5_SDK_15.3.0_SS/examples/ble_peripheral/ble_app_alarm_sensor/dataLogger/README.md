***(c)  All rights reserved.***

*This document include 2 parts: Part A - **INSTRUCTION** and Part B - **EXAMPLE** to use **dataLogger** functions.*

**Part A - DATA LOGGER INSTRUCTION**

#dataLogger_Interface Instruction

Over View:
**This source include files:**
* **dataLogger_Config.c** (*UserConfig*): User config MemoryDataInfoTable
* **dataLogger_Config.h** (*UserConfig*): User config for Data struct, base address and Memory layout
* **dataLogger_Interface.c**(*ReadOnly*): contain  functions to save and read memory.
* **dataLogger_Interface.h**(*ReadOnly*): contain DB Memory functions prototype.
* **dataLogger_UserPort.c** (*User Modify*): User modify function to develop Driver for memory
* **dataLogger_UserPort.h** (*User Modify*): User modify function to develop Driver for memory

**1.Config:Declare data structure**

**1.1.Define**

*In file dataLogger_Config.h CONFIG for MEMORY*
* MAX_DB_MEMORY_DATA_TYPE_CONFIG: Maximum of Data type which user need to save in mememory.
* LOGRING_INDEX_MAX_SIZE: Maximum size of Index which to save Ringlog index, it must be 2*MEMORY_SECTOR_SIZE
* MEMORY_SECTOR_SIZE: The memory sector size, default BLOCK4K
* MAX_OF_STRUCT_DATA_LOG: User must calc this value, MAX_OF_STRUCT_DATA_LOG = Max of datatype + number byte Dummy + 2byte CRC
*Note: (MAX_OF_STRUCT_DATA_LOG%4)==0 => User must count number byte dummy*
* dataLogger_GetLatest_PARAM_BASE_ADDR: base address of memory, it depend on FLASH.
* [DATA_TYPE]_INDEX_SECTOR: Sector to save index of RingLog (size = 1 sector)
* [DATA_TYPE]_DATA_SECTOR: Sector to save Data structure
* [DATA_TYPE]_NUMBER_SECTOR:  Number sector to save Data (**It must be more than 2 sectors**)

*Example:*
```
#define MAX_DB_MEMORY_DATA_TYPE_CONFIG      4
#define MEMORY_SECTOR_SIZE                  1024
#define NUMBER_SECTOR_INDEX                 2
#define LOGRING_INDEX_MAX_SIZE              (NUMBER_SECTOR_INDEX * MEMORY_SECTOR_SIZE) /*Default 2 sector 4K*/
#define MAX_OF_STRUCT_DATA_LOG              512 /*Max of struct data record + Dummy + 2byteCRC : (Must %4 == 0)*/
#define MEMORY_BASE_ADDR                    0x00000000 /*Depend on FLASH*/


#define DL_TC_8B_INDEX_SECTOR           0 /*Default: Use 2 sector*/
#define DL_TC_8B_DATA_SECTOR            2
#define DL_TC_8B_DATA_NUMBER_SECTOR     3 /*Must be use more than 2 sector*/
```

**1.2. Declare Enum**

*In file dataLogger_Config.h Add enum datatype in **dbDataType_t***

**Note: User must define Index DataType sync with index on struct dataLoggerConfigTable**

*Example:*
```
typedef enum
{
    DATA_LOG_TYPE_8BYTE,

/*User add more type here*/
} dbDataType_t;
```

**1.3. Declare DataStructure**

*In file dataLogger_Config.h declare data structure, this is optional. User can declare by depend on their feature*

*Example:*
```
typedef struct
{/*Demo data structure*/
    uint16_t data1;
    uint16_t data2;
    uint16_t data3;
    uint16_t data4;
} dataAign8b_t;
```

**1.4. Config Table Info**

*In file dataLogger_Config.c config memory table info*

*Example:*
```
const dataLoggerConfig_t dataLoggerConfigTable[MAX_DB_MEMORY_DATA_TYPE_CONFIG] =
{
    {DATA_LOG_TYPE_8BYTE       ,DL_TC_8B_INDEX_SECTOR       ,DL_TC_8B_DATA_SECTOR  ,sizeof(dataAign8b_t)    , DL_TC_8B_DATA_NUMBER_SECTOR},
    /* user add more here */
};
```

**2.Input:Modify Function**

**2.1.CRCchecksum Function**

*In file dataLogger_UserPort.c user can modify function dataLoggerCheckSumCRC16 to calc CRCchecksum. This is optional*
**2.2.Driver Function**

*Provide 3 functions follow driver:*
* dataLoggerDriver_Read : to read memory
* dataLoggerDriver_Write : to write memory (able to write 4 byte data)
* dataLoggerDriver_Erase : to erase memory (erase follow sector)

*Note: Do not modify prototype format*

*Example:*
```
dataLoggerStatus_t dataLoggerDriver_Read(uint32_t Address, uint32_t Lengh, uint8_t* DataBuffer);
dataLoggerStatus_t dataLoggerDriver_Read(uint32_t Address, uint32_t Lengh, uint8_t* DataBuffer)
{
    //Develop your code here
    return DB_MEM_STT_SUCCESS;
};
dataLoggerStatus_t dataLoggerDriver_Write(uint8_t* DataBuffer,uint32_t Address, uint32_t Lengh);
dataLoggerStatus_t dataLoggerDriver_Write(uint8_t* DataBuffer,uint32_t Address, uint32_t Lengh)
{
    //Develop your code here
    return DB_MEM_STT_SUCCESS;
};
dataLoggerStatus_t dataLoggerDriver_Erase(uint32_t sectorIndex, uint32_t numberSector);
dataLoggerStatus_t dataLoggerDriver_Erase(uint32_t sectorIndex, uint32_t numberSector)
{
    //Develop your code here
    return DB_MEM_STT_SUCCESS;
};
```

**3.List functions**

**3.1.DB Memory Functions**

* dataLogger_Init : Initialize a log record
* dataLogger_Save :  Save data record
* dataLogger_Get :  Get latest data from Memory
* dataLogger_Pop : Get pop data valid data record: get oldest and shift tail
* dataLogger_GetNumberSaved: get number of data saved in memory
* dataLogger_GetLatestValid: This function to get data valid data record, if fail, get previous value.

*Note*:

*Input*: 

dbDataType_t dbDataType, void* pdbDataItem
* dbDataType : data type enum
* pdbDataItem : pointer of data structure

*Output*: 

```
typedef enum
{
    DATA_LOGGER_STT_NONE,        /**< return when memory is empty*/
    DATA_LOGGER_STT_SUCCESS,     /**< return when process init, read, write, get, pop is successful*/
    DATA_LOGGER_STT_FAIL,        /**< return when process init, read, write, get, pop is failure*/
    DATA_LOGGER_STT_NON_INIT,    /**< return when datatype has not initialized yet*/
    DATA_LOGGER_STT_MAX = 0xFF   /**< Max size of status enum*/
} dataLoggerStatus_t;
```


**3.2.Prototype**

*In file dataLogger_Interface.h*
```
dataLoggerStatus_t dataLogger_Init(dbDataType_t dbDataType);
dataLoggerStatus_t dataLogger_Save(dbDataType_t dbDataType, void* pdbDataItem);  
dataLoggerStatus_t dataLogger_Get(dbDataType_t dbDataType, void* pdbDataItem);
dataLoggerStatus_t dataLogger_Pop(dbDataType_t dbDataType, void* pdbDataItem);
dataLoggerStatus_t dataLogger_GetLatestValid(dbDataType_t dbDataType, void* pdbDataItem);
uint32_t dataLogger_GetNumberSaved(dbDataType_t dbDataType);
```


**Part B: EXAMPLE TO USE DATA LOGGER**

**1: Config**

*In file dataLogger_Config.h*
```
typedef enum
{
    EXAMPLE1_TYPE
/*User add more type here*/
} dbDataType_t;
typedef struct
{
    uint32_t head;
    uint32_t count;
    uint32_t data;
    uint32_t tail;
} dataExample_t;

#define MAX_DB_MEMORY_DATA_TYPE_CONFIG      1

#define MEMORY_SECTOR_SIZE                  128
#define NUMBER_SECTOR_INDEX                 2
#define LOGRING_INDEX_MAX_SIZE              (NUMBER_SECTOR_INDEX * MEMORY_SECTOR_SIZE) /*Default 2 sector 4K*/
#define MAX_OF_STRUCT_DATA_LOG              32 /*Max of struct data record + Dummy + 2byteCRC : (Must %4 == 0)*/
#define MEMORY_BASE_ADDR                    0x00000000 /*Depend on FLASH*/

#define EXAMPLE_DB_MEM_INDEX_SECTOR           0 /*Default: Use 2 sector*/
#define EXAMPLE_DB_MEM_DATA_SECTOR            2
#define EXAMPLE_DB_MEM_DATA_NUMBER_SECTOR     2 /*Must be use more than 2 sector*/
```

*In file dataLogger_Config.c*
```
const dataLoggerConfig_t dataLoggerConfigTable[MAX_DB_MEMORY_DATA_TYPE_CONFIG] =
{
    {EXAMPLE1_TYPE       ,EXAMPLE_DB_MEM_INDEX_SECTOR       ,EXAMPLE_DB_MEM_DATA_SECTOR  ,sizeof(dataExample_t)    , EXAMPLE_DB_MEM_DATA_NUMBER_SECTOR},
    /* user add more here */
};
```
**2: Example for functions**


*Exampe Init*
```
void initDataLog(void)
{
    dataLogger_Init();
}
```


*Exampe Save*

```
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
    else
    {
        printf("Get data log SUCCESS\n");
    }
}
```


*Exampe Get*

```
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
        printf("Get data log SUCCESS\n");
    }
}
```


*Exampe Pop*

```
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
        printf("Pop data log SUCCESS\n");
    }
}
```


*Exampe Latest*

```
void getLatestValidLog(void)
{
    dataExample_t exampleDataTmp;
    dataLoggerStatus_t retVal = DATA_LOGGER_STT_SUCCESS;

    retVal = dataLogger_GetLatestValid(EXAMPLE1_TYPE, &exampleDataTmp);

    if(DATA_LOGGER_STT_SUCCESS != retVal)
    {
        printf("Get Latest data log FAIL\n");
    }
    else
    {
        printf("PGet Latest log SUCCESS\n");
    }
}
```


*Exampe Get Number of record in memory*

```
void getNumberRecord(void)
{
    dataExample_t exampleDataTmp;
    uint16_t retVal = 0;

    retVal = dataLogger_GetNumberSaved(EXAMPLE1_TYPE, &exampleDataTmp);
    printf("Number Record:    %d\n", retVal);
}
```

**2: Main function**

```
/*Example to use DataLogger Functions*/
void main(void)
{
    /*Init*/
    dataLogger_Init();
    /*Exampe Save*/
    saveDataLog();
    /*Exampe Get*/
    getDataLog();
    /*Exampe Pop*/
    popDataLog();
    /*Exampe Latest*/
    getLatestValidLog();
    /*Exampe Get Number of record in memory*/
    getNumberRecord();
}
```

#include <stdio.h>
#include <stdlib.h>
#include "external_flash.h"

extern FILE* pExternalFlash;
extern bool exitMenu;
int main()
{
    externalFlashErrorCode_t retVal = EXT_FLASH_SUCCESS;

    retVal = externalFlash_Init();

    if(EXT_FLASH_SUCCESS == retVal)
    {
        printf("Init external flash... OK.\n");

        while(1)
        {
            main_menu_display();
            if(exitMenu == true)
            {
                break;
            }
        }
    }
    else
    {
        printf("Init external flash... NOK.\n");
    }


    fclose(pExternalFlash);

    return 0;
}

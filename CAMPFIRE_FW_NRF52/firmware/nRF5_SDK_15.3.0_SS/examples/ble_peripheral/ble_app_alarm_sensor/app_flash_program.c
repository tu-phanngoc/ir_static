/******************************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 22 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN
******************************************************************************/
#include "app_flash_program.h"
#include "nrf_use_flash.h"
#include "nrf_log.h"
#include "crc32.h"
#include <string.h>
#include "app_scheduler.h"
#include "nrf_delay.h"

/** @brief  This variable reserves a codepage for bootloader specific settings,
 *          to ensure the compiler doesn't locate any code or variables at his location.
 */
#if defined (__CC_ARM )

    uint8_t  app_settings_buffer[CODE_PAGE_SIZE] __attribute__((at(APP_SYSTEM_CONFIG_ADDRESS)))
                                                   __attribute__((used));

#elif defined ( __GNUC__ )

    uint8_t app_settings_buffer[CODE_PAGE_SIZE] __attribute__ ((section(".bootloaderSettings")))
                                                  __attribute__((used));

#elif defined ( __ICCARM__ )

    __no_init __root uint8_t app_settings_buffer[CODE_PAGE_SIZE] @ APP_SYSTEM_CONFIG_ADDRESS;

#else

    #error Not a valid compiler/linker for m_dfu_settings placement.

#endif // Compiler specific



extern CONFIG_POOL sysCfg;

//lint -save -esym(551, flash_operation_pending)
static bool flash_operation_pending; // barrier for reading flash
//lint -restore

static dfu_flash_callback_t m_callback;


static void dfu_settings_write_callback(fs_evt_t const * const evt, fs_ret_t result)
{
    if (result == FS_SUCCESS)
    {
        flash_operation_pending = false;
    }
    if (m_callback != NULL)
    {
        m_callback(evt, result);
    }
}

static void delay_operation(void)
{
   nrf_delay_ms(100);
   app_sched_execute();
}

static void wait_for_pending(void)
{
    while (flash_operation_pending == true)
    {
        NRF_LOG_DEBUG("Waiting for other flash operation to finish.\r\n");
        delay_operation();
    }
}


static void wait_for_queue(void)
{
#ifdef BLE_STACK_SUPPORT_REQD
    while (fs_queue_is_full())
    {
        NRF_LOG_DEBUG("Waiting for available space on flash queue.\r\n");
        delay_operation();
    }
#endif
}


uint32_t app_flash_program_calculate_crc(void)
{
    // the crc is calculated from the sysCfg struct, except the crc itself and the init command
    return crc32_compute((uint8_t*)&sysCfg + 4, sizeof(CONFIG_POOL) - 4, NULL);
}


void app_flash_program_init(void)
{
    NRF_LOG_DEBUG("running app_flash_program_init\r\n");

    uint32_t crc;

    flash_operation_pending = false;

    // Copy the DFU settings out of flash and into a buffer in RAM.
    memcpy((void*)&sysCfg, &app_settings_buffer[0], sizeof(CONFIG_POOL));

    if(sysCfg.crc != 0xFFFFFFFF)
    {
        // CRC is set. Content must be valid
        crc = app_flash_program_calculate_crc();
        if(crc == sysCfg.crc)
        {
            return;
        }
    }

    // Reached if nothing is configured or if CRC was wrong
    NRF_LOG_DEBUG("!!!!!!!!!!!!!!! Resetting bootloader settings !!!!!!!!!!!\r\n");
    memset(&sysCfg, 0x00, sizeof(CONFIG_POOL));
    APP_ERROR_CHECK(app_flash_program_write(NULL));
}


ret_code_t app_flash_program_write(dfu_flash_callback_t callback)
{
    ret_code_t err_code = FS_SUCCESS;
    NRF_LOG_DEBUG("Erasing old settings at: 0x%08x\r\n", (uint32_t)&app_settings_buffer[0]);

    // Wait for any ongoing operation (because of multiple calls to app_flash_program_write)
    wait_for_pending();

    flash_operation_pending = true;
    m_callback = callback;

    do
    {
        wait_for_queue();

        // Not setting the callback function because ERASE is required before STORE
        // Only report completion on successful STORE.
        err_code = nrf_dfu_flash_erase((uint32_t*)&app_settings_buffer[0], 1, NULL);

    } while (err_code == FS_ERR_QUEUE_FULL);


    if (err_code != FS_SUCCESS)
    {
        NRF_LOG_ERROR("Erasing from flash memory failed.\r\n");
        flash_operation_pending = false;
        return NRF_ERROR_INTERNAL;
    }

    sysCfg.crc = app_flash_program_calculate_crc();

    NRF_LOG_DEBUG("Writing 0x%08x words\r\n", sizeof(CONFIG_POOL)/4);

    static CONFIG_POOL temp_dfu_settings;
    memcpy(&temp_dfu_settings, &sysCfg, sizeof(CONFIG_POOL));

    do
    {
        wait_for_queue();

        err_code = nrf_dfu_flash_store((uint32_t*)&app_settings_buffer[0],
                                       (uint32_t*)&temp_dfu_settings,
                                       sizeof(CONFIG_POOL)/4,
                                       dfu_settings_write_callback);

    } while (err_code == FS_ERR_QUEUE_FULL);

    if (err_code != FS_SUCCESS)
    {
        NRF_LOG_ERROR("Storing to flash memory failed.\r\n");
        flash_operation_pending = false;
        return NRF_ERROR_INTERNAL;
    }

    NRF_LOG_DEBUG("Writing settings...\r\n");
    return NRF_SUCCESS;
}


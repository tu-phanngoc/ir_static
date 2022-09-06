
/******************************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 22 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN
******************************************************************************/
#ifndef APP_FLASH_PROGRAM_H__
#define APP_FLASH_PROGRAM_H__

#include <stdint.h>
#include "app_util_platform.h"
#include "system_config.h"
#include "nrf_use_flash.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@brief Global DFU settings.
 *
 * @note Using this variable is not thread-safe.
 *
 */
extern CONFIG_POOL sysCfg;


/** @brief Function for writing DFU settings to flash.
 *
 * @param[in]   callback    Pointer to a function that is called after completing the write operation.
 *
 * @retval      NRF_SUCCESS         If the write process was successfully initiated.
 * @retval      NRF_ERROR_INTERNAL  If a flash error occurred.
 */
ret_code_t app_flash_program_write(dfu_flash_callback_t callback);


/** @brief Function for initializing the DFU settings module.
 */
void app_flash_program_init(void);


#ifdef __cplusplus
}
#endif

#endif // NRF_DFU_SETTINGS_H__

/**@} */

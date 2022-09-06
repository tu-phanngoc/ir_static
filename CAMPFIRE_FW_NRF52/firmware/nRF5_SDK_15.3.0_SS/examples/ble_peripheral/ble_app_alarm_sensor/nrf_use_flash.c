/**
 * Copyright (c) 2016 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include "nrf_use_flash.h"
#include "system_config.h"
#include "nrf_nvmc.h"
#include "nrf_log.h"

#include "nrf_fstorage.h"

#ifdef SOFTDEVICE_PRESENT
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_fstorage_sd.h"
#else
#include "nrf_drv_clock.h"
#include "nrf_fstorage_nvmc.h"
#endif

//#define NRF_USE_FLASH_DEBUG
#ifdef NRF_USE_FLASH_DEBUG		
#define USE_FLASH_PRINT			NRF_LOG_INFO
#else
#define USE_FLASH_PRINT
#endif

// Function prototypes
static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt);

NRF_FSTORAGE_DEF(nrf_fstorage_t fs_dfu_config) =
{
    /* Set a handler for fstorage events. */
    .evt_handler = fstorage_evt_handler,

    /* These below are the boundaries of the flash space assigned to this instance of fstorage.
     * You must set these manually, even at runtime, before nrf_fstorage_init() is called.
     * The function nrf5_flash_end_addr_get() can be used to retrieve the last address on the
     * last page of flash available to write data. */
    .start_addr = APP_SYSTEM_CONFIG_ADDRESS,
    .end_addr   = APP_SYSTEM_CONFIG_ADDRESS + CODE_PAGE_SIZE,
};

static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt)
{
    if (p_evt->result != NRF_SUCCESS)
    {
        USE_FLASH_PRINT("--> Event received: ERROR while executing an fstorage operation.");
        return;
    }

    switch (p_evt->id)
    {
        case NRF_FSTORAGE_EVT_WRITE_RESULT:
        {
            USE_FLASH_PRINT("--> Event received: wrote %d bytes at address 0x%x.",
                         p_evt->len, p_evt->addr);
        } break;

        case NRF_FSTORAGE_EVT_ERASE_RESULT:
        {
            USE_FLASH_PRINT("--> Event received: erased %d page from address 0x%x.",
                         p_evt->len, p_evt->addr);
        } break;

        default:
            break;
    }
}

static void print_flash_info(nrf_fstorage_t * p_fstorage)
{
    USE_FLASH_PRINT("========| flash info |========");
    USE_FLASH_PRINT("erase unit: \t%d bytes",      p_fstorage->p_flash_info->erase_unit);
    USE_FLASH_PRINT("program unit: \t%d bytes",    p_fstorage->p_flash_info->program_unit);
    USE_FLASH_PRINT("==============================");
}

void wait_for_flash_ready(nrf_fstorage_t const * p_fstorage)
{
    /* While fstorage is busy, sleep and wait for an event. */
    while (nrf_fstorage_is_busy(p_fstorage))
    {

    }
}

/**@brief   Helper function to obtain the last address on the last page of the on-chip flash that
 *          can be used to write user data.
 */
static uint32_t nrf5_flash_end_addr_get()
{
    uint32_t const bootloader_addr = NRF_UICR->NRFFW[0];
    uint32_t const page_sz         = NRF_FICR->CODEPAGESIZE;
    uint32_t const code_sz         = NRF_FICR->CODESIZE;

    return (bootloader_addr != 0xFFFFFFFF ?
            bootloader_addr : (code_sz * page_sz));
}

uint32_t nrf_use_flash_init(void)
{
    nrf_fstorage_api_t * p_fs_api;
	ret_code_t rc;

#ifdef SOFTDEVICE_PRESENT
    USE_FLASH_PRINT("SoftDevice is present.");
    USE_FLASH_PRINT("Initializing nrf_fstorage_sd implementation...");
    /* Initialize an fstorage instance using the nrf_fstorage_sd backend.
     * nrf_fstorage_sd uses the SoftDevice to write to flash. This implementation can safely be
     * used whenever there is a SoftDevice, regardless of its status (enabled/disabled). */
    p_fs_api = &nrf_fstorage_sd;
#else
    USE_FLASH_PRINT("SoftDevice not present.");
    USE_FLASH_PRINT("Initializing nrf_fstorage_nvmc implementation...");
    /* Initialize an fstorage instance using the nrf_fstorage_nvmc backend.
     * nrf_fstorage_nvmc uses the NVMC peripheral. This implementation can be used when the
     * SoftDevice is disabled or not present.
     *
     * Using this implementation when the SoftDevice is enabled results in a hardfault. */
    p_fs_api = &nrf_fstorage_nvmc;
#endif
    rc = nrf_fstorage_init(&fs_dfu_config, p_fs_api, NULL);
    APP_ERROR_CHECK(rc);

    print_flash_info(&fs_dfu_config);

    /* It is possible to set the start and end addresses of an fstorage instance at runtime.
     * They can be set multiple times, should it be needed. The helper function below can
     * be used to determine the last address on the last page of flash memory available to
     * store data. */	
}


/*	p_dest   : destination address
 *	p_src    : pointer to data src
 *  len_byte : length of data in byte
 */

uint32_t nrf_use_flash_store(uint32_t p_dest, uint32_t const *p_src, uint32_t len_bytes, bool *flash_operation_pending)
{
	*flash_operation_pending = true;
    ret_code_t rc = nrf_fstorage_write(&fs_dfu_config, p_dest, p_src, len_bytes, NULL);
    if(rc == NRF_SUCCESS)
	{
		wait_for_flash_ready(&fs_dfu_config);
	}
	else
	{
		USE_FLASH_PRINT("error %d",rc);
	}
	*flash_operation_pending = false;
	return rc;
}

uint32_t nrf_use_flash_erase(uint32_t page_addr, uint32_t number_pages, bool *flash_operation_pending)
{
	*flash_operation_pending = true;
	ret_code_t rc = nrf_fstorage_erase(&fs_dfu_config, page_addr, number_pages, NULL);
    if(rc == NRF_SUCCESS)
	{
		wait_for_flash_ready(&fs_dfu_config);
	}
	*flash_operation_pending = false;
	return rc;
}
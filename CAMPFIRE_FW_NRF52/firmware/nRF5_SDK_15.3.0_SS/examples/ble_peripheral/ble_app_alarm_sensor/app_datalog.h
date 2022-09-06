#ifndef __APP_DATALOG_H__
#define __APP_DATALOG_H__

#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "system_config.h"

#define DATALOG_STORAGE_ADDR_START	0x50000

extern DB_t dbData;
extern bool is_app_alive;
extern bool is_pop_data_triggered;
extern DB_t saveDB;
extern bool is_datalog_save_enabled;



void datalog_init(void);
uint32_t datalog_nrf_use_flash_write(uint32_t p_dest, uint32_t const *p_src, uint32_t len_bytes, bool *flash_operation_pending);
uint32_t datalog_nrf_use_flash_read(uint32_t src, uint32_t const *p_dest, uint32_t len_bytes, bool *flash_operation_pending);
uint32_t datalog_nrf_use_flash_erase(uint32_t page_addr, uint32_t number_pages, bool *flash_operation_pending);
void datalog_test(void);
void app_datalog_task();
void app_save_datalog(void);

void datalog_tick_ms_run();

void datalog_tick_ms_reset();

uint32_t datalog_tick_ms_get();

void datalog_tick_s_run();
void datalog_tick_s_reset();
uint32_t datalog_tick_s_get();


#endif
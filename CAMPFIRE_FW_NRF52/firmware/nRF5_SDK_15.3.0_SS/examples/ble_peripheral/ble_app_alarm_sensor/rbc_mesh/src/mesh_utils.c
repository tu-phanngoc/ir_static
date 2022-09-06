#include <stdint.h>
#include "app_error.h"
#include "rbc_mesh.h"
#include "mesh_utils.h"
#include "system_config.h"

#define INVALID_HANDLE	0xFF
#define PROPERTY_IDX_POS	8

/* Static function prototypes */
static uint64_t _convert_ble_addr_to_long(uint8_t* p_addr);

/* Public functions */
void mesh_enable_all_handle(void) {
	uint32_t error_code;
	uint8_t no_handle;
	
	error_code = rbc_mesh_handle_count_get(&no_handle);
	APP_ERROR_CHECK(error_code);
	
	for (int i = RBC_MESH_HANDLE_IDX_MIN; i <= no_handle; i++) {
		error_code = rbc_mesh_value_enable(i);
		APP_ERROR_CHECK(error_code);
	}
}

void mesh_disable_all_handle(void) {
	uint32_t error_code;
	uint8_t no_handle;
	
	error_code = rbc_mesh_handle_count_get(&no_handle);
	APP_ERROR_CHECK(error_code);
	
	for (int i = RBC_MESH_HANDLE_IDX_MIN; i <= no_handle; i++) {
		error_code = rbc_mesh_value_disable(i);
		APP_ERROR_CHECK(error_code);
	}
}

uint32_t mesh_add_packet(uint8_t handle, uint8_t* p_data, uint16_t len) {
	uint64_t addr_long = _convert_ble_addr_to_long(p_data);// + p_data[8];
	uint8_t m_handle = (addr_long % MESH_HANDLE_CNT) + 1;
	
	if(m_handle == sysCfg.gatewayHandle)
	{
		m_handle = m_handle + 1;
	}
	
	if(m_handle == 0) m_handle = 1;
	if(m_handle > MESH_HANDLE_CNT) m_handle = 1;
	
	handle = m_handle;
//	if(handle == 0xff)
//	{
//		handle = m_handle;
//	}
//	else if(handle != m_handle)
//	{
//		handle = m_handle;
//	}
	return rbc_mesh_value_set(handle, p_data, len);
}

/* Static functions */
static uint64_t _convert_ble_addr_to_long(uint8_t* p_addr)
{
	uint64_t ll_addr = 0;
	uint8_t *pt = (uint8_t *)&ll_addr;
	pt[0] = p_addr[0];
	pt[1] = p_addr[1];
	pt[2] = p_addr[2];
	pt[3] = p_addr[3];
	pt[4] = p_addr[4];
	pt[5] = p_addr[5];

	return ll_addr;
}

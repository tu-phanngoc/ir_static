#ifndef __MESH_UTILS_H__
#define __MESH_UTILS_H__

#include <stdint.h>

/* Module APIs */
void mesh_enable_all_handle(void);
void mesh_disable_all_handle(void);
uint32_t mesh_add_packet(uint8_t handle, uint8_t* p_data, uint16_t len);

#endif

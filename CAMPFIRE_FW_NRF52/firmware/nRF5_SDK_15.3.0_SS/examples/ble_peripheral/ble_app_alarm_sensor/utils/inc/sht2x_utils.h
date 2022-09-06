/**************************************************************************************************
/@ Project   :  Smart Home      
/@ File      :  sht2x_utils.h
/@ Author    :  
/@ Brief     :  Header SHT2x Sensors Utils functions for Application
**************************************************************************************************/

#ifndef __SHT_SENSOR_UTILS_H__
#define __SHT_SENSOR_UTILS_H__

#ifdef __cplusplus
extern "C"
{
#endif

/**************************************************************************************************
 * INCLUDES
 */

#include <stdint.h>
#include <stdbool.h>
	
/**************************************************************************************************
 * FUNCTIONS PROTOTYPES
 */

/**************************************************************************************************
* @fn          humidity_sensor_init
*
* @brief       Initialise the humidity sensor driver
*
* @return      None
*/

void humidity_sensor_init(void);

/**************************************************************************************************
* @fn          humidity_sensor_send_presentation
*
* @brief       Send SHT2x Sensor's ID to Gateway
*
* @return      None
*/

void humidity_sensor_send_presentation(void);

/**************************************************************************************************
* @fn          humidity_sensor_send_data
*
* @brief       Send Temperature and Humidity in 4 Bytes to Gateway
*			   Two first bytes are temperature and the second is humidity 
*
* @return      None
*/

void humidity_sensor_send_data(uint8_t sequence);
	
/**************************************************************************************************
* @fn          humidity_sensor_trigger_to_send_data
*
* @brief       Set sending flag to ready when interrupt occurs
*
* @return      None
*/

void humidity_sensor_trigger_to_send_data(void);

bool humidity_sensor_is_need_to_send(void);

void humidity_sensor_tick_1s_handler(void) ;

#endif

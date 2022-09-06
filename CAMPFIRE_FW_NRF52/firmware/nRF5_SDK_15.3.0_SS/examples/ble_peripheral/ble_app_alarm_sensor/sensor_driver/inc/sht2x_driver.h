/**************************************************************************************************
/@ Project   :  Smart Home      
/@ File      :  humid_driver.h
/@ Author    :  
/@ Brief     :  Header for the Sensirion SHT21 Humidity sensor
**************************************************************************************************/

#ifndef HAL_HUMID_H
#define HAL_HUMID_H

#ifdef __cplusplus
extern "C"
{
#endif
  
/**************************************************************************************************
 * INCLUDES
 */
	
#include <stdint.h>
#include <stdbool.h>

#define Temperature SHT_Temp_Read()
#define Humidity    SHT_Humid_Read()
   
/**************************************************************************************************
* @fn          vt_sensor_humidity_init
*
* @brief       Initialise the humidity sensor driver
*
* @return      TRUE if process is completed
*/

bool vt_sensor_humidity_init(void);

/**************************************************************************************************
* @fn          vt_sensor_humidity_read
*
* @brief       Read humidity and ambience temperature value from sensor SHT21
*
* @param       Pointer signed integer 16 bit points to sensor's value
*			   *type_measurement_function : function pointer to select : Temperature of Humidity 
*
* @return      TRUE if process is completed
*/

bool vt_sensor_humidity_read(int16_t *p_temp, int16_t *p_humid);
   
/**************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* HAL_HUMID_H */

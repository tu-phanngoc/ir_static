
/******************************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 22 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN
******************************************************************************/

#ifndef __AMPM_TIMESLOT_H__
#define __AMPM_TIMESLOT_H__



#include "nrf_soc.h"

#include <stdbool.h>


void  timeslot_sys_evt_handler(uint32_t evt);

/* Initialize the timeslot scanner. The irq passed to the function will be triggered when
 * an advertising report is ready.
 */
void timeslot_handler_init (void);
/** 
* @brief order a timeslot as soon as possible.
* 
* @param[in] length_us Desired length of timeslot in microseconds
* @param[in] immediately Whether to wait for current timeslot to end before 
*   ordering. At the end of the timeslot, the last request is the one that's
*   processed.
*/
void timeslot_order_earliest(uint32_t length_us, bool immediately);

/** 
* @brief order a timeslot some time after the one before it
* 
* @param[in] length_us Desired length of timeslot in microseconds
* @param[in] distance_us Distance between start of previous and current timeslot
* @param[in] immediately Whether to wait for current timeslot to end before 
*   ordering. At the end of the timeslot, the last request is the one that's
*   processed.
*/
void timeslot_order_normal(uint32_t length_us, uint32_t distance_us, bool immediately);

/**
* @brief Extend current timeslot by some extra time 
*/
void timeslot_extend(uint32_t extra_time_us);

/** @brief returns the time in us until current timeslot ends */
uint32_t timeslot_get_remaining_time(void);

/** @brief returns the timestamp the timeslot is set to end at */
uint64_t timeslot_get_end_time(void);


#endif /* __AMPM_TIMESLOT_H__ */

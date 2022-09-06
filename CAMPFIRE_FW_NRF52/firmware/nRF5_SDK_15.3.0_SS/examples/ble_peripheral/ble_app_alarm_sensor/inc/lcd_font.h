

#ifndef __LCD_FONT_H__
#define __LCD_FONT_H__

#include <stdint.h>
// Font sizes
#define SMALL		0
#define MEDIUM		1
#define	LARGE		2

extern const unsigned char G_Ultrachip[];
extern const uint8_t logo[];
extern const uint8_t LUTDefault_part[31];
extern const uint8_t LUTDefault_full[31];
//----------------------------------------------------
// Step
//----------------------------------------------------
extern const uint8_t uc_arryStepLeft[];
extern const uint8_t uc_arryStepRight[];
extern const uint8_t uc_arryStepClear[];
//-----------------------------------------------------------------------------
//ki tu dau . , : + - 
extern const uint8_t font1_dot[];
extern const uint8_t font2_dot[];
extern const uint8_t font1_colon[];
extern const uint8_t font2_colon[];
extern const uint8_t font3_colon[];
extern const uint8_t fontIco_add[];
extern const uint8_t font_forwardSlash[];
extern const uint8_t font_minus[];
//cum tu
extern const uint8_t font_ble[];
extern const uint8_t font_setting[];
extern const uint8_t font_exit[];
extern const uint8_t font1_timer[];
extern const uint8_t font2_timer[];
extern const uint8_t font2_pedometer[];
extern const uint8_t font1_step[];
extern const uint8_t font1_dst[];
extern const uint8_t font1_userTime[];
extern const uint8_t font2_userTime[];
extern const uint8_t font1_worldTime[];
extern const uint8_t font2_worldTime[];
extern const uint8_t font1_alarm[];
extern const uint8_t font2_alarm[];
extern const uint8_t font_alm[];
extern const uint8_t font1_hourlyChime[];
extern const uint8_t font2_hourlyChime[];
extern const uint8_t font1_light[];
extern const uint8_t font2_light[];
extern const uint8_t font_seconds[];
extern const uint8_t font2_seconds[];
extern const uint8_t font1_keyTone[];
extern const uint8_t font2_keyTone[];
extern const uint8_t font1_keyLock[];
extern const uint8_t font2_keyLock[];
extern const uint8_t font_sig[];
extern const uint8_t font1_off[];
extern const uint8_t font2_off[];
extern const uint8_t font3_off[];
extern const uint8_t font1_on[];
extern const uint8_t font2_on[];
extern const uint8_t font3_on[];
extern const uint8_t font_t1[];
extern const uint8_t font_t2[];
extern const uint8_t font_12h[];
extern const uint8_t font_24h[];
extern const uint8_t font_cityUtc[];
extern const uint8_t font_city[];
extern const uint8_t font_dst[];
extern const uint8_t font_run[];
extern const uint8_t font_set[];
extern const uint8_t font_sec[];
extern const uint8_t font_success[];
extern const uint8_t font_glucose[];
extern const uint8_t font_link[];
extern const uint8_t font_signal[];
extern const uint8_t font_lost[];
extern const uint8_t font_lowBattery[];
extern const uint8_t font_charging[];
extern const uint8_t font_chargingFull[];
extern const uint8_t fontIco_choose[];
extern const uint8_t fontIco_back[];
extern const uint8_t fontIco_ble[];
extern const uint8_t fontIco_setting[];
extern const uint8_t fontIco_exit[];
extern const uint8_t fontIco_timer[];
extern const uint8_t fontIco_up[];
extern const uint8_t fontIco_down[];
extern const uint8_t fontIco_left[];
extern const uint8_t fontIco_alarm[];
extern const uint8_t fontIco_hourlyChime[];
extern const uint8_t fontIco_light[];
extern const uint8_t fontIco_keyTone[];
extern const uint8_t fontIco_keyLock[];
extern const uint8_t fontIco2_keyLock[];
extern const uint8_t fontIco_link[];
extern const uint8_t fontIco_step[];
extern const uint8_t fontIco_stepLeft[];
extern const uint8_t fontIco_stepRight[];
extern const uint8_t fontIco_khungStep[];
extern const uint8_t fontIco_stepCount[];

extern const uint8_t fontPin_0_0[];
extern const uint8_t fontPin_0_1[];
extern const uint8_t fontPin_0[];
extern const uint8_t fontPin_1[];
extern const uint8_t fontPin_2[];
extern const uint8_t fontPin_3[];
extern const uint8_t khung_menu[];
extern const uint8_t khung_citycode[];
extern const uint8_t khung_ymd[];
extern const uint8_t khung_hms[];
extern const uint8_t khung_alm[];

extern const uint8_t font1_doc[];

extern const uint8_t FONT6x8[97][8];

extern const uint8_t FONT8x8[97][8];		

extern const uint8_t FONT8x16[97][16];

extern const uint8_t		*font1Table[];//<2>

extern const uint8_t		*font2Table[];//<3>

extern const uint8_t		*font3Table[];//<4>

extern const uint8_t		*font4Table[];//<5>

extern const uint8_t		*font5Table[]; //<1>

extern const uint8_t		*fontMTable[];

extern const uint8_t		*fontDTable[];

extern const uint8_t *cityCodeTable[];

extern const int16_t cityCodeTimeZoneTable[];

extern const uint8_t dstTimeTable[];

extern const uint8_t fontIco_rising[];

extern const uint8_t fontIco_rapidlyRising[];

extern const uint8_t fontIco_falling[];

extern const uint8_t fontIco_rapidlyFalling[];

extern const uint8_t fontIco_constant[];

extern const uint8_t fontIco_slowlyRising[];

extern const uint8_t fontIco_slowlyFalling[];
extern const uint8_t *trendArrowTable[];

#endif



#ifndef __SMARTTOUCH_UTILS_H__
#define __SMARTTOUCH_UTILS_H__

/* Module APIs */
void smarttouch_init(void);

void smarttouch_send_presentation(void);

void smarttouch_send_status(uint8_t sequence);

void _send_smarttouch_info(uint8_t sequence);

void _set_smarttouch_status(bool status);
bool _get_smarttouch_status(void);
#endif

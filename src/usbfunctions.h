#ifndef USB_FUNCS_H_
#define USB_FUNCS_H
#include <stdint.h>
void usb_setup(void);
uint8_t usb_getc(void);
void usb_putc(uint8_t c); 
int usb_send(char *out_buf,int out_buf_len);
void usb_timer_callback(void);
#endif

/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include "usbfunctions.h"
#include <errno.h>
#include <stdio.h>

#define SYSTICK_RELOAD_PERIOD 624 //(72000000/((115200/8)*8))
#define SYSTICK_STARTUP_DELAY 1000
volatile uint32_t startup_counter=0;

int _write(int file, char *ptr, int len);

char cmdline[256];

void systick_setup(void){
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
	/* SysTick interrupt every N clock pulses: set reload to N-1 */
	systick_set_reload(SYSTICK_RELOAD_PERIOD);
	systick_interrupt_enable();
	systick_counter_enable();
}

char out_buf[10];
int out_buf_len=0;

void sys_tick_handler(void){
  
  if(startup_counter>SYSTICK_STARTUP_DELAY){
    usb_timer_callback();
  }else{
    startup_counter++;
  }
}

int _write(int file, char *ptr, int len)
{
	int i;

	if (file == 1) {
		for (i = 0; i < len; i++)
			usb_putc(ptr[i]);
		return i;
	}

	errno = EIO;
	return -1;
}

static void read_string(int len, char *my_buffer){
	uint8_t pos=0;
    uint8_t i;
    usb_putc('\r');
    usb_putc('>');
	//_write(1,"Command> ",sizeof("Command> ")-1);
	char c=usb_getc();
	while(c!=13 && pos<len-1){
		usb_putc(c);
		if(c!=127/*backspace*/){
			my_buffer[pos]=c;
			pos++;
		}else{
			my_buffer[pos]=0;
			if(pos>0) pos--;
			my_buffer[pos]=0;
            usb_putc('\033');
            usb_putc('[');
            usb_putc('2');
            usb_putc('K');
            usb_putc('\r');
//          printf("\033[2K\r");
			usb_putc('>');
            for(i=0;i<pos;i++){
               usb_putc(my_buffer[i]);
            } 
		}
	  c = usb_getc();
	}
    usb_putc('\n');
    usb_putc('\r');
	my_buffer[pos]=0;
	return;
}

int main(void)
{
	int i;


	rcc_clock_setup_in_hsi_out_48mhz();

    /*Enable transistor switch to make usb autodetect work*/
    usb_setup();
    systick_setup();
    //nvic_enable_irq(NVIC_USB_HP_CAN_TX_IRQ);
    out_buf[0]='a';
    out_buf[1]='b';
    out_buf_len=2;

	while (1){
		//usbd_poll(usbd_dev);
          printf("cmd:\n\r");
          read_string(255,cmdline);
          printf("Got cmd: %s\n\r",cmdline);
    }

}

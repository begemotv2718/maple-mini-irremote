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

#define SYSTICK_RELOAD_PERIOD 624 //(72000000/((115200/8)*8))
#define SYSTICK_STARTUP_DELAY 1000
volatile uint32_t startup_counter=0;

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
    if(out_buf_len>0)
      usb_send(out_buf,out_buf_len);
  }else{
    startup_counter++;
  }
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
    }

}

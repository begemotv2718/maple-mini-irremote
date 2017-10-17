#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include "irled.h"

volatile struct led_prg_command commands[CMD_ARRAY_LEN];
volatile uint8_t start_blink=0;
volatile uint8_t cmd_idx=0;
volatile uint16_t cmd_time=0;

void setup_irled_timer(void)
{
	/* Set up the timer TIM2 for injected sampling */
	uint32_t timer;
    uint32_t period=1263;

	timer   = TIM4;

	rcc_periph_clock_enable(RCC_TIM4);

	/* Time Base configuration */
    timer_reset(timer);
    timer_set_mode(timer, TIM_CR1_CKD_CK_INT,
	    TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    timer_set_period(timer, period-1);
    timer_set_prescaler(timer, 0);
    timer_set_clock_division(timer, 0x0);

    timer_set_oc_mode(timer,TIM_OC1,TIM_OCM_PWM1);
    timer_enable_oc_preload(timer,TIM_OC1);
    timer_set_oc_polarity_low(timer,TIM_OC1);
    timer_enable_oc_output(timer,TIM_OC1);
    timer_set_oc_value(timer,TIM_OC1,period/2-1);

    timer_enable_counter(timer);
}

void setup_irled_gpio(void)
{
    rcc_periph_clock_enable(RCC_GPIOB);
    gpio_set_mode(GPIOB,GPIO_MODE_OUTPUT_50_MHZ,
        GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,GPIO_TIM4_CH1);
    gpio_set_mode(GPIOB,GPIO_MODE_OUTPUT_2_MHZ,
        GPIO_CNF_OUTPUT_PUSHPULL,GPIO7);
}

void setup_transmission_timer(void)
{
    uint32_t timer = TIM2;

    rcc_periph_clock_enable(RCC_TIM2);

    nvic_enable_irq(NVIC_TIM2_IRQ);

    timer_reset(timer);

    timer_set_mode(timer,TIM_CR1_CKD_CK_INT,TIM_CR1_CMS_EDGE,TIM_CR1_DIR_UP);
    timer_set_period(timer,65535);
    timer_set_prescaler(timer,0);
    

    timer_enable_counter(timer);
    timer_enable_irq(timer,TIM_DIER_UIE);

}

void program_nec_code(uint8_t *data, uint8_t len)
{
  int i,j;
  int offset=0;
  while(is_blinking());
  commands[offset].time_len=START_PULSE_ON_DELAY;
  commands[offset].action=LED_ON;
  offset++;
  commands[offset].time_len=START_PULSE_OFF_DELAY;
  commands[offset].action=LED_OFF;
  offset++;
  for(i=0;i<len;i++)
  {
    for(j=0;j<8;j++)
    {
      if(data[i]&(1>>j))
      {
        commands[offset].time_len=ONE_ON_DELAY;
        commands[offset].action=LED_ON;
        offset++;
        commands[offset].time_len=ONE_OFF_DELAY;
        commands[offset].action=LED_OFF;
        offset++;
      }
      else
      {
        commands[offset].time_len=ZERO_ON_DELAY;
        commands[offset].action=LED_ON;
        offset++;
        commands[offset].time_len=ZERO_OFF_DELAY;
        commands[offset].action=LED_OFF;
        offset++;
      }
    }
  }
  commands[offset].action=STOP;
}

int is_blinking(void)
{
  return start_blink;
}

void start_blinking(void)
{
  while(is_blinking());
  cmd_idx=0;
  start_blink=1;
}


void tim2_isr(void)
{
  if(timer_get_flag(TIM2,TIM_SR_UIF))
  { 
    if(start_blink)
    {
      switch(commands[cmd_idx].action)
      {
        case STOP:
          start_blink=0;
          gpio_set(GPIOB,GPIO7);
          cmd_idx=0;
          break;
        case LED_ON:
          gpio_clear(GPIOB,GPIO7);
          if(cmd_time<commands[cmd_idx].time_len)
          {
            cmd_time++;
          }
          else
          {
            cmd_idx++;
            cmd_time=0;
          }
          break;
        case LED_OFF:
          gpio_set(GPIOB,GPIO7);
          if(cmd_time<commands[cmd_idx].time_len)
          {
            cmd_time++;
          }
          else
          {
            cmd_idx++;
            cmd_time=0;
          }
          break;
      }
      if(cmd_idx>60)
      {
        cmd_idx=0;
        start_blink=0;
      }
    }
    gpio_toggle(GPIOB,GPIO7);
    timer_clear_flag(TIM2,TIM_SR_UIF);
  }
}

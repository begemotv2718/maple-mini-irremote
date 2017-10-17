#ifndef IRLED_H_
#define IRLED_H_
#include <stdint.h>
void setup_irled_timer(void);
void setup_irled_gpio(void);
void setup_transmission_timer(void);
void start_blinking(void);
int is_blinking(void);
void program_nec_code(uint8_t *data, uint8_t len);

enum led_prg_action {
  STOP,
  LED_ON,
  LED_OFF
};

struct led_prg_command {
  uint16_t time_len;
  enum led_prg_action action;
};

#define CMD_ARRAY_LEN (128U)
#define START_PULSE_ON_DELAY (16U)
#define START_PULSE_OFF_DELAY (8U)
#define ONE_ON_DELAY (1U)
#define ONE_OFF_DELAY (2U)
#define ZERO_ON_DELAY (1U)
#define ZERO_OFF_DELAY (1U)
#endif  

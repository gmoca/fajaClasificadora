#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

typedef enum { BTN_MODE, BTN_UP, BTN_DOWN } button_t;
typedef enum { HB_STOP, HB_FWD, HB_REV } hbridge_dir_t;

void    gpio_init(void);
void    gpio_scan_buttons(void);
uint8_t gpio_button_pressed(button_t btn);
void    gpio_hbridge_dir(hbridge_dir_t dir);
void    gpio_breakbeam_emitter(uint8_t on);
uint8_t gpio_breakbeam_read(uint8_t station);

#endif

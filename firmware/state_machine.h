#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdint.h>

typedef enum {
    ST_IDLE,
    ST_RUNNING,
    ST_SORTING,
    ST_ERROR,
    ST_TEST
} state_t;

void state_machine_init(void);
void state_machine_run(void);
void state_machine_estop(void);
void state_machine_estop_reason(const char *reason);
void state_machine_start(void);
void state_machine_test_enter(void);
void state_machine_test_exit(void);
void state_machine_test_motor(void);
extern volatile uint8_t estop_triggered_by_button;

// Task 12 missing setters
void state_machine_set_mode(uint8_t mode);
void state_machine_set_spacing(uint16_t space);
void state_machine_set_dwell(uint16_t dwell);

state_t state_machine_get(void);

#endif

#include "anti_jam.h"
#include "encoder.h"
#include "gpio.h"
#include "state_machine.h"
#include "system.h"
#include "pwm.h"

static uint8_t jammed = 0;
static uint32_t zero_speed_start = 0;

void anti_jam_init(void) {
    jammed = 0;
    zero_speed_start = 0;
}

void anti_jam_check(void) {
    if (state_machine_get() != ST_RUNNING && state_machine_get() != ST_SORTING) {
        zero_speed_start = 0;
        return; 
    }
    
    /* COMENTADO TEMPORALMENTE PARA PRUEBAS EN FÍSICO SIN ENCODER
    if (encoder_get_speed_mm_s() == 0) {
        if (zero_speed_start == 0) {
            zero_speed_start = system_ticks;
        } else if (system_ticks - zero_speed_start > 1000) { 
            jammed = 1;
            state_machine_estop_reason("BELT_MOTOR_JAM");
        }
    } else {
        zero_speed_start = 0;
    }
    */
    
    // Check continuous break-beam blockage (more than 3 seconds)
    static uint32_t beam_block_start = 0;
    if (gpio_breakbeam_read(1) || gpio_breakbeam_read(2)) {
        if (beam_block_start == 0) beam_block_start = system_ticks;
        else if (system_ticks - beam_block_start > 3000) {
            jammed = 1;
            state_machine_estop_reason("LASER_BEAM_BLOCKED");
        }
    } else {
        beam_block_start = 0;
    }
}

uint8_t anti_jam_is_jammed(void) {
    return jammed;
}

#ifndef ANTI_JAM_H
#define ANTI_JAM_H

#include <stdint.h>

void anti_jam_init(void);
void anti_jam_check(void);
uint8_t anti_jam_is_jammed(void);

#endif

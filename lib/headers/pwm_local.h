#ifndef PWM_LOCAL_H
#define PWM_LOCAL_H

#include <stdlib.h>
#include "pico/stdlib.h"

void setup_pwm(uint pin, uint pwm_wrap);
void update_duty_cycle_pwm(uint pin, uint duty_cycle);

#endif
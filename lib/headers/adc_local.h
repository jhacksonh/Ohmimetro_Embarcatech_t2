#ifndef ADC_LOCAL_H
#define ADC_LOCAL_H

#include <stdlib.h>
#include "pico/stdlib.h"

void setup_adc(uint pin);
int read_adc(uint input);

#endif
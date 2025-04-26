#ifndef INTERRUPT_LOCAL_H
#define INTERRUPT_LOCAL_H

#include <stdlib.h>
#include "pico/stdlib.h"

void itr_SetCallbackFunction(void (*func)(uint, uint32_t));
void itr_Interruption(uint pin);

#endif
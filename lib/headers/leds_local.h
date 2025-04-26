#ifndef LEDS_LOCAL_H
#define LEDS_LOCAL_H

#include <stdlib.h>
#include <pico/stdlib.h>

void Leds_init(uint pin, int len_leds);
void Leds_Map_leds_ON(uint8_t *LedsOn, uint8_t colorsOn[][3], int LedsOnCount, bool clear_cache);
void Leds_Clear_leds(bool clear_all);

#endif
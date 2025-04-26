#ifndef BUZZER_LOCAL_H
#define BUZZER_LOCAL_H

#include <stdlib.h>
#include "pico/stdlib.h"

void buzzer_init(uint pin);
void buzzer_play_note(int hz, int ms);
void buzzer_multiplay(int *notes, int *ms_s, int length);

#endif 
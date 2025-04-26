#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "headers/buzzer_local.h"

static int SLICE_NUM  = 0;
static uint PIN = 0;


static void buzzer_control(uint PIN, bool turn_on) {
    if (turn_on){
        gpio_set_function(PIN, GPIO_FUNC_PWM);
    }else{
        gpio_set_function(PIN, GPIO_FUNC_SIO);
        gpio_put(PIN, 0);
    }    
    pwm_set_enabled(SLICE_NUM, turn_on);
}
static void buzzer_setup(uint PIN, int hz) {
    if (hz > 0) { // Verifica frequência e duração válidas
        SLICE_NUM = pwm_gpio_to_slice_num(PIN);
        int channel = pwm_gpio_to_channel(PIN);
        uint32_t freq_sistema = clock_get_hz(clk_sys); // Frequência do sistema
        uint16_t wrap_valor = freq_sistema / hz - 1; // Define o valor de wrap
        pwm_set_wrap(SLICE_NUM, wrap_valor);
        pwm_set_chan_level(SLICE_NUM, channel, wrap_valor / 2); // Duty cycle de 50%
    }
}
void buzzer_init(uint pin){
    PIN = pin;
    gpio_set_function(PIN, GPIO_FUNC_SIO); // Configura o pino como GPIO
    gpio_set_dir(PIN, GPIO_OUT); // Define como saída
    gpio_put(PIN, 0); // Define o pino como LOW
}


void buzzer_play_note(int hz, int ms) {
    buzzer_setup(PIN, hz);
    buzzer_control(PIN, true); // Configura o pino como PWM
    sleep_ms(ms); // Duração da nota
    buzzer_control(PIN, false);
}

//  duracoes em ingles é: duration
void buzzer_multiplay(int *notes, int *ms_s, int length) {
    if (notes && ms_s && length > 0) { // Verifica arrays válidos
        for (int i = 0; i < length; i++) {
            int hz = notes[i];
            int ms = ms_s[i];
            buzzer_play_note(hz, ms);
        }
    }
}
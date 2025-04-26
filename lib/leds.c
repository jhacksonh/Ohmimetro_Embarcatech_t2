#include <stdio.h>
#include "pico/stdlib.h"
#include "ws2812.pio.h"
#include <string.h>
#include "headers/leds_local.h"

#define MAX_LEDS 100 // Quantidade máxima de LEDs que podem ser controlados
PIO pio = pio0; // PIO usada para controlar os LEDs
uint sm = 0; // Máquina de estados usada para controlar os LEDs
static int LED_COUNT; // Quantidade de LEDs controlados
static uint8_t colors[MAX_LEDS][3] = {0}; // Array para armazenar as cores dos LEDs

// Função para Conversão de cores RGB para GRB
static void Leds_rgb_to_grb(uint8_t colors[MAX_LEDS][3]) {
    // Array para armazenar as cores em formato GRB
    uint32_t grb[LED_COUNT];
    // Itera sobre os LEDs e faz a conversão de RGB para GRB
    for (int i = 0; i < LED_COUNT; i++) {
        // Pega a cor atual em formato RGB
        uint8_t r = colors[i][0];
        uint8_t g = colors[i][1];
        uint8_t b = colors[i][2];
        grb[i] = (g << 16) | (r << 8) | b;// Converte de RGB para GRB, invertendo a ordem dos bits para compatibilidade com a biblioteca ws2812.
        // Envia a cor convertida para a ws2812, aplicando shift de 8 bits para o formato GRB.  
        pio_sm_put_blocking(pio, sm, grb[i] << 8u);
    }
}
// Inicializa o controlador ws2812 para controlar LEDs
void Leds_init(uint pin, int len_leds){
    // Define a quantidade de LEDs que serão controlados
    LED_COUNT = len_leds;
    // Adiciona o programa ws2812 ao PIO e obtém o offset
    uint offset = pio_add_program(pio, &ws2812_program);
    // Inicializa o programa ws2812 no estado da máquina com parâmetros: PIO, estado, offset, pino, frequência e saída invertida.  
    ws2812_program_init(pio, sm, offset, pin, 800000, false);
    // Habilita o estado da máquina para começar a enviar dados
    pio_sm_set_enabled(pio, sm, true);
    // Limpa o estado atual dos LEDs, apagando-os
    Leds_Clear_leds(true);
    // Aguarda um pequeno tempo para garantir que o estado seja atualizado
    sleep_us(100);
}
// Ativa LEDs específicos com cores específicas e converte as cores para o formato GRB
void Leds_Map_leds_ON(uint8_t *LedsOn, uint8_t colorsOn[][3], int LedsOnCount, bool clear_cache){
    // Se o parâmetro clear_cache for true, limpa o estado atual dos LEDs
    if (clear_cache){
        Leds_Clear_leds(false);
    }
    // Itera sobre os LEDs que devem ser ligados
    for (uint8_t i = 0; i < LedsOnCount; i++) {
        // Define a cor do LED especificado em LedsOn[i] usando a cor correspondente em colorsOn[i]
        colors[LedsOn[i]][0] = colorsOn[i][0]; // Componente vermelho
        colors[LedsOn[i]][1] = colorsOn[i][1]; // Componente verde
        colors[LedsOn[i]][2] = colorsOn[i][2]; // Componente azul
    }
    // Converte as cores dos LEDs para o formato GRB e envia para o controlador de LEDs
    Leds_rgb_to_grb(colors);
}
// Função para limpar o estado dos LEDs
void Leds_Clear_leds(bool clear_all){
    // Zera todos os valores do array colors, apagando as cores dos LEDs com memset.  
    memset(colors, 0, sizeof(colors));
    if (clear_all){
        Leds_rgb_to_grb(colors);   // Limpa o estado atual dos LEDs, apagando-os
    }
}

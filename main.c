// Inclusão de bibliotecas necessárias
#include <math.h>       // Para funções matemáticas
#include <stdio.h>      // Para entrada/saída padrão
#include <string.h>     // Para manipulação de strings
#include <stdlib.h>     // Para funções gerais
#include "pico/stdlib.h" // SDK do Raspberry Pi Pico
#include "pico/bootrom.h" // Para funções de boot
#include "hardware/adc.h" // Para o conversor analógico-digital
#include "hardware/i2c.h" // Para comunicação I2C
#include "lib/headers/adc_local.h" // Biblioteca local para ADC
#include "lib/headers/oled_local.h" // Biblioteca local para OLED
#include "lib/headers/leds_local.h" // Biblioteca local para LEDs
#include "lib/headers/interrupt_local.h" // Biblioteca local para interrupções

// Definições dos pinos
#define PIN_I2C_SDA 14   // Pino de dados I2C
#define PIN_I2C_SCL 15   // Pino de clock I2C
#define PIN_ADC 28       // Pino GPIO para o voltímetro
#define INPUT_ADC 2      // Canal de entrada do ADC
#define PIN_LEDS 7       // Pino GPIO para os LEDs
#define PIN_Botao_B 6    // Pino GPIO para o botão B

// Estrutura para representar uma cor
typedef struct {
  char *name;       // Nome da cor
  uint8_t rgb[3];   // Valores RGB (0-10)
} Color;

// Estrutura para representar um resistor
typedef struct {
  Color colors[3];  // 3 cores das faixas
  float ohms;       // Valor em ohms
} Resistor;

// Constantes e variáveis globais
int R_conhecido = 10000;   // Resistor conhecido de 10k ohm (divisor de tensão)
float R_x = 0.0;           // Resistor desconhecido (a ser medido)
int ADC_RESOLUTION = 4095; // Resolução do ADC (12 bits)
int LED_LENGHT = 25;       // Quantidade de LEDs na fita
int POS_COLOR[3] = {0,0,0}; // Posições das cores no array COLORS
int LEN_CHARACTER_OHMs = 0; // Comprimento do texto do valor em ohms

// Cores padrão para as faixas dos resistores
Color COLORS[10] = {
  {"preto",   {0, 0, 0}},    // 0
  {"marrom",  {5, 2, 0}},    // 1
  {"vermelho",{10, 0, 0}},   // 2
  {"laranja", {10, 6, 0}},   // 3
  {"amarelo", {10, 10, 0}},  // 4
  {"verde",   {0, 5, 0}},    // 5
  {"azul",    {0, 0, 10}},   // 6
  {"violeta", {5, 1, 8}},    // 7
  {"cinza",   {5, 5, 5}},    // 8
  {"branco",  {10, 10, 10}}  // 9
};
// Cores para indicar resistor fora da faixa
Color COLOR_UNKNOWN[3] = {{"fora", {0, 0, 0}},{"da", {0, 0, 0}},{"faixa", {0, 0, 0}}};
// Variável global para armazenar o resistor atual
Resistor RESISTOR = {{{"fora",{0, 0, 0}}, {"da",{0, 0, 0}}, {"faixa",{0, 0, 0}}}, 0};

// Série E24 de valores comerciais de resistores
const int E24_SERIES[] = {
  510, 560, 620, 680, 750, 820, 910,1000, 1100, 1200, 1300, 1500, 1600, 1800, 2000, 2200, 2400, 2700,3000, 3300, 3600,
  3900,4300, 4700, 5100, 5600, 6200, 6800, 7500, 8200, 9100,10000, 11000, 12000, 13000, 15000, 16000, 18000, 20000,
  22000,24000,27000,30000, 33000, 36000, 39000, 43000, 47000, 51000, 56000, 62000,68000, 75000,82000, 91000, 100000,
};
const int E24_COUNT = sizeof(E24_SERIES) / sizeof(E24_SERIES[0]);

// Protótipos das funções
void update_information();
float adjust_to_e24_series(float value);
bool handle_unknown_resistor(float adjusted_value);
void prepare_display_and_led_values(float original_value, float adjusted_value);
void Content_Display();
void Content_Leds();

// Função de callback para o botão (entra no modo bootloader USB)
void callback_BTs(uint gpio, uint32_t events){
  reset_usb_boot(0, 0);
}

// Função principal
int main(void) {
    // Inicializações
    stdio_init_all(); // Inicializa comunicação serial
    
    // Configura o pino do botão
    gpio_init(PIN_Botao_B);
    gpio_set_dir(PIN_Botao_B,GPIO_IN);
    gpio_pull_up(PIN_Botao_B);

    // Configura interrupção para o botão
    itr_SetCallbackFunction(callback_BTs);
    itr_Interruption(PIN_Botao_B);

    // Inicializa hardware
    setup_adc(PIN_ADC);          // Configura ADC
    oled_Init(PIN_I2C_SDA, PIN_I2C_SCL); // Inicializa display OLED
    Leds_init(PIN_LEDS, LED_LENGHT);     // Inicializa LEDs
    
    // Loop principal
    while (1) {
      update_information(); // Atualiza as medições
      sleep_ms(200);       // Pequeno delay
    }
    return 0;
}

// Função para exibir conteúdo no display OLED
void Content_Display() {
    char buffer[7]; // Buffer para formatação do valor
    
    oled_Clear(); // Limpa o display
    
    // Desenha bordas e elementos gráficos
    oled_Write_String("OHMIMETRO", 37, 3);
    oled_Draw_Line_Horizontal(3, 125, 11, true);
    oled_Draw_Line_Vertical(2, 2, 62, true);
    oled_Draw_Line_Vertical(126, 5, 62, true);
    oled_Draw_Line_Horizontal(2, 126, 62, true);
    
    // Escreve os nomes das cores das faixas
    oled_Write_String(RESISTOR.colors[0].name, 5, 16);
    oled_Draw_Line_Horizontal(3, 63, 27, true);
    
    oled_Write_String(RESISTOR.colors[1].name, 5, 32);
    oled_Draw_Line_Horizontal(3, 63, 43, true);
    
    oled_Write_String(RESISTOR.colors[2].name, 5, 48);
    
    // Linha vertical separadora
    oled_Draw_Line_Vertical(63,11,61,true);
    
    // Escreve o valor do resistor
    oled_Write_String("VALOR", 61+((128-61)/2)-5*6/2, 20);
    oled_Write_String("Ohms",61+((128-61)/2)-4*6/2, 29);
    sprintf(buffer, "%.2f", RESISTOR.ohms);
    oled_Write_String(buffer, (61+(((128-61)/2)-LEN_CHARACTER_OHMs*6/2)), 45);
    
    oled_Update(); // Atualiza o display físico
}

// Função para controlar os LEDs
void Content_Leds() {
  // Mapeamento dos LEDs na fita
  static uint8_t leds[] = {5,14,15,7,12,17,9,10,19,24,23,22,21,20,4,3,2,1,0,6,13,16,8,11,18};
  static uint8_t leds_colors[19][3]; // Cores para cada LED
  int index = 0;

  // Define as cores dos LEDs baseado nas cores do resistor
  for (int i = 0; i < LED_LENGHT; i++) {
    if ((i == 3 || i == 6)) index++; // Avança para próxima cor a cada 3 LEDs
    for (int j = 0; j < 3 && i < 9; j++){
      leds_colors[i][j] = RESISTOR.colors[index].rgb[j];
    }
    // LEDs adicionais (azuis)
    if (i >= 9){
      leds_colors[i][0] = 0;
      leds_colors[i][1] = 1;
      leds_colors[i][2] = 1;
    }
  }
  // Aciona os LEDs
  Leds_Map_leds_ON(leds,leds_colors,LED_LENGHT,true);
}

// Função para ler o valor do resistor desconhecido
float Read_Resistor_Value(){
  float sample = 0.0f;
  
  // Faz 5000 leituras para média (reduz ruído)
  for (int i = 0; i < 5000; i++){
    sample += read_adc(INPUT_ADC);
    sleep_us(300);
  }
  
  float sample_media = sample / 5000.0f;
  printf("Sample Media ADC: %.2f\n", sample_media);
  // Calcula o resistor desconhecido usando divisor de tensão
  return (R_conhecido * sample_media) / (ADC_RESOLUTION - sample_media);
}

// Atualiza as informações do resistor
void update_information() {
  R_x = Read_Resistor_Value(); // Lê o valor do resistor
  float original_value = R_x;
  static char str_y[10]; // Buffer para formatação
  
  // Ajusta para o valor comercial mais próximo
  float adjusted_value = adjust_to_e24_series(R_x);
  sprintf(str_y, "%.2f", adjusted_value);
  LEN_CHARACTER_OHMs = strlen(str_y);

  // Verifica se o resistor está fora da faixa
  if (handle_unknown_resistor(adjusted_value)) {
    Content_Display();
    Content_Leds();
    return;
  }
  
  // Prepara os valores para exibição
  prepare_display_and_led_values(original_value, adjusted_value);

  // Atualiza display e LEDs
  Content_Display();
  Content_Leds();
}

// Ajusta o valor lido para o valor comercial mais próximo (série E24)
float adjust_to_e24_series(float value) {
  float adjusted_value = 0.0f;
  
  // Verifica cada valor da série E24
  for (int i = 0; i < E24_COUNT; i++) {
    float ref = (float)E24_SERIES[i];
    float delta = ref * 0.05f; // Margem de 5%
    
    // Se o valor estiver dentro da margem, usa este valor
    if (value >= (ref - delta) && value <= (ref + delta)) {
      adjusted_value = ref;
      break;
    }
  }
  return adjusted_value;
}

// Trata resistores fora da faixa conhecida
bool handle_unknown_resistor(float adjusted_value) {
  if (adjusted_value == 0.0f) {
    // Configura mensagem de "fora da faixa"
    strcpy(RESISTOR.colors[0].name, "fora");
    strcpy(RESISTOR.colors[1].name, "da");
    strcpy(RESISTOR.colors[2].name, "faixa");
    RESISTOR.ohms = 0.0f;
    
    // Usa cores padrão para "desconhecido"
    for (int i = 0; i < 3; i++) {
      RESISTOR.colors[i] = COLOR_UNKNOWN[i];
    }
    return true; // Indica que o resistor está fora da faixa
  }
  return false; // Valor válido
}

// Prepara os valores para exibição no display e LEDs
void prepare_display_and_led_values(float original_value, float adjusted_value) {
  R_x = adjusted_value;
  RESISTOR.ohms = original_value;

  // Calcula os dígitos significativos e multiplicador
  int exponent = (int)floor(log10(R_x));
  int significant_digits = (int)(R_x / pow(10, exponent - 1));
  int d1 = significant_digits / 10; // Primeiro dígito
  int d2 = significant_digits % 10; // Segundo dígito
  int multiplier = exponent - 1;    // Multiplicador (terceira faixa)

  // Armazena as posições das cores
  POS_COLOR[0] = d1;
  POS_COLOR[1] = d2;
  POS_COLOR[2] = multiplier;

  // Atualiza as cores do resistor
  for (int i = 0; i < 3; i++) {
    RESISTOR.colors[i] = COLORS[POS_COLOR[i]];
  }
  printf("Original R_x: %.2f, Adjusted: %.2f\n", original_value, R_x);
}
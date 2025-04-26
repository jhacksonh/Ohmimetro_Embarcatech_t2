#include <math.h>
#include <stdio.h>
#include <string.h>
 #include <stdlib.h>
 #include "pico/stdlib.h"
 #include "pico/bootrom.h"
 #include "hardware/adc.h"
 #include "hardware/i2c.h"
 #include "lib/headers/adc_local.h"
 #include "lib/headers/oled_local.h"
 #include "lib/headers/leds_local.h"
 #include "lib/headers/interrupt_local.h"

#define PIN_I2C_SDA 14
#define PIN_I2C_SCL 15
#define PIN_ADC 28 // GPIO para o voltímetro
#define PIN_LEDS 7
#define PIN_Botao_A 5  // GPIO para botão A
#define PIN_Botao_B 6  // GPIO para botão B

typedef struct {
  char *name;
  uint8_t rgb[3];
} Color;
typedef struct {
  Color colors[3];
  float ohms;
} Resistor;

int R_conhecido = 10000;   // RESISTOR de 10k ohm
float R_x = 0.0;           // RESISTOR desconhecido
int ADC_RESOLUTION = 4095; // Resolução do ADC (12 bits)
int LED_LENGHT = 25;
int POS_COLOR[3] = {0,0,0};
int LEN_CHARACTER_OHMs = 0;

Color COLORS[10] = {
  {"preto",   {0, 0, 0}},
  {"marrom",  {5, 2, 0}},
  {"vermelho",{10, 0, 0}},
  {"laranja", {10, 6, 0}},
  {"amarelo", {10, 10, 0}},
  {"verde",   {0, 5, 0}},
  {"azul",    {0, 0, 10}},
  {"violeta", {5, 1, 8}},
  {"cinza",   {5, 5, 5}},
  {"branco",  {10, 10, 10}}
};
Color COLOR_UNKNOWN[3] = {
  {"fora", {0, 0, 0}},
  {"da", {0, 0, 0}},
  {"faixa", {0, 0, 0}}
};
Resistor RESISTOR = {{{"fora",{0, 0, 0}}, {"da",{0, 0, 0}}, {"faixa",{0, 0, 0}}}, 0};
const int E24_SERIES[] = {
  510, 560, 620, 680, 750, 820, 910,
  1000, 1100, 1200, 1300, 1500, 1600, 1800, 2000, 2200, 2400, 2700,
  3000, 3300, 3600, 3900, 4300, 4700, 5100, 5600, 6200, 6800, 7500, 8200, 9100,
  10000, 11000, 12000, 13000, 15000, 16000, 18000, 20000, 22000, 24000, 27000,
  30000, 33000, 36000, 39000, 43000, 47000, 51000, 56000, 62000, 68000, 75000,
  82000, 91000, 100000
};
const int E24_COUNT = sizeof(E24_SERIES) / sizeof(E24_SERIES[0]);
void update_information();
void Content_Display();
void Content_Leds();

void callback_BTs(uint gpio, uint32_t events){
  reset_usb_boot(0, 0);
}
int main(void) {
    stdio_init_all();

    gpio_init(PIN_Botao_B);
    gpio_set_dir(PIN_Botao_B,GPIO_IN);
    gpio_pull_up(PIN_Botao_B);

    itr_SetCallbackFunction(callback_BTs);
    itr_Interruption(PIN_Botao_B);

    setup_adc(PIN_ADC);
    oled_Init(PIN_I2C_SDA, PIN_I2C_SCL);
    Leds_init(PIN_LEDS, LED_LENGHT);
    
    while (1) {
      update_information();
      sleep_ms(300);
    }
    return 0;
}

float Read_Resistor_Value(){
  float  sample= 0.0f;
  for (int i = 0; i < 5000; i++){
    sample += read_adc(1);
    sleep_us(300);
  }
  float sample_media = sample / 5000.0f;
  printf("Sample Media: %f\n", sample_media);
  return (R_conhecido * sample_media) / (ADC_RESOLUTION - sample_media);
}

void update_information() {
  R_x = Read_Resistor_Value();
  float original_value = R_x;

  static char str_y[10]; // Buffer para armazenar a string

  // Encontra o valor mais próximo da série E24 dentro da faixa de 5%
  float adjusted_value = 0.0f;
  for (int i = 0; i < E24_COUNT; i++) {
    float ref = (float)E24_SERIES[i];
    float delta = ref * 0.05f;
    if (R_x >= (ref - delta) && R_x <= (ref + delta)) {
      adjusted_value = ref;
      break;
    }
  }

  sprintf(str_y, "%.2f", adjusted_value);
  LEN_CHARACTER_OHMs = strlen(str_y);
  
  if (adjusted_value == 0.0f) {
    strcpy(RESISTOR.colors[0].name, "fora");
    strcpy(RESISTOR.colors[1].name, "da");
    strcpy(RESISTOR.colors[2].name, "faixa");
    RESISTOR.ohms = 0.0f;
    for (int i = 0; i < 3; i++) {
      RESISTOR.colors[i] = COLOR_UNKNOWN[i];
    }
    Content_Display();
    Content_Leds();
    return;
  }
  R_x = adjusted_value;
  RESISTOR.ohms = R_x;
  // Cálculo de dígitos e multiplicador
  int expoente = (int)floor(log10(R_x));
  int significativos = (int)(R_x / pow(10, expoente - 1));
  int d1 = significativos / 10;
  int d2 = significativos % 10;
  int multiplicador = expoente - 1;

  // Atribui cores
  POS_COLOR[0] = d1;
  POS_COLOR[1] = d2;
  POS_COLOR[2] = multiplicador;

  for (int i = 0; i < 3; i++) {
    RESISTOR.colors[i] = COLORS[POS_COLOR[i]];
  }
  printf("Original R_x: %.2f, Ajustado: %.2f\n", original_value, R_x);
  Content_Display();
  Content_Leds();
}

// void update_information(){
//   static char str_y[10]; // Buffer para armazenar a string
//   R_x = Read_Resistor_Value();

//   sprintf(str_y, "%.2f", R_x);   // Converte o float em string
//   int len = strlen(str_y);
  
//   bool dot_not = str_y[0] != "." && str_y[1] != "." && str_y[2] != ".";
//   for (int i = 0; i < 3; i++) {
//       POS_COLOR[i] = len>3 && len<=7 && dot_not?str_y[i] - '0':0;
//   }
//   printf("tamanho da string ANTES: %d\n", len);
//   LEN_CHARACTER_OHMs = len<=7?len:0;
//   RESISTOR.ohms = len<=7?R_x:0.0f;
//   printf("Tamanho da string DEPOIS: %d\n", LEN_CHARACTER_OHMs);

//   for (int i = 0; i < 3; i++) {
//     RESISTOR.colors[i] = COLORS[POS_COLOR[i]];
//   }

//   Content_Display();
//   Content_Leds();
// }

void Content_Display() {
    char buffer[7];
    oled_Clear();
    oled_Write_String("OHMIMETRO", 37, 3);
    //=================================================
    oled_Draw_Line_Horizontal(3, 125, 11, true);
    oled_Draw_Line_Vertical(2, 2, 62, true);
    oled_Draw_Line_Vertical(126, 5, 62, true);
    oled_Draw_Line_Horizontal(2, 126, 62, true);
    //=================================================
    oled_Write_String(RESISTOR.colors[0].name, 5, 16);
    oled_Draw_Line_Horizontal(3, 63, 27, true);
    //=================================================
    oled_Write_String(RESISTOR.colors[1].name, 5, 32);
    oled_Draw_Line_Horizontal(3, 63, 43, true);
    //=================================================
    oled_Write_String(RESISTOR.colors[2].name, 5, 48);
    //=================================================
    oled_Draw_Line_Vertical(63,11,61,true);
    //=================================================
    oled_Write_String("VALOR", 61+((128-61)/2)-5*6/2, 20);
    oled_Write_String("Ohms",61+((128-61)/2)-4*6/2, 29);
    sprintf(buffer, "%.2f", RESISTOR.ohms);
    oled_Write_String(buffer, (61+(((128-61)/2)-LEN_CHARACTER_OHMs*6/2)), 45);
    //=================================================
    oled_Update();
}
void Content_Leds() {
  static uint8_t leds[] = {5,14,15,7,12,17,9,10,19,24,23,22,21,20,4,3,2,1,0,6,13,16,8,11,18};
  static uint8_t leds_colors[19][3];
  int index = 0;

  for (int i = 0; i < LED_LENGHT; i++) {
    if ((i == 3 || i == 6)) index++;
    for (int j = 0; j < 3 && i < 9; j++){
      leds_colors[i][j] = RESISTOR.colors[index].rgb[j];
    }
    if (i >= 9){
      leds_colors[i][0] = 0;
      leds_colors[i][1] = 1;
      leds_colors[i][2] = 1;
      // printf("%d\n",leds_colors[i][2]);
    }
  }
  Leds_Map_leds_ON(leds,leds_colors,LED_LENGHT,true);
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

// Lista de cores de 0 a 9
const char* codColors[] = {
    "preto", "marrom", "vermelho", "laranja", "amarelo",
    "verde", "azul", "violeta", "cinza", "branco"
};

// Função que extrai as cores de acordo com o valor em texto
char* GetResistorColors(const char* valorOhms) {
    static char resultado[100];  // Buffer para a resposta final
    char valorLimpo[20] = "";
    char sufixo = '\0';
    double valor = 0.0;
    int i, j = 0;

    // Limpa os caracteres e separa o número do sufixo (k, m, etc)
    for (i = 0; valorOhms[i] != '\0'; i++) {
        if (isdigit(valorOhms[i]) || valorOhms[i] == '.' || valorOhms[i] == '-') {
            valorLimpo[j++] = valorOhms[i];
        } else if (tolower(valorOhms[i]) == 'k' || tolower(valorOhms[i]) == 'm') {
            sufixo = tolower(valorOhms[i]);
        }
    }
    valorLimpo[j] = '\0';

    // Converte string numérica para double
    valor = atof(valorLimpo);

    // Ajusta multiplicador conforme sufixo
    int multiplicador = 0;
    if (sufixo == 'k') {
        multiplicador = 2; // 10^2
    } else if (sufixo == 'm') {
        multiplicador = 5; // 10^5
    }

    // Verifica se tem parte decimal diferente de zero
    int parteDecimal = (valor - (int)valor) * 10;
    if (parteDecimal != 0) {
        valor *= 10;
        multiplicador++;
    }

    // Converte valor para int para extrair dígitos
    int valorInt = (int)round(valor);
    char strValor[20];
    sprintf(strValor, "%d", valorInt);

    int d1 = strValor[0] - '0';                   // Primeiro dígito
    int d2 = (strlen(strValor) > 1) ? strValor[1] - '0' : 0; // Segundo dígito

    // Formata o resultado com as três cores e "dourado"
    snprintf(resultado, sizeof(resultado), "%s, %s, %s e dourado",
             codColors[d1], codColors[d2], codColors[multiplicador]);

    return resultado;
}

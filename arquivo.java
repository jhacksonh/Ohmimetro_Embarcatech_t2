package com.jhacksonh.resistores_codigodecores;

/**
 *
 * @author Jhacksonhexit
 * 
 */
public class Resistores_codigoDeCores {

    public static void main(String[] args) {
        // Chama a função que converte o valor do resistor em texto para as cores correspondentes
        System.out.println("\n" + GetResistorColors("2m ohms")); // Exemplo: 2 mega ohms
    }

    // Função que recebe uma string com o valor do resistor e retorna as cores das faixas
    public static String GetResistorColors(String valorOhms) {
        // Array com o nome das cores conforme os dígitos (0 a 9)
        String[] codColors = {
            "preto", "marrom", "vermelho", "laranja", "amarelo",
            "verde", "azul", "violeta", "cinza", "branco"
        };

        // Remove todos os caracteres que não sejam número, ponto ou sinal de menos
        Double ohms = Double.parseDouble(valorOhms.replaceAll("[^\\d.-]", ""));

        // Transforma o número em string e separa a parte decimal
        // Se houver parte decimal diferente de 0, mantém; senão, usa apenas parte inteira
        String dotRemoved = Integer.parseInt((""+ohms).split("\\.")[1]) != 0 
            ? ("" + ohms) 
            : ("" + ohms).split("\\.")[0];

        // Extrai o primeiro dígito significativo
        int number1 = Integer.parseInt("" + dotRemoved.charAt(0));

        // Extrai o segundo dígito significativo (se houver)
        int number2 = dotRemoved.length() > 1 
            ? Integer.parseInt("" + dotRemoved.charAt(1)) 
            : 0;

        // Inicializa o multiplicador com 0 ou 1 dependendo da quantidade de dígitos
        int number3 = dotRemoved.length() > 2 ? 1 : 0;

        // Verifica se o valor contém "k" (kilo ohms)
        if (valorOhms.split(" ")[0].contains("k")) {
            number3 = 2; // Multiplicador para kΩ (10^2)
        } 
        // Verifica se o valor contém "m" (mega ohms)
        else if (valorOhms.split(" ")[0].contains("m")) {
            number3 = 5; // Multiplicador para MΩ (10^5)
        }

        // Se a parte decimal for diferente de zero, multiplica por 10 e ajusta o expoente
        // Exemplo: 2.2k vira 22 × 10^3 = 22k
        ohms = Integer.parseInt(("" + ohms).split("\\.")[1]) != 0
            ? (ohms * 10) * Math.pow(10, number3)
            : ohms * Math.pow(10, number3);

        // Retorna a string formatada com as 3 faixas de cor e a faixa de tolerância (dourado = 5%)
        return String.format("%s, %s, %s e dourado", 
            codColors[number1], 
            codColors[number2], 
            codColors[number3]);
    }
}

#include "../include/ranking_score.h"
#include <stdio.h>

static int assertEqual(const char *nome, int esperado, int atual) {
    if (esperado != atual) {
        printf("FAIL %s: esperado %d, recebeu %d\n", nome, esperado, atual);
        return 1;
    }
    printf("PASS %s\n", nome);
    return 0;
}

int main(void) {
    int falhas = 0;

    falhas += assertEqual("pontuacao soma de zero para cima", 621, calcularPontuacaoRanking(45.12f, 8));
    falhas += assertEqual("tempo rapido predomina", 692, calcularPontuacaoRanking(25.48f, 6));
    falhas += assertEqual("run lenta com moedas nao domina", 476, calcularPontuacaoRanking(130.07f, 8));
    falhas += assertEqual("tempo zero protegido", 10000, calcularPontuacaoRanking(0.0f, 0));

    return falhas;
}

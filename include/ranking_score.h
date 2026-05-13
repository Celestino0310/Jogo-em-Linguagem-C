#ifndef RANKING_SCORE_H
#define RANKING_SCORE_H

#define PONTOS_TEMPO 10000
#define PONTOS_POR_MOEDA 50

static inline int calcularPontuacaoRanking(float tempo, int moedasPegas) {
    if (tempo <= 0.0f) tempo = 1.0f;
    if (moedasPegas < 0) moedasPegas = 0;

    return (int)(PONTOS_TEMPO / tempo) + moedasPegas * PONTOS_POR_MOEDA;
}

#endif

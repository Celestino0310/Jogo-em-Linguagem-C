#ifndef ESPINHO_H
#define ESPINHO_H

#include <stdbool.h>

// Uma zona de espinhos: cobre de x1 ate x2
// no topo do bloco (base = y do topo do bloco)
typedef struct {
    float x1, x2, y; // y = topo do bloco (base dos espinhos)
} ZonaEspinho;

// Renderiza todos os espinhos da fase atual
void renderEspinhos(int fase);

// Retorna true se o jogador tocou em algum espinho
// px,py = posicao do jogador | pw,ph = meia-largura e altura
bool checaEspinhoColisao(float px, float py,
                          float pw, float ph, int fase);

#endif

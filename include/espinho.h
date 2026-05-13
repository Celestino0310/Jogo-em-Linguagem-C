#ifndef ESPINHO_H
#define ESPINHO_H

#include <stdbool.h>



typedef struct {
    float x1, x2, y; 
} ZonaEspinho;


void renderEspinhos(int fase);



bool checaEspinhoColisao(float px, float py,
                          float pw, float ph, int fase);

#endif

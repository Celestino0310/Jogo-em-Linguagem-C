#ifndef MAPA_H
#define MAPA_H

#include <stdbool.h>

typedef struct { float x1, y1, x2, y2; } Bloco;
typedef struct { float r, g, b; }         Cor;

extern Bloco *blocos;
extern int    numBlocos;
extern int    faseAtual;

void initMapa();
void renderMapa();
void updateMapa();
void avancarFase();
int  indiceSaida();
bool ehEstrutura(int i);

#endif

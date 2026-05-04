#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

void initGame();
void renderGame();
void updateGame();
void handleGameInput(unsigned char tecla);
void handleGameInputUp(unsigned char tecla);
void renderVitoria();
void renderDerrota();
void renderPause();
void handleVitoriaInput(unsigned char tecla);
void handleDerrotaInput(unsigned char tecla);
void handlePauseInput(unsigned char tecla);

extern bool pausado;

#endif

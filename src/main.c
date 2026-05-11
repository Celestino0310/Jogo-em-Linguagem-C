#define GLUT_DISABLE_ATEXIT_HACK
#include "../include/GL/glut.h"
#include "../include/menu.h"
#include "../include/game.h"
#include "../include/audio.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

int   gameState = 0;
float lastTime  = 0;
int largura = 1920;
int altura  = 1080;

void render() {
    if      (gameState == 0)             renderMenu();
    else if (gameState == 1 && !pausado) renderGame();
    else if (gameState == 1 &&  pausado) renderPause();
    else if (gameState == 2)             renderVitoria();
    else if (gameState == 3)             renderDerrota();
}

void keyboardDown(unsigned char tecla, int x, int y) {
    if (tecla == 'm' || tecla == 'M') { audioMutar(); glutPostRedisplay(); return; }
    if      (gameState == 0)             handleMenuInput(tecla);
    else if (gameState == 1 && !pausado) handleGameInput(tecla);
    else if (gameState == 1 &&  pausado) handlePauseInput(tecla);
    else if (gameState == 2)             handleVitoriaInput(tecla);
    else if (gameState == 3)             handleDerrotaInput(tecla);
}

void keyboardUp(unsigned char tecla, int x, int y) {
    if (gameState == 1) handleGameInputUp(tecla);
}

void specialDown(int tecla, int x, int y) {
    if (gameState == 0) handleMenuSpecial(tecla);
}

void update(int valor) {
    if      (gameState == 0) updateMenu();
    else if (gameState == 1) updateGame();
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(200, 200);
    glutCreateWindow("Althea");

    if (audioInit()) audioTocarMusica(MUSICA_1);

    initMenu();
    initGame();

    glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

    glutDisplayFunc(render);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialDown);
    glutMouseFunc(handleMenuMouse);
    glutTimerFunc(16, update, 0);

    glutMainLoop();

    audioFechar();
    return 0;
}

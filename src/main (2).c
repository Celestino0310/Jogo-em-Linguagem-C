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

void render() {
    if (gameState == 0) renderMenu();
    else                renderGame();
}

void keyboardDown(unsigned char tecla, int x, int y) {
    // M = mudo global em qualquer tela
    if (tecla == 'm' || tecla == 'M') {
        audioMutar();
        glutPostRedisplay();
        return;
    }
    if (gameState == 0) handleMenuInput(tecla);
    else                handleGameInput(tecla);
}

void keyboardUp(unsigned char tecla, int x, int y) {
    if (gameState == 1) handleGameInputUp(tecla);
}

void specialDown(int tecla, int x, int y) {
    if (gameState == 0) handleMenuSpecial(tecla);
}

void update(int valor) {
    if (gameState == 0) updateMenu();
    else                updateGame();
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(200, 100);
    glutCreateWindow("Althea");

    // Inicializa audio ANTES de tudo
    if (audioInit()) {
        // Toca musica do menu/fases 1 e 2
        audioTocarMusica(MUSICA_1);
    }

    initMenu();
    initGame();

    glutDisplayFunc(render);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialDown);
    glutTimerFunc(16, update, 0);

    glutMainLoop();

    audioFechar();
    return 0;
}

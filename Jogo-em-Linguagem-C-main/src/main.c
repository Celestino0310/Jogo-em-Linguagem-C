#define GLUT_DISABLE_ATEXIT_HACK
#include "../include/GL/glut.h"
#include "../include/menu.h"
#include "../include/game.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

int   gameState = 0;
float lastTime  = 0;
int largura= 1920;
int altura =1080; 
void render() {
    if (gameState == 0) renderMenu();
    else                renderGame();
}

void keyboardDown(unsigned char tecla, int x, int y) {
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
    glutInitWindowPosition(200, 200);
    glutCreateWindow("Althea");

    initMenu();
    initGame();

    glutDisplayFunc(render);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialDown);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}

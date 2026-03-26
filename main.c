#include "glut.h"
#include <stdbool.h>

// Variáveis do Personagem
int direcao = 1;
float posX = -0.8f, posY = -0.8f; // Posição inicial
float velY = 0.0f;                // Velocidade vertical (pulo)
float gravidade = -0.001f;        // Força que puxa para baixo
bool noChao = true;               // Impede pular no ar

void desenhaPersonagem() {
    glBegin(GL_QUADS);
        glColor3f(0.0f, 1.0f, 0.0f); // Personagem Verde
        glVertex2f(posX - 0.05f, posY - 0.05f);
        glVertex2f(posX + 0.05f, posY - 0.05f);
        glVertex2f(posX + 0.05f, posY + 0.05f);
        glVertex2f(posX - 0.05f, posY + 0.05f);
    glEnd();
}

void teclado(unsigned char tecla, int x, int y) {
    switch (tecla) {
        case 'a': posX -= 0.05f; direcao = -1; break; // Andar Esquerda
        case 'd': posX += 0.05f; direcao=1;  break; // Andar Direita
        
        case 'w': // PULO
            if (noChao) {
                velY = 0.04f; // Força do pulo
                noChao = false;
            }
            break;

        case 'e': // IMPULSO (Dash)
            posX += 0.2f * direcao; // Joga o personagem para frente
            break;
    }
}

void atualiza(int valor) {
    // Aplica Gravidade
    if (!noChao) {
        velY += gravidade; // Diminui a velocidade vertical
        posY += velY;      // Move o personagem
    }

    // Checa colisão com o "Chão" (ajuste o valor conforme seu cenário)
    if (posY <= -0.8f) {
        posY = -0.8f;
        velY = 0;
        noChao = true;
    }

    glutPostRedisplay();
    glutTimerFunc(16, atualiza, 0); // Mantém o loop
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    desenhaPersonagem();
    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Personagem Reativo");
    
    glutDisplayFunc(display);
    glutKeyboardFunc(teclado);
    glutTimerFunc(16, atualiza, 0);
    
    glutMainLoop();
    return 0;
}

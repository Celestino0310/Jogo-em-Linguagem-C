#define GLUT_DISABLE_ATEXIT_HACK
#include "../include/menu.h"
#include "../include/GL/glut.h"
#include <stdlib.h>

extern int gameState;

// === ESTADO DO MENU ===
static int opcaoSelecionada = 0;
static int subTela = 0; // 0=principal, 1=credits
static const int TOTAL_OPCOES = 4;
static const char *opcoes[] = { "START", "OPTIONS", "CREDITS", "EXIT" };

// === HELPER TEXTO ===
static void desenhaTexto(float x, float y, const char *txt, void *fonte) {
    glRasterPos2f(x, y);
    while (*txt) { glutBitmapCharacter(fonte, *txt); txt++; }
}

// === TELA CREDITS ===
static void renderCredits() {
    glClearColor(0.05f, 0.05f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1.0f, 0.85f, 0.0f);
    desenhaTexto(-0.15f, 0.65f, "CREDITS", GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(0.9f, 0.9f, 0.9f);
    desenhaTexto(-0.35f,  0.30f, "Desenvolvido por: Seu Nome",  GLUT_BITMAP_HELVETICA_18);
    desenhaTexto(-0.35f,  0.05f, "Engine: OpenGL / GLUT",       GLUT_BITMAP_HELVETICA_18);
    desenhaTexto(-0.35f, -0.20f, "Audio:  BASS Library",        GLUT_BITMAP_HELVETICA_18);
    desenhaTexto(-0.35f, -0.45f, "Inspirado em: Celeste",       GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.4f, 0.6f, 1.0f);
    desenhaTexto(-0.28f, -0.80f, "ESC = voltar", GLUT_BITMAP_HELVETICA_18);

    glutSwapBuffers();
}

// === TELA OPTIONS ===
static void renderOptions() {
    glClearColor(0.05f, 0.05f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1.0f, 0.85f, 0.0f);
    desenhaTexto(-0.17f, 0.65f, "OPTIONS", GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(0.9f, 0.9f, 0.9f);
    desenhaTexto(-0.40f, 0.20f, "Em breve...", GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.4f, 0.6f, 1.0f);
    desenhaTexto(-0.28f, -0.80f, "ESC = voltar", GLUT_BITMAP_HELVETICA_18);

    glutSwapBuffers();
}

// === MENU PRINCIPAL ===
static void renderMenuPrincipal() {
    glClearColor(0.05f, 0.05f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Titulo
    glColor3f(1.0f, 0.85f, 0.0f);
    desenhaTexto(-0.20f, 0.75f, "ALTHEA", GLUT_BITMAP_TIMES_ROMAN_24);

    // Linha decorativa
    glColor3f(0.25f, 0.25f, 0.55f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        glVertex2f(-0.55f, 0.63f);
        glVertex2f( 0.55f, 0.63f);
    glEnd();

    // Itens
    float y = 0.38f;
    int i;
    for (i = 0; i < TOTAL_OPCOES; i++) {
        if (i == opcaoSelecionada) {
            // fundo highlight
            glColor3f(0.12f, 0.12f, 0.38f);
            glBegin(GL_QUADS);
                glVertex2f(-0.50f, y - 0.055f);
                glVertex2f( 0.50f, y - 0.055f);
                glVertex2f( 0.50f, y + 0.085f);
                glVertex2f(-0.50f, y + 0.085f);
            glEnd();
            // seta
            glColor3f(1.0f, 0.85f, 0.0f);
            desenhaTexto(-0.42f, y, ">", GLUT_BITMAP_HELVETICA_18);
            glColor3f(1.0f, 0.85f, 0.0f);
        } else {
            glColor3f(0.70f, 0.70f, 0.70f);
        }
        desenhaTexto(-0.08f, y, opcoes[i], GLUT_BITMAP_HELVETICA_18);
        y -= 0.23f;
    }

    // Dica
    glColor3f(0.35f, 0.35f, 0.35f);
    desenhaTexto(-0.45f, -0.88f, "W/S ou Setas = navegar   ENTER = selecionar", GLUT_BITMAP_HELVETICA_12);

    glutSwapBuffers();
}

// === PUBLICAS ===
void initMenu() {
    opcaoSelecionada = 0;
    subTela = 0;
}

void renderMenu() {
    if      (subTela == 1) renderOptions();
    else if (subTela == 2) renderCredits();
    else                   renderMenuPrincipal();
}

void updateMenu() { /* animacoes futuras */ }

void handleMenuInput(unsigned char tecla) {
    if (subTela != 0) {
        if (tecla == 27) { subTela = 0; } // ESC volta
        return;
    }
    switch (tecla) {
        case 'w': case 'W':
            opcaoSelecionada = (opcaoSelecionada - 1 + TOTAL_OPCOES) % TOTAL_OPCOES;
            break;
        case 's': case 'S':
            opcaoSelecionada = (opcaoSelecionada + 1) % TOTAL_OPCOES;
            break;
        case '\r': case '\n':
            if (opcaoSelecionada == 0) gameState = 1;       // START
            if (opcaoSelecionada == 1) subTela = 1;          // OPTIONS
            if (opcaoSelecionada == 2) subTela = 2;          // CREDITS
            if (opcaoSelecionada == 3) exit(0);              // EXIT
            break;
        case 27:
            exit(0);
            break;
    }
}

void handleMenuSpecial(int tecla) {
    if (subTela != 0) return;
    if (tecla == GLUT_KEY_UP)   opcaoSelecionada = (opcaoSelecionada - 1 + TOTAL_OPCOES) % TOTAL_OPCOES;
    if (tecla == GLUT_KEY_DOWN) opcaoSelecionada = (opcaoSelecionada + 1) % TOTAL_OPCOES;
}

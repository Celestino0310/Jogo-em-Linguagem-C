#define GLUT_DISABLE_ATEXIT_HACK
#include "../include/menu.h"
#include "../include/audio.h"
#include "../include/GL/glut.h"
#include <stdlib.h>
#include <stdio.h>

extern int gameState;

// === ESTADO DO MENU ===
static int opcaoSelecionada = 0;
static int subTela = 0; // 0=principal, 1=options, 2=credits
static const int TOTAL_OPCOES = 4;
static const char *opcoes[] = { "START", "OPTIONS", "CREDITS", "EXIT" };

// === HELPER TEXTO ===
static void desenhaTexto(float x, float y, const char *txt, void *fonte) {
    glRasterPos2f(x, y);
    while (*txt) { glutBitmapCharacter(fonte, *txt); txt++; }
}

// Desenha barra de volume estilizada
static void desenhaBarra(float x, float y, float largura, float valor,
                          float r, float g, float b) {
    // fundo da barra
    glColor3f(0.15f, 0.15f, 0.25f);
    glBegin(GL_QUADS);
        glVertex2f(x,          y);
        glVertex2f(x+largura,  y);
        glVertex2f(x+largura,  y+0.05f);
        glVertex2f(x,          y+0.05f);
    glEnd();
    // preenchimento
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
        glVertex2f(x,                   y+0.003f);
        glVertex2f(x+largura*valor,     y+0.003f);
        glVertex2f(x+largura*valor,     y+0.047f);
        glVertex2f(x,                   y+0.047f);
    glEnd();
    // borda
    glColor3f(r*0.6f, g*0.6f, b*0.6f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(x,         y);
        glVertex2f(x+largura, y);
        glVertex2f(x+largura, y+0.05f);
        glVertex2f(x,         y+0.05f);
    glEnd();
    // cursor
    float cx = x + largura*valor;
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
        glVertex2f(cx-0.008f, y-0.008f);
        glVertex2f(cx+0.008f, y-0.008f);
        glVertex2f(cx+0.008f, y+0.058f);
        glVertex2f(cx-0.008f, y+0.058f);
    glEnd();
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

// === TELA OPTIONS (com controle de audio) ===
static void renderOptions() {
    glClearColor(0.05f, 0.05f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Titulo
    glColor3f(1.0f, 0.85f, 0.0f);
    desenhaTexto(-0.17f, 0.72f, "OPTIONS", GLUT_BITMAP_TIMES_ROMAN_24);

    // Linha decorativa
    glColor3f(0.25f, 0.25f, 0.55f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        glVertex2f(-0.55f, 0.60f); glVertex2f(0.55f, 0.60f);
    glEnd();

    // --- VOLUME GERAL ---
    glColor3f(0.9f, 0.9f, 0.9f);
    desenhaTexto(-0.50f, 0.40f, "Volume Geral:", GLUT_BITMAP_HELVETICA_18);

    float vg = audioGetVolume();
    desenhaBarra(-0.50f, 0.20f, 1.0f, vg, 0.3f, 0.6f, 1.0f);

    // percentual
    char buf[32];
    sprintf(buf, "%d%%", (int)(vg * 100));
    glColor3f(0.7f, 0.9f, 1.0f);
    desenhaTexto(0.55f, 0.22f, buf, GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.5f, 0.5f, 0.5f);
    desenhaTexto(-0.50f, 0.10f, "A/D ou Setas Esq/Dir = ajustar",
                 GLUT_BITMAP_HELVETICA_12);

    // --- VOLUME MUSICA ---
    glColor3f(0.9f, 0.9f, 0.9f);
    desenhaTexto(-0.50f, -0.10f, "Volume Musica:", GLUT_BITMAP_HELVETICA_18);

    float vm = audioGetVolumMusica();
    desenhaBarra(-0.50f, -0.30f, 1.0f, vm, 0.8f, 0.4f, 1.0f);

    sprintf(buf, "%d%%", (int)(vm * 100));
    glColor3f(0.9f, 0.7f, 1.0f);
    desenhaTexto(0.55f, -0.28f, buf, GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.5f, 0.5f, 0.5f);
    desenhaTexto(-0.50f, -0.42f, "W/S ou Setas Cima/Baixo = ajustar",
                 GLUT_BITMAP_HELVETICA_12);

    // --- MUDO ---
    bool mudo = audioGetMutado();
    if (mudo) {
        glColor3f(1.0f, 0.3f, 0.3f);
        desenhaTexto(-0.18f, -0.58f, "[ MUDO ATIVO ]", GLUT_BITMAP_HELVETICA_18);
    } else {
        glColor3f(0.3f, 1.0f, 0.5f);
        desenhaTexto(-0.18f, -0.58f, "[ SOM LIGADO ]", GLUT_BITMAP_HELVETICA_18);
    }
    glColor3f(0.5f, 0.5f, 0.7f);
    desenhaTexto(-0.22f, -0.68f, "M = alternar mudo", GLUT_BITMAP_HELVETICA_12);

    // Voltar
    glColor3f(0.4f, 0.6f, 1.0f);
    desenhaTexto(-0.30f, -0.88f, "ESC = voltar ao menu", GLUT_BITMAP_HELVETICA_18);

    glutSwapBuffers();
}

// === MENU PRINCIPAL ===
static void renderMenuPrincipal() {
    glClearColor(0.05f, 0.05f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1.0f, 0.85f, 0.0f);
    desenhaTexto(-0.20f, 0.75f, "ALTHEA", GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(0.25f, 0.25f, 0.55f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        glVertex2f(-0.55f, 0.63f); glVertex2f(0.55f, 0.63f);
    glEnd();

    float y = 0.38f;
    int i;
    for (i = 0; i < TOTAL_OPCOES; i++) {
        if (i == opcaoSelecionada) {
            glColor3f(0.12f, 0.12f, 0.38f);
            glBegin(GL_QUADS);
                glVertex2f(-0.50f, y-0.055f); glVertex2f(0.50f, y-0.055f);
                glVertex2f( 0.50f, y+0.085f); glVertex2f(-0.50f, y+0.085f);
            glEnd();
            glColor3f(1.0f, 0.85f, 0.0f);
            desenhaTexto(-0.42f, y, ">", GLUT_BITMAP_HELVETICA_18);
            glColor3f(1.0f, 0.85f, 0.0f);
        } else {
            glColor3f(0.70f, 0.70f, 0.70f);
        }
        desenhaTexto(-0.08f, y, opcoes[i], GLUT_BITMAP_HELVETICA_18);
        y -= 0.23f;
    }

    // Indicador de mudo no menu principal
    if (audioGetMutado()) {
        glColor3f(1.0f, 0.3f, 0.3f);
        desenhaTexto(0.65f, 0.88f, "[MUDO]", GLUT_BITMAP_HELVETICA_12);
    }

    glColor3f(0.35f, 0.35f, 0.35f);
    desenhaTexto(-0.45f, -0.88f,
                 "W/S = navegar   ENTER = selecionar   M = mudo",
                 GLUT_BITMAP_HELVETICA_12);

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

void updateMenu() {}

void handleMenuInput(unsigned char tecla) {
    // M = mudo em qualquer sub-tela
    if (tecla == 'm' || tecla == 'M') {
        audioMutar();
        glutPostRedisplay();
        return;
    }

    if (subTela != 0) {
        // Controles da tela de OPTIONS
        if (subTela == 1) {
            float step = 0.05f;
            float vg, vm;
            switch (tecla) {
                case 'a': case 'A': // volume geral -
                    vg = audioGetVolume() - step;
                    audioSetVolume(vg < 0.0f ? 0.0f : vg);
                    break;
                case 'd': case 'D': // volume geral +
                    vg = audioGetVolume() + step;
                    audioSetVolume(vg > 1.0f ? 1.0f : vg);
                    break;
                case 'w': case 'W': // volume musica +
                    vm = audioGetVolumMusica() + step;
                    audioSetVolumMusica(vm > 1.0f ? 1.0f : vm);
                    break;
                case 's': case 'S': // volume musica -
                    vm = audioGetVolumMusica() - step;
                    audioSetVolumMusica(vm < 0.0f ? 0.0f : vm);
                    break;
            }
        }
        if (tecla == 27) { subTela = 0; }
        glutPostRedisplay();
        return;
    }

    switch (tecla) {
        case 'w': case 'W':
            opcaoSelecionada = (opcaoSelecionada-1+TOTAL_OPCOES)%TOTAL_OPCOES;
            break;
        case 's': case 'S':
            opcaoSelecionada = (opcaoSelecionada+1)%TOTAL_OPCOES;
            break;
        case '\r': case '\n':
            if (opcaoSelecionada == 0) gameState = 1;
            if (opcaoSelecionada == 1) subTela   = 1;
            if (opcaoSelecionada == 2) subTela   = 2;
            if (opcaoSelecionada == 3) { audioFechar(); exit(0); }
            break;
        case 27:
            audioFechar(); exit(0);
            break;
    }
    glutPostRedisplay();
}

void handleMenuSpecial(int tecla) {
    if (subTela == 1) {
        float step = 0.05f;
        float vg, vm;
        switch (tecla) {
            case GLUT_KEY_LEFT:  // volume geral -
                vg = audioGetVolume() - step;
                audioSetVolume(vg < 0.0f ? 0.0f : vg);
                break;
            case GLUT_KEY_RIGHT: // volume geral +
                vg = audioGetVolume() + step;
                audioSetVolume(vg > 1.0f ? 1.0f : vg);
                break;
            case GLUT_KEY_UP:    // volume musica +
                vm = audioGetVolumMusica() + step;
                audioSetVolumMusica(vm > 1.0f ? 1.0f : vm);
                break;
            case GLUT_KEY_DOWN:  // volume musica -
                vm = audioGetVolumMusica() - step;
                audioSetVolumMusica(vm < 0.0f ? 0.0f : vm);
                break;
        }
        glutPostRedisplay();
        return;
    }
    if (subTela != 0) return;
    if (tecla == GLUT_KEY_UP)
        opcaoSelecionada = (opcaoSelecionada-1+TOTAL_OPCOES)%TOTAL_OPCOES;
    if (tecla == GLUT_KEY_DOWN)
        opcaoSelecionada = (opcaoSelecionada+1)%TOTAL_OPCOES;
    glutPostRedisplay();
}

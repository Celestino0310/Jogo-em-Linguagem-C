#define GLUT_DISABLE_ATEXIT_HACK
#include "../include/menu.h"
#include "../include/audio.h"
#include "../include/galeria_screenshot.h"
#include "../include/GL/glut.h"
#include <stdlib.h>
#include <stdio.h>

extern int gameState;


static int opcaoSelecionada = 0;
static int subTela = 0; 
static const int TOTAL_OPCOES = 5;
static const char *opcoes[] = { "START", "OPTIONS", "SCREENSHOTS","CREDITS", "EXIT" };


static void desenhaTexto(float x, float y, const char *txt, void *fonte) {
    glRasterPos2f(x, y);
    while (*txt) { glutBitmapCharacter(fonte, *txt); txt++; }
}


static void desenhaBarra(float x, float y, float largura, float valor,float r, float g, float b) {
    
    glColor3f(0.15f, 0.15f, 0.25f);
    glBegin(GL_QUADS);
        glVertex2f(x,          y);
        glVertex2f(x+largura,  y);
        glVertex2f(x+largura,  y+0.05f);
        glVertex2f(x,          y+0.05f);
    glEnd();
    
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
        glVertex2f(x,                   y+0.003f);
        glVertex2f(x+largura*valor,     y+0.003f);
        glVertex2f(x+largura*valor,     y+0.047f);
        glVertex2f(x,                   y+0.047f);
    glEnd();
    
    glColor3f(r*0.6f, g*0.6f, b*0.6f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(x,         y);
        glVertex2f(x+largura, y);
        glVertex2f(x+largura, y+0.05f);
        glVertex2f(x,         y+0.05f);
    glEnd();
    
    float cx = x + largura*valor;
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
        glVertex2f(cx-0.008f, y-0.008f);
        glVertex2f(cx+0.008f, y-0.008f);
        glVertex2f(cx+0.008f, y+0.058f);
        glVertex2f(cx-0.008f, y+0.058f);
    glEnd();
}
static void renderCredits() {

    glClearColor(0.05f, 0.05f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1.0f, 0.85f, 0.0f);

    desenhaTexto(
        -0.12f,
         0.88f,
        "CREDITS",
        GLUT_BITMAP_TIMES_ROMAN_24
    );

    
    glColor3f(0.25f, 0.25f, 0.55f);

    glLineWidth(2.0f);

    glBegin(GL_LINES);
        glVertex2f(-0.60f, 0.78f);
        glVertex2f( 0.60f, 0.78f);
    glEnd();



    float y = 0.58f;

    
    glColor3f(0.90f, 0.90f, 0.90f);

    desenhaTexto(
        -0.48f,
         y,
        "Creative Director & Lead Map Designer",
        GLUT_BITMAP_HELVETICA_18
    );

    
    glColor3f(0.65f, 0.75f, 1.0f);

    desenhaTexto(
         0.12f,
         y,
        "Gabriel Celestino",
        GLUT_BITMAP_HELVETICA_18
    );

    y -= 0.14f;

    glColor3f(0.90f, 0.90f, 0.90f);

    desenhaTexto(
        -0.48f,
         y,
        "Gameplay & Movement Programmer",
        GLUT_BITMAP_HELVETICA_18
    );

    glColor3f(0.65f, 0.75f, 1.0f);

    desenhaTexto(
         0.12f,
         y,
        "Gabriel Neves",
        GLUT_BITMAP_HELVETICA_18
    );

    y -= 0.14f;

    glColor3f(0.90f, 0.90f, 0.90f);

    desenhaTexto(
        -0.48f,
         y,
        "Animation Designer & Visual Effects",
        GLUT_BITMAP_HELVETICA_18
    );

    glColor3f(0.65f, 0.75f, 1.0f);

    desenhaTexto(
         0.12f,
         y,
        "Diogo Bassalobre",
        GLUT_BITMAP_HELVETICA_18
    );

    y -= 0.14f;

    glColor3f(0.90f, 0.90f, 0.90f);

    desenhaTexto(
        -0.48f,
         y,
        "Lead Gameplay Developer",
        GLUT_BITMAP_HELVETICA_18
    );

    glColor3f(0.65f, 0.75f, 1.0f);

    desenhaTexto(
         0.12f,
         y,
        "Igor de Abreu",
        GLUT_BITMAP_HELVETICA_18
    );

    y -= 0.14f;

    glColor3f(0.90f, 0.90f, 0.90f);

    desenhaTexto(
        -0.48f,
         y,
        "Save System & Technical Support",
        GLUT_BITMAP_HELVETICA_18
    );

    glColor3f(0.65f, 0.75f, 1.0f);

    desenhaTexto(
         0.12f,
         y,
        "Tiago Coquinho",
        GLUT_BITMAP_HELVETICA_18
    );

    
    
    

    glColor3f(0.20f, 0.20f, 0.45f);

    glBegin(GL_LINES);
        glVertex2f(-0.55f, -0.20f);
        glVertex2f( 0.55f, -0.20f);
    glEnd();


    glColor3f(0.55f, 0.65f, 0.95f);

    desenhaTexto(
        -0.45f,
        -0.38f,
        "Graphics Engine:",
        GLUT_BITMAP_HELVETICA_18
    );

    glColor3f(0.80f, 0.80f, 0.80f);

    desenhaTexto(
        -0.02f,
        -0.38f,
        "OpenGL / GLUT",
        GLUT_BITMAP_HELVETICA_18
    );

    glColor3f(0.55f, 0.65f, 0.95f);

    desenhaTexto(
        -0.45f,
        -0.52f,
        "Audio System:",
        GLUT_BITMAP_HELVETICA_18
    );

    glColor3f(0.80f, 0.80f, 0.80f);

    desenhaTexto(
        -0.02f,
        -0.52f,
        "BASS Audio Library",
        GLUT_BITMAP_HELVETICA_18
    );

    glColor3f(0.55f, 0.65f, 0.95f);

    desenhaTexto(
        -0.45f,
        -0.66f,
        "Inspired By:",
        GLUT_BITMAP_HELVETICA_18
    );

    glColor3f(0.80f, 0.80f, 0.80f);

    desenhaTexto(
        -0.02f,
        -0.66f,
        "Celeste",
        GLUT_BITMAP_HELVETICA_18
    );

 
    glColor3f(0.40f, 0.60f, 1.0f);
    desenhaTexto(
        -0.16f,
        -0.88f,
        "ESC = Return",
        GLUT_BITMAP_HELVETICA_18
    );

    glutSwapBuffers();
}

static void renderOptions() {
    glClearColor(0.05f, 0.05f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    
    glColor3f(1.0f, 0.85f, 0.0f);
    desenhaTexto(-0.17f, 0.72f, "OPTIONS", GLUT_BITMAP_TIMES_ROMAN_24);

    
    glColor3f(0.25f, 0.25f, 0.55f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        glVertex2f(-0.55f, 0.60f); glVertex2f(0.55f, 0.60f);
    glEnd();

    
    glColor3f(0.9f, 0.9f, 0.9f);
    desenhaTexto(-0.50f, 0.40f, "Volume Geral:", GLUT_BITMAP_HELVETICA_18);

    float vg = audioGetVolume();
    desenhaBarra(-0.50f, 0.20f, 1.0f, vg, 0.3f, 0.6f, 1.0f);

    
    char buf[32];
    sprintf(buf, "%d%%", (int)(vg * 100));
    glColor3f(0.7f, 0.9f, 1.0f);
    desenhaTexto(0.55f, 0.22f, buf, GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.5f, 0.5f, 0.5f);
    desenhaTexto(-0.50f, 0.10f, "A/D ou Setas Esq/Dir = ajustar",
                 GLUT_BITMAP_HELVETICA_12);

    
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

    
    glColor3f(0.4f, 0.6f, 1.0f);
    desenhaTexto(-0.30f, -0.88f, "ESC = voltar ao menu", GLUT_BITMAP_HELVETICA_18);

    glutSwapBuffers();
}


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


void initMenu() {
    opcaoSelecionada = 0;
    subTela = 0;
}


void renderMenu() {
    if      (subTela == 1) renderOptions();
    else if (subTela == 2) desenharGaleriaScreenshots();
    else if (subTela == 3) renderCredits();
    else                   renderMenuPrincipal();
}

void updateMenu() {
    if (subTela == 3) atualizarMenuScreenshots();
}
void handleMenuInput(unsigned char tecla) {
    
    if (tecla == 'm' || tecla == 'M') {
        audioMutar();
        glutPostRedisplay();
        return;
    }

    if (subTela != 0) {
        
        if (subTela == 1) {
            float step = 0.05f;
            float vg, vm;
            switch (tecla) {
                case 'a': case 'A': 
                    vg = audioGetVolume() - step;
                    audioSetVolume(vg < 0.0f ? 0.0f : vg);
                    break;
                case 'd': case 'D': 
                    vg = audioGetVolume() + step;
                    audioSetVolume(vg > 1.0f ? 1.0f : vg);
                    break;
                case 'w': case 'W': 
                    vm = audioGetVolumMusica() + step;
                    audioSetVolumMusica(vm > 1.0f ? 1.0f : vm);
                    break;
                case 's': case 'S': 
                    vm = audioGetVolumMusica() - step;
                    audioSetVolumMusica(vm < 0.0f ? 0.0f : vm);
                    break;
            }
        }
        if (subTela == 2) {
            



            if (tecla == 'a' || tecla == 'A') galeriaScreenshotAnterior();
            if (tecla == 'd' || tecla == 'D') galeriaScreenshotProxima();
            if (tecla == 27) { liberarTexturasScreenshots(); subTela = 0; }
        } else if (tecla == 27) {
            subTela = 0; 
    	}
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
            if (opcaoSelecionada == 2) { carregarScreenshots();subTela   = 2;}
            if (opcaoSelecionada == 3) subTela   = 3;
            if (opcaoSelecionada == 4) { audioFechar(); exit(0); }
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
            case GLUT_KEY_LEFT:  
                vg = audioGetVolume() - step;
                audioSetVolume(vg < 0.0f ? 0.0f : vg);
                break;
            case GLUT_KEY_RIGHT: 
                vg = audioGetVolume() + step;
                audioSetVolume(vg > 1.0f ? 1.0f : vg);
                break;
            case GLUT_KEY_UP:    
                vm = audioGetVolumMusica() + step;
                audioSetVolumMusica(vm > 1.0f ? 1.0f : vm);
                break;
            case GLUT_KEY_DOWN:  
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

void handleMenuMouse(int button, int state, int x, int y) {
    if (subTela != 0) return;
    if (button != GLUT_LEFT_BUTTON || state != GLUT_UP) return;
    int alturaJanela = glutGet(GLUT_WINDOW_HEIGHT);
    float oy = 1.0f - 2.0f * ((float)y / alturaJanela);
    float centros[] = { 0.38f, 0.15f, -0.08f, -0.31f };
    int i;
    for (i = 0; i < TOTAL_OPCOES; i++) {
        if (oy >= centros[i]-0.055f && oy <= centros[i]+0.085f) {
            opcaoSelecionada = i;
            if (i==0) gameState=1;
            if (i==1) subTela=1;
            if (i==2) subTela=2;
            if (i==3) subTela=3;
            if (i==4) { audioFechar(); exit(0); }
            glutPostRedisplay();
            return;
        }
    }
}

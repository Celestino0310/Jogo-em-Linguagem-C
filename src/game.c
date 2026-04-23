#define GLUT_DISABLE_ATEXIT_HACK
#include "../include/game.h"
#include "../include/mapa.h"
#include "../include/GL/glut.h"
#include <stdbool.h>
#include <stdlib.h>

extern int gameState;
static bool vitoriaFinal = false;
bool pausado = false;
static int  opcaoPause = 0;

// ============================================================
// JOGADOR
// ============================================================
static int   Health       = 3;
static float posX         = -0.75f;
static float posY         = -0.80f;
static float velX         =  0.0f;
static float velY         =  0.0f;
static float gravidade    = -0.002f;
static bool  noChao       = true;
static bool  podeDash     = true;
static int   direcao      =  1;
static float tempoDash    =  0.0f;
static float cooldownDash =  3.0f;
static bool  kA=false, kD=false, kW=false, kSpace=false;

// ============================================================
// COLISAO
// ============================================================
#define PW 0.055f
#define PH 0.090f

static bool colideCom(Bloco b){
    return posX+PW>b.x1 && posX-PW<b.x2 &&
           posY+PH>b.y1 && posY<b.y2;
}
static void resolveColisoes(){
    int i;
	noChao=false;
    float atrito=(faseAtual==2)?0.05f:0.28f;
    for(i=0;i<numBlocos;i++){
        if(!colideCom(blocos[i])) continue;
        Bloco b=blocos[i];
        float oB=b.y2-posY;
        float oC=(posY+PH)-b.y1;
        float oD=(posX+PW)-b.x1;
        float oE=b.x2-(posX-PW);
        float mH=oB<oC?oB:oC;
        float mV=oD<oE?oD:oE;
        if(mH<mV){
            if(oB<oC){ posY=b.y2; velY=0; noChao=true; podeDash=true; }
            else      { posY=b.y1-PH; velY=0; }
        } else {
            if(oD<oE) posX-=oD; else posX+=oE;
            velX=0;
        }
    }
    if(noChao) velX*=(1.0f-atrito);
}

static void checaSaida(){
    int idx=indiceSaida();
    if(idx<0||idx>=numBlocos) return;
    Bloco s=blocos[idx];
    if(posX+PW>s.x1 && posX-PW<s.x2 &&
   		posY+PH>s.y1 && posY<s.y2){
        if(faseAtual<3){
            vitoriaFinal = false;
             kA=false; kD=false; kW=false; kSpace=false;
    		gameState = 2;   
        } else {
            vitoriaFinal = true;
             kA=false; kD=false; kW=false; kSpace=false;
    		gameState = 2;   
        }
        return;
    }
}

// ============================================================
// PUBLICAS
// ============================================================
void initGame(){
    Health=3;
    posX=-0.75f; posY=-0.80f;
    velX=0; velY=0; noChao=true; podeDash=true; tempoDash=0;
    kA=kD=kW=kSpace=false;
    initMapa(); // inicializa o mapa
}

void renderPause() {
    // Desenha o jogo por baixo
    renderMapa();
    
    // Overlay escuro semitransparente
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    glBegin(GL_QUADS);
        glVertex2f(-1,-1); glVertex2f(1,-1);
        glVertex2f(1,1);   glVertex2f(-1,1);
    glEnd();
    glDisable(GL_BLEND);

    // Titulo
    glColor3f(1.0f, 0.85f, 0.0f);
    glRasterPos2f(-0.12f, 0.40f);
    const char *t = "PAUSE";
    while (*t) { glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *t); t++; }

    // Opcao 0 - Continue
    if (opcaoPause == 0) { glColor3f(1.0f, 0.85f, 0.0f); } else { glColor3f(0.6f, 0.6f, 0.6f); }
    glRasterPos2f(-0.20f, 0.10f);
    const char *op0 = "> CONTINUAR";
    while (*op0) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *op0); op0++; }

    // Opcao 1 - Menu
    if (opcaoPause == 1) { glColor3f(1.0f, 0.85f, 0.0f); } else { glColor3f(0.6f, 0.6f, 0.6f); }
    glRasterPos2f(-0.20f, -0.10f);
    const char *op1 = "  MENU PRINCIPAL";
    while (*op1) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *op1); op1++; }

    glColor3f(0.4f, 0.4f, 0.4f);
    glRasterPos2f(-0.40f, -0.80f);
    const char *h = "W/S = navegar   ENTER = confirmar   ESC = continuar";
    while (*h) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *h); h++; }

    glutSwapBuffers();
}

void renderGame(){
    renderMapa(); // desenha fundo + blocos

    // personagem
    glColor3f(0.6f,0.2f,0.8f);
    glBegin(GL_QUADS);
        glVertex2f(posX-(PW-0.025f),posY);
        glVertex2f(posX+(PW-0.025f),posY);
        glVertex2f(posX+(PW-0.025f),posY+PH);
        glVertex2f(posX-(PW-0.025f),posY+PH);
    glEnd();
    glColor3f(0,0,0); glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(posX-(PW-0.025f),posY); glVertex2f(posX+(PW-0.025f),posY);
        glVertex2f(posX+(PW-0.025f),posY+PH); glVertex2f(posX-(PW-0.025f),posY+PH);
    glEnd();
    // cabeca
    glColor3f(1.0f,0.8f,0.6f);
    glBegin(GL_QUADS);
        glVertex2f(posX-0.03f,posY+0.09f); glVertex2f(posX+0.03f,posY+0.09f);
        glVertex2f(posX+0.03f,posY+0.14f); glVertex2f(posX-0.03f,posY+0.14f);
    glEnd();
    // olho
    float ox=(direcao>0)?posX+0.017f:posX-0.017f;
    glColor3f(0,0,0);
    glBegin(GL_QUADS);
        glVertex2f(ox-0.009f,posY+0.119f); glVertex2f(ox+0.009f,posY+0.119f);
        glVertex2f(ox+0.009f,posY+0.125f); glVertex2f(ox-0.009f,posY+0.125f);
    glEnd();
    // braco
    float oax=(direcao>0)?posX+0.015f:posX-0.015f;
    glColor3f(0.3f,0.0f,0.5f);
    glBegin(GL_QUADS);
        glVertex2f(oax-0.009f,posY+0.018f); glVertex2f(oax+0.009f,posY+0.018f);
        glVertex2f(oax+0.009f,posY+0.068f); glVertex2f(oax-0.009f,posY+0.068f);
    glEnd();

    // HUD vidas
    int h;
    for(h=0;h<3;h++){
        float cor=(h<Health)?0.9f:0.2f;
        glColor3f(cor,0.1f,0.1f);
        glBegin(GL_QUADS);
            glVertex2f(-0.98f+h*0.09f,0.88f); glVertex2f(-0.90f+h*0.09f,0.88f);
            glVertex2f(-0.90f+h*0.09f,0.95f); glVertex2f(-0.98f+h*0.09f,0.95f);
        glEnd();
        glColor3f(0.5f,0.0f,0.0f); glLineWidth(1.0f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(-0.98f+h*0.09f,0.88f); glVertex2f(-0.90f+h*0.09f,0.88f);
            glVertex2f(-0.90f+h*0.09f,0.95f); glVertex2f(-0.98f+h*0.09f,0.95f);
        glEnd();
    }

    // HUD cooldown dash
    if(!podeDash){
        float prog=tempoDash/cooldownDash;
        glColor3f(0.2f,0.2f,0.5f);
        glBegin(GL_QUADS);
            glVertex2f(-0.98f,0.80f); glVertex2f(-0.98f+prog*0.38f,0.80f);
            glVertex2f(-0.98f+prog*0.38f,0.84f); glVertex2f(-0.98f,0.84f);
        glEnd();
        glColor3f(0.5f,0.5f,1.0f);
        glRasterPos2f(-0.98f,0.85f);
        const char *dc="DASH"; while(*dc){glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,*dc);dc++;}
    }

    // nome fase
    const char *nomes[4]={"Mapa 1 - Floresta","Mapa 2 - Cidade","Mapa 3 - Geleiras","Mapa Final - Pico"};
    glColor3f(0.5f,0.6f,0.9f);
    glRasterPos2f(0.25f,0.92f);
    const char *fn=nomes[faseAtual]; while(*fn){glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,*fn);fn++;}

    // dica
    glColor3f(0.20f,0.22f,0.35f);
    glRasterPos2f(-0.98f,-0.96f);
    const char *hud="WASD/SPACE=mover/pular  Q=dash(3s)  ESC=menu";
    while(*hud){glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,*hud);hud++;}

    glutSwapBuffers();
}

void updateGame(){
	if (pausado) return;
    float lerp=(faseAtual==2)?0.10f:0.30f;
    float velAlvo=0;
    if(kA){velAlvo=-0.025f;direcao=-1;}
    if(kD){velAlvo= 0.025f;direcao= 1;}
    velX+=(velAlvo-velX)*lerp;

    if((kW||kSpace)&&noChao){
        velY=0.048f; noChao=false; kW=false; kSpace=false;
    }

    velY+=gravidade;
    if(velY<-0.06f) velY=-0.06f;

    posX+=velX; posY+=velY;
	checaSaida();
    resolveColisoes();
    updateMapa();

    if(!podeDash){
        tempoDash+=0.016f;
        if(tempoDash>=cooldownDash){ podeDash=true; tempoDash=0.0f; }
    }

    if(posY<-1.2f){
        Health--;
        if(Health<=0){ gameState=3; return; }
        posX=-0.75f; posY=-0.70f; velX=0; velY=0; noChao=true;
    }
}

void handleGameInput(unsigned char tecla){
    switch(tecla){
        case 'a':case 'A': kA=true;    break;
        case 'd':case 'D': kD=true;    break;
        case 'w':case 'W': kW=true;    break;
        case ' ':          kSpace=true; break;
        case 'q':case 'Q':
            if(podeDash){ velX=0.13f*direcao; velY=0.01f; podeDash=false; tempoDash=0.0f; }
            break;
        case 'p': case 'P':
            pausado = !pausado;
            opcaoPause = 0;
            break;
        case 27:
            if (pausado) { pausado = false; opcaoPause = 0; }
            else         { gameState = 0; initGame(); }
            break;
    }
}

void handleGameInputUp(unsigned char tecla){
    switch(tecla){
        case 'a':case 'A': kA=false;    break;
        case 'd':case 'D': kD=false;    break;
        case 'w':case 'W': kW=false;    break;
        case ' ':          kSpace=false; break;
    }
}

static int opcaoVitoria = 0;  // 0=próxima fase (ou menu), 1=menu

void renderVitoria() {
    glClearColor(0.02f, 0.12f, 0.02f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (vitoriaFinal) {
        glColor3f(1.0f, 0.85f, 0.0f);
        glRasterPos2f(-0.30f, 0.55f);
        const char *t = "VOCE VENCEU!";
        while (*t) { glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *t); t++; }

        glColor3f(0.7f, 1.0f, 0.7f);
        glRasterPos2f(-0.40f, 0.30f);
        const char *s = "Parabens! Você completou o jogo!";
        while (*s) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *s); s++; }
    } else {
        glColor3f(1.0f, 0.85f, 0.0f);
        glRasterPos2f(-0.28f, 0.55f);
        const char *t = "FASE CONCLUÍDA!";
        while (*t) { glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *t); t++; }
    }

    // Opção 0
    if (opcaoVitoria == 0) { glColor3f(1.0f, 0.85f, 0.0f); } else { glColor3f(0.6f, 0.6f, 0.6f); }
    glRasterPos2f(-0.25f, 0.0f);
    const char *op0 = vitoriaFinal ? "> MENU PRINCIPAL" : "> PROXIMA FASE";
    while (*op0) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *op0); op0++; }

    // Opção 1
    if (opcaoVitoria == 1) { glColor3f(1.0f, 0.85f, 0.0f); } else { glColor3f(0.6f, 0.6f, 0.6f); }
    glRasterPos2f(-0.25f, -0.20f);
    const char *op1 = "  MENU PRINCIPAL";
    if (!vitoriaFinal) { while (*op1) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *op1); op1++; } }

    glColor3f(0.3f, 0.5f, 0.3f);
    glRasterPos2f(-0.40f, -0.80f);
    const char *h = "W/S = navegar   ENTER = confirmar";
    while (*h) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *h); h++; }

    glutSwapBuffers();
}

void handleVitoriaInput(unsigned char tecla) {
    int totalOpcoes = vitoriaFinal ? 1 : 2;
    switch (tecla) {
        case 'w': case 'W':
            opcaoVitoria = (opcaoVitoria - 1 + totalOpcoes) % totalOpcoes;
            break;
        case 's': case 'S':
            opcaoVitoria = (opcaoVitoria + 1) % totalOpcoes;
            break;
        case '\r': case '\n':
            if (vitoriaFinal || opcaoVitoria == 1) {
                // Voltar ao menu principal
                gameState = 0;
                initGame();
            } else {
                // Proxima fase
                avancarFase();
                posX = -0.75f; posY = -0.70f; velX = 0; velY = 0; noChao = true;
                Health = 3;
                gameState = 1;
            }
            opcaoVitoria = 0;
            break;
        case 27:
            gameState = 0; initGame(); opcaoVitoria = 0;
            break;
    }
}

// ============================================================
// TELA DE DERROTA
// ============================================================
static int opcaoDerrota = 0;  // 0=tentar de novo, 1=menu

void renderDerrota() {
    glClearColor(0.12f, 0.02f, 0.02f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1.0f, 0.2f, 0.2f);
    glRasterPos2f(-0.22f, 0.55f);
    const char *t = "GAME OVER";
    while (*t) { glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *t); t++; }

    glColor3f(0.8f, 0.5f, 0.5f);
    glRasterPos2f(-0.30f, 0.28f);
    const char *s = "Suas vidas acabaram...";
    while (*s) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *s); s++; }

    // Opção 0
    if (opcaoDerrota == 0) { glColor3f(1.0f, 0.3f, 0.3f); } else { glColor3f(0.6f, 0.6f, 0.6f); }
    glRasterPos2f(-0.25f, 0.0f);
    const char *op0 = "> TENTAR DE NOVO";
    while (*op0) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *op0); op0++; }

    // Opção 1
    if (opcaoDerrota == 1) { glColor3f(1.0f, 0.3f, 0.3f); } else { glColor3f(0.6f, 0.6f, 0.6f); }
    glRasterPos2f(-0.25f, -0.20f);
    const char *op1 = "  MENU PRINCIPAL";
    while (*op1) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *op1); op1++; }

    glColor3f(0.4f, 0.2f, 0.2f);
    glRasterPos2f(-0.40f, -0.80f);
    const char *h = "W/S = navegar   ENTER = confirmar";
    while (*h) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *h); h++; }

    glutSwapBuffers();
}

void handleDerrotaInput(unsigned char tecla) {
    switch (tecla) {
        case 'w': case 'W':
            opcaoDerrota = (opcaoDerrota - 1 + 2) % 2;
            break;
        case 's': case 'S':
            opcaoDerrota = (opcaoDerrota + 1) % 2;
            break;
        case '\r': case '\n':
            if (opcaoDerrota == 0) {
                // Tentar de novo — reinicia do zero
                initGame();
                gameState = 1;
            } else {
                // Menu principal
                gameState = 0;
                initGame();
            }
            opcaoDerrota = 0;
            break;
        case 27:
            gameState = 0; initGame(); opcaoDerrota = 0;
            break;
    }
}
void handlePauseInput(unsigned char tecla) {
    switch (tecla) {
        case 'w': case 'W':
            opcaoPause = (opcaoPause - 1 + 2) % 2;
            break;
        case 's': case 'S':
            opcaoPause = (opcaoPause + 1) % 2;
            break;
        case '\r': case '\n':
            if (opcaoPause == 0) {
                pausado = false;   // continuar
            } else {
                pausado = false;
                gameState = 0;
                initGame();
            }
            opcaoPause = 0;
            break;
        case 27:
            pausado = false; opcaoPause = 0;  // ESC continua
            break;
    }
}

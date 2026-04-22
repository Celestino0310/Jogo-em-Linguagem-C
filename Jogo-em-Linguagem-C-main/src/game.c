#define GLUT_DISABLE_ATEXIT_HACK
#include "../include/game.h"
#include "../include/mapa.h"
#include "../include/anim.h"
#include "../include/GL/glut.h"
#include <stdbool.h>
#include <stdlib.h>

extern int gameState;

// ============================================================
// JOGADOR
// ============================================================
static int   Health       = 3;
static float posX         = -0.715f;
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
    if(posX+PW>=s.x1-0.01f && posX-PW<s.x1 &&
       posY+PH>s.y1 && posY<s.y2){
        if(faseAtual<3){
            avancarFase();
            posX=-0.75f; posY=-0.70f; velX=0; velY=0; noChao=true;
        } else {
            gameState=0; initGame();
        }
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
    animInit(); // carrega a spritesheet da Althea
}

void renderGame(){
    renderMapa(); // desenha fundo + blocos

    // personagem — usa sprite se carregado, senão retângulo de fallback
    animDraw(posX, posY, (PW-0.025f)*3.0f, PH);

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
    resolveColisoes();
    checaSaida();
    updateMapa();

    if(!podeDash){
        tempoDash+=0.016f;
        if(tempoDash>=cooldownDash){ podeDash=true; tempoDash=0.0f; }
    }

    if(posY<-1.2f){
        Health--;
        if(Health<=0){ gameState=0; initGame(); return; }
        posX=-0.75f; posY=-0.70f; velX=0; velY=0; noChao=true;
    }

    // atualiza animação do personagem (sempre no final do update)
    animUpdate(0.016f, velX, velY, (int)noChao,
               (int)podeDash, tempoDash, direcao);
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
        case 27: gameState=0; initGame(); break;
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

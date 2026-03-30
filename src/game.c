#define GLUT_DISABLE_ATEXIT_HACK
#include "../include/game.h"
#include "../include/GL/glut.h"
#include <stdbool.h>
#include <stdlib.h>

extern int gameState;

// ============================================================
// JOGADOR
// ============================================================
static float posX     = -0.75f;
static float posY     = -0.65f;
static float velX     =  0.0f;
static float velY     =  0.0f;
static float gravidade = -0.002f;
static bool  noChao   = false;
static bool  podeDash  = true;
static int   direcao  =  1;

static bool kA = false, kD = false, kW = false, kSpace = false;

// ============================================================
// MAPA
// ============================================================
typedef struct { float x1, y1, x2, y2; } Bloco;

static Bloco blocos[] = {
    {-1.00f, -1.00f, -0.10f, -0.80f}, // 0 chao esq
    { 0.10f, -1.00f,  1.00f, -0.80f}, // 1 chao dir
    {-1.00f, -1.00f, -0.90f,  1.00f}, // 2 parede esq
    { 0.90f, -1.00f,  1.00f,  1.00f}, // 3 parede dir
    {-1.00f,  0.90f,  1.00f,  1.00f}, // 4 teto
    {-0.90f,  0.30f, -0.80f,  0.90f}, // 5 pilar esq caixa
    {-0.90f,  0.80f, -0.30f,  0.90f}, // 6 teto caixa
    {-0.40f,  0.30f, -0.30f,  0.80f}, // 7 pilar dir caixa
    {-0.90f,  0.20f, -0.30f,  0.30f}, // 8 chao caixa
    {-0.60f, -0.20f,  0.00f, -0.10f}, // 9 plat centro-esq
    {-0.10f,  0.10f,  0.30f,  0.20f}, // 10 plat centro
    { 0.20f, -0.50f,  0.90f, -0.40f}, // 11 chao caixa dir
    { 0.20f, -0.50f,  0.30f,  0.10f}, // 12 pilar esq dir
    { 0.80f, -0.50f,  0.90f,  0.10f}, // 13 pilar dir dir
    { 0.20f,  0.00f,  0.40f,  0.10f}, // 14 teto esq dir
    { 0.70f,  0.00f,  0.90f,  0.10f}, // 15 teto dir dir
    { 0.40f,  0.40f,  0.90f,  0.50f}, // 16 plat alta dir
    { 0.00f,  0.55f,  0.30f,  0.65f}, // 17 plat topo centro
};
static int numBlocos = sizeof(blocos) / sizeof(Bloco);

// indices de blocos que sao "pedra" (paredes/pilares)
static bool ehPedra(int i) {
    return (i==2||i==3||i==4||i==5||i==6||i==7||i==12||i==13||i==14||i==15);
}

// ============================================================
// DESENHO
// ============================================================
static void quad(float x1,float y1,float x2,float y2,float r,float g,float b){
    glColor3f(r,g,b);
    glBegin(GL_QUADS);
        glVertex2f(x1,y1);glVertex2f(x2,y1);
        glVertex2f(x2,y2);glVertex2f(x1,y2);
    glEnd();
}
static void borda(float x1,float y1,float x2,float y2,float r,float g,float b){
    glColor3f(r,g,b);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(x1,y1);glVertex2f(x2,y1);
        glVertex2f(x2,y2);glVertex2f(x1,y2);
    glEnd();
}
static void desenhaTerra(float x1,float y1,float x2,float y2){
    quad(x1,y1,x2,y2, 0.35f,0.22f,0.10f);
    borda(x1,y1,x2,y2, 0.18f,0.11f,0.05f);
    quad(x1,y2-0.025f,x2,y2, 0.20f,0.55f,0.20f); // grama
}
static void desenhaPedra(float x1,float y1,float x2,float y2){
    quad(x1,y1,x2,y2, 0.25f,0.28f,0.45f);
    borda(x1,y1,x2,y2, 0.12f,0.14f,0.22f);
}

// ============================================================
// COLISAO
// ============================================================
#define PW 0.055f
#define PH 0.09f

static bool colideCom(Bloco b){
    return posX+PW > b.x1 && posX-PW < b.x2 &&
           posY+PH > b.y1 && posY    < b.y2;
}

static void resolveColisoes(){
    int i;
    noChao = false;
    for(i=0;i<numBlocos;i++){
        if(!colideCom(blocos[i])) continue;
        Bloco b = blocos[i];
        float oB = (posY+PH) - b.y1; // overlap baixo
        float oC = b.y2 - posY;      // overlap cima
        float oD = (posX+PW) - b.x1; // overlap dir
        float oE = b.x2 - (posX-PW); // overlap esq
        float mH = oB<oC ? oB : oC;
        float mV = oD<oE ? oD : oE;
        if(mH < mV){
            if(oB < oC){ posY -= oB; if(velY>0) velY=0; noChao=true; podeDash=true; }
            else        { posY += oC; if(velY<0) velY=0; }
        } else {
            if(oD < oE) posX -= oD;
            else        posX += oE;
            velX = 0;
        }
    }
}

// ============================================================
// PUBLICAS
// ============================================================
void initGame(){
    posX=-0.75f; posY=-0.65f;
    velX=0; velY=0; noChao=false; podeDash=true;
    kA=kD=kW=kSpace=false;
}

void renderGame(){
    glClearColor(0.05f,0.05f,0.15f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Mapa
    int i;
    for(i=0;i<numBlocos;i++){
        Bloco b=blocos[i];
        if(ehPedra(i)) desenhaPedra(b.x1,b.y1,b.x2,b.y2);
        else           desenhaTerra(b.x1,b.y1,b.x2,b.y2);
    }

    //Jogador
    float largura = PW - 0.025f;
	
	quad(posX-largura, posY, posX+largura, posY+PH, 0.6f, 0.2f, 0.8f);
	borda(posX-largura, posY, posX+largura, posY+PH, 0.0f,0.0f,0.0f);
	  
    
	// CABEÇA
	quad(posX-0.03f, posY+0.09f, posX+0.03f, posY+0.14f, 1.0f,0.8f,0.6f);
	
	// olho
    float ox = (direcao>0) ? posX+0.017f : posX-0.017f;
    quad(ox-0.009f, posY+0.119f, ox+0.009f, posY+0.125f, 0.0f,0.0f,0.0f);
	//braço
	float oax = (direcao>0) ? posX+0.015f : posX-0.015f;
	quad(oax-0.009f, posY+0.018f, oax+0.009f, posY+0.068f, 0.3f, 0.0f, 0.5f);
	
    // HUD
    glColor3f(0.4f,0.4f,0.4f);
    glRasterPos2f(-0.98f, 0.92f);
    const char *hud="WASD/SPACE=mover/pular  Q=dash  ESC=menu";
    while(*hud){ glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,*hud); hud++; }

    glutSwapBuffers();
}

void updateGame(){
    // Movimento horizontal suavizado
    float velAlvo = 0.0f;
    if(kA){ velAlvo=-0.025f; direcao=-1; }
    if(kD){ velAlvo= 0.025f; direcao= 1; }
    velX += (velAlvo-velX)*0.3f;

    // Pulo
    if((kW||kSpace)&&noChao){
        velY=0.048f;
        noChao=false;
        kW=false; kSpace=false; // consome o input
    }

    // Gravidade
    velY += gravidade;
    if(velY<-0.06f) velY=-0.06f;

    posX+=velX;
    posY+=velY;

    resolveColisoes();

    // Caiu no buraco = respawn
    if(posY < -1.2f){ initGame(); }
}

void handleGameInput(unsigned char tecla){
    switch(tecla){
        case 'a': case 'A': kA=true;    break;
        case 'd': case 'D': kD=true;    break;
        case 'w': case 'W': kW=true;    break;
        case ' ':           kSpace=true; break;
        case 'q': case 'Q': // DASH
            if(podeDash){
                velX=0.13f*direcao;
                velY=0.01f;
                podeDash=false;
            }
            break;
        case 27: gameState=0; initGame(); break;
    }
}

// Chamado pelo keyboardUp do main
void handleGameInputUp(unsigned char tecla){
    switch(tecla){
        case 'a': case 'A': kA=false;    break;
        case 'd': case 'D': kD=false;    break;
        case 'w': case 'W': kW=false;    break;
        case ' ':           kSpace=false; break;
    }
}

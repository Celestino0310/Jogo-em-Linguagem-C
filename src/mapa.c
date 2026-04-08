#define GLUT_DISABLE_ATEXIT_HACK
#include "../include/GL/glut.h"
#include "../include/mapa.h"
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

// ============================================================
// ESTADO PUBLICO
// ============================================================
Bloco *blocos    = NULL;
int    numBlocos = 0;
int    faseAtual = 0;

// ============================================================
// ESTRELAS
// ============================================================
#define MAX_ESTRELAS 100
typedef struct { float x,y,brilho; } Estrela;
static Estrela estrelas[MAX_ESTRELAS];
static float   tempoEstrela = 0;

static void initEstrelas(){
    int i;
    for(i=0;i<MAX_ESTRELAS;i++){
        estrelas[i].x      = ((float)rand()/RAND_MAX)*2.0f - 1.0f;
        estrelas[i].y      = ((float)rand()/RAND_MAX)*2.0f - 1.0f;
        estrelas[i].brilho = 0.3f + ((float)rand()/RAND_MAX)*0.7f;
    }
}
static void desenhaEstrelas(){
    tempoEstrela += 0.016f;
    int i;
    glPointSize(2.5f);//1.5f
    glBegin(GL_POINTS);
    for(i=0;i<MAX_ESTRELAS;i++){
        float b = estrelas[i].brilho*(0.6f+0.4f*sinf(tempoEstrela*1.5f+i));
        glColor3f(b,b,b*1.1f);
        glVertex2f(estrelas[i].x, estrelas[i].y);
    }
    glEnd();
}

// ============================================================
// NEVE CAINDO
// ============================================================
#define MAX_NEVE 80
typedef struct { float x,y,vel,tam; } Floco;
static Floco neve[MAX_NEVE];
static bool  neveIniciada = false;

static void initNeve(){
    int i;
    for(i=0;i<MAX_NEVE;i++){
        neve[i].x   = ((float)rand()/RAND_MAX)*2.2f - 1.1f;
        neve[i].y   = ((float)rand()/RAND_MAX)*2.0f - 1.0f;
        neve[i].vel = 0.002f + ((float)rand()/RAND_MAX)*0.004f;
        neve[i].tam = 1.0f   + ((float)rand()/RAND_MAX)*1.5f;
    }
    neveIniciada = true;
}
static void atualizaNeve(){
    int i;
    for(i=0;i<MAX_NEVE;i++){
        neve[i].y -= neve[i].vel;
        neve[i].x -= neve[i].vel * 0.4f;
        if(neve[i].y < -1.0f){
            neve[i].y = 1.05f;
            neve[i].x = ((float)rand()/RAND_MAX)*2.2f - 1.1f;
        }
    }
}
static void desenhaNeveCaindo(){
    int i;
    for(i=0;i<MAX_NEVE;i++){
        glColor3f(0.85f,0.90f,1.0f);
        glPointSize(neve[i].tam);
        glBegin(GL_POINTS); glVertex2f(neve[i].x,neve[i].y); glEnd();
    }
}

// ============================================================
// DADOS DOS MAPAS
// ============================================================

// --- MAPA 1: FLORESTA NEVADA ---
static Bloco blocosFase0[] = {
    {-1.00f,-1.00f,-0.95f, 1.00f}, // 0  parede esq
    { 0.975f,-1.00f, 1.00f, 1.00f}, // 1  parede dir
    {-1.00f, 0.95f, 0.55f, 1.00f}, //2  teto
    //em ordem da esquerda pra direita
    {-0.95f,-1.00f,-0.67f,-0.82f}, // 3  chao esq(onde o player nasce)
    {-0.67f,-1.00f,-0.30f,-0.98f}, // 4 plat espinho 1
    {-0.30f,-1.00f, -0.10f,-0.60f}, // 5  plat safe 1
    { -0.10f,-1.00f, 0.30f,-0.82f}, // 6  plat espinho 2  
	{ 0.30f,-0.50f, 0.35f,-0.15f}, // 7  ferro esquerdo
	{ 0.45f,-0.50f, 0.50f,-0.15f}, // 8  ferro direito 
	{ 0.30f,-0.10f, 0.50f,-0.15f}, // 9 plataforma de ferro 
    {0.843f, -0.30f, 0.975f,  0.25f},// 20 parede de terra DIREITA marrom
	{ 0.30f,-1.00f, 0.50f,-0.50f}, // 11  sustenta ferro marrom
	{0.860f, -0.60f, 0.975f, -0.30f},// 19 parede de terra DIREITA BAIXO marrom
	{0.55f, 0.95f, 0.80f, 1.00f}, //13  teto de cimento
	
    //decoração esquerda  
   
    {-0.875f, 0.95f,-0.85f, 0.60f}, // 14  torre esq topo
    {-0.675f, 0.95f,-0.65f, 0.60f}, // 15  torre esq topo
    { -0.875f, 0.60f,-0.65f, 0.55f}, // 16 base da decoração de ferro esqurda
    
    
    { -0.95, 0.95f,-0.875f, 0.30f}, // 17 parede de terra esq
  
    {0.885f, -1.00f, 0.975f, -0.60f},// 18 parede de terra DIREITA BAIXISSIMO

  
    { 0.45f, 0.95f, 0.50f, 0.55f}, // 12  ferro direito do teto tambem ta aqui pra fica cinza
    { 0.35f,-0.35f, 0.45f,-0.30f}, // 10 ferro central ta aqui pra fica cinza
    {0.87f,  0.25f, 0.97f,  0.70f},// 21 parede de terra DIREITA ALTO

    {0.15f, 0.95f, 0.45f, 0.88f}, // 22 teto menorzinho marrom central
    {-0.05f, 0.95f, 0.15f, 0.80f}, // 23 teto menor marrom central
    {-0.25f, 0.95f, -0.05f, 0.60f}, //24 teto central marrom	
    
    { 0.87f, 0.70f, 0.97f, 0.75f}, // 25 SAIDA
};

// --- MAPA 2: CIDADE NEVADA ---
static Bloco blocosFase1[] = {
    {-1.00f,-1.00f,-0.92f, 1.00f}, // 0
    { 0.92f,-1.00f, 1.00f, 1.00f}, // 1
    {-1.00f, 0.92f, 1.00f, 1.00f}, // 2
    {-0.92f,-1.00f,-0.55f,-0.82f}, // 3 chao esq
    {-0.20f,-1.00f, 0.15f,-0.82f}, // 4 chao centro
    { 0.50f,-1.00f, 0.92f,-0.82f}, // 5 chao dir
    {-0.90f,-0.82f,-0.72f, 0.50f}, // 6 predio corpo
    {-0.93f, 0.48f,-0.69f, 0.60f}, // 7 predio topo
    {-0.87f, 0.10f,-0.75f, 0.20f}, // 8 janela baixa
    {-0.87f, 0.30f,-0.75f, 0.40f}, // 9 janela alta
    {-0.68f,-0.48f,-0.38f,-0.38f}, // 10
    {-0.55f,-0.08f,-0.25f, 0.02f}, // 11
    {-0.65f, 0.28f,-0.35f, 0.38f}, // 12
    {-0.50f, 0.60f,-0.20f, 0.70f}, // 13
    {-0.18f,-0.82f,-0.08f, 0.30f}, // 14 pilar esq
    { 0.08f, 0.00f, 0.18f, 0.30f}, // 15 pilar dir
    {-0.18f, 0.20f, 0.18f, 0.30f}, // 16 teto
    {-0.14f,-0.40f, 0.14f,-0.30f}, // 17 plat interna
    { 0.22f,-0.82f, 0.32f, 0.40f}, // 18 pilar fino
    { 0.19f, 0.38f, 0.35f, 0.48f}, // 19 topo pilar
    { 0.40f,-0.82f, 0.92f,-0.72f}, // 20 base larga dir
    { 0.55f,-0.72f, 0.92f,-0.62f}, // 21 degrau 1
    { 0.65f,-0.42f, 0.92f,-0.32f}, // 22 degrau 2
    { 0.72f,-0.12f, 0.92f,-0.02f}, // 23 degrau 3
    { 0.78f, 0.20f, 0.92f, 0.30f}, // 24 degrau 4
    { 0.70f, 0.78f, 0.92f, 0.80f}, // 25 SAIDA
};

// --- MAPA 3: GELEIRAS ---
static Bloco blocosFase2[] = {
    {-1.00f,-1.00f,-0.92f, 1.00f}, // 0
    { 0.92f,-1.00f, 1.00f, 1.00f}, // 1
    {-1.00f, 0.92f, 1.00f, 1.00f}, // 2
    {-0.92f,-1.00f,-0.65f,-0.80f}, // 3 ilha esq
    {-0.10f,-1.00f, 0.25f,-0.80f}, // 4 ilha centro
    { 0.60f,-1.00f, 0.92f,-0.80f}, // 5 ilha dir
    {-0.88f,-0.45f,-0.60f,-0.35f}, // 6
    {-0.50f,-0.15f,-0.22f,-0.05f}, // 7
    { 0.00f,-0.40f, 0.28f,-0.30f}, // 8
    { 0.35f,-0.10f, 0.62f, 0.00f}, // 9
    {-0.75f, 0.15f,-0.48f, 0.25f}, // 10
    {-0.30f, 0.10f, 0.00f, 0.20f}, // 11
    { 0.10f, 0.15f, 0.38f, 0.25f}, // 12
    { 0.50f, 0.20f, 0.78f, 0.30f}, // 13
    {-0.60f, 0.45f,-0.32f, 0.55f}, // 14
    {-0.15f, 0.42f, 0.15f, 0.52f}, // 15
    { 0.28f, 0.48f, 0.60f, 0.58f}, // 16
    {-0.45f, 0.72f,-0.15f, 0.82f}, // 17
    { 0.10f, 0.70f, 0.45f, 0.80f}, // 18
    {-0.08f,-0.80f, 0.08f,-0.20f}, // 19 pilar gelo
    { 0.60f, 0.78f, 0.92f, 0.86f}, // 20 SAIDA
};

// --- MAPA FINAL: PICO ---
static Bloco blocosFase3[] = {
    {-1.00f,-1.00f,-0.92f, 1.00f}, // 0
    { 0.92f,-1.00f, 1.00f, 1.00f}, // 1
    {-1.00f, 0.92f, 1.00f, 1.00f}, // 2
    {-0.92f,-1.00f,-0.70f,-0.78f}, // 3
    {-0.55f,-1.00f,-0.30f,-0.82f}, // 4
    {-0.15f,-1.00f, 0.10f,-0.75f}, // 5
    { 0.22f,-1.00f, 0.45f,-0.80f}, // 6
    { 0.58f,-1.00f, 0.92f,-0.76f}, // 7
    {-0.88f,-0.42f,-0.58f,-0.32f}, // 8
    {-0.70f,-0.08f,-0.42f, 0.02f}, // 9
    {-0.38f,-0.35f,-0.10f,-0.25f}, // 10
    {-0.20f,-0.02f, 0.10f, 0.08f}, // 11
    { 0.05f,-0.38f, 0.32f,-0.28f}, // 12
    { 0.28f,-0.05f, 0.55f, 0.05f}, // 13
    { 0.50f,-0.42f, 0.78f,-0.32f}, // 14
    { 0.62f,-0.08f, 0.90f, 0.02f}, // 15
    {-0.82f, 0.22f,-0.55f, 0.32f}, // 16
    {-0.50f, 0.18f,-0.22f, 0.28f}, // 17
    {-0.12f, 0.22f, 0.18f, 0.32f}, // 18
    { 0.22f, 0.18f, 0.52f, 0.28f}, // 19
    { 0.58f, 0.25f, 0.88f, 0.35f}, // 20
    {-0.72f, 0.50f,-0.45f, 0.60f}, // 21
    {-0.35f, 0.48f,-0.05f, 0.58f}, // 22
    { 0.08f, 0.52f, 0.38f, 0.62f}, // 23
    { 0.45f, 0.55f, 0.75f, 0.65f}, // 24
    {-0.20f, 0.78f, 0.20f, 0.86f}, // 25 TOPO FINAL
};

// ============================================================
// CORES POR FASE
// ============================================================


static Cor corFundo[]  = {
    {0.02f,0.04f,0.08f},//fase 1
    {0.03f,0.03f,0.10f},
    {0.03f,0.06f,0.12f},
    {0.05f,0.04f,0.08f},
};
static Cor corBloco[]  = {
    {0.38f,0.24f,0.12f},//fase 1
    {0.25f,0.28f,0.42f},
    {0.50f,0.68f,0.88f},
    {0.35f,0.32f,0.38f},
};
static Cor corBorda[]  = {
    {0.20f,0.12f,0.06f},//fase 1
    {0.35f,0.40f,0.60f},
    {0.28f,0.48f,0.72f},
    {0.20f,0.18f,0.22f},
};
static Cor corPedra[]  = {
    {0.20f,0.22f,0.38f},//fase 1
    {0.18f,0.20f,0.35f},//fase 2
    {0.22f,0.35f,0.55f},
    {0.22f,0.20f,0.28f},
};
static Cor corBordaP[] = {
    {0.30f,0.35f,0.55f},
    {0.28f,0.32f,0.52f},
    {0.35f,0.55f,0.80f},
    {0.32f,0.28f,0.40f},
};

// ============================================================
// HELPERS DE DESENHO
// ============================================================
static void quad(float x1,float y1,float x2,float y2,float r,float g,float b){
    glColor3f(r,g,b);
    glBegin(GL_QUADS);
        glVertex2f(x1,y1);glVertex2f(x2,y1);
        glVertex2f(x2,y2);glVertex2f(x1,y2);
    glEnd();
}
static void bordaLinha(float x1,float y1,float x2,float y2,float r,float g,float b){
    glColor3f(r,g,b);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(x1,y1);glVertex2f(x2,y1);
        glVertex2f(x2,y2);glVertex2f(x1,y2);
    glEnd();
}

//é um facilitador pra deixar aquela camada branca

static void neveTopo(float x1,float y2,float x2){
    float esp=(faseAtual>=2)?0.045f:0.030f;
    glColor3f(0.88f,0.93f,1.00f);
    glBegin(GL_QUADS);
        glVertex2f(x1,y2); glVertex2f(x2,y2);
        glVertex2f(x2,y2+esp); glVertex2f(x1,y2+esp);
    glEnd();
    glColor3f(0.65f,0.75f,0.95f);
    glLineWidth(1.0f);
    glBegin(GL_LINE_STRIP);
        glVertex2f(x1,y2+esp); glVertex2f(x2,y2+esp);
    glEnd();
}



static void desenhaBloco(float x1,float y1,float x2,float y2){
    Cor c=corBloco[faseAtual], b=corBorda[faseAtual];
    quad(x1,y1,x2,y2, c.r,c.g,c.b);
    bordaLinha(x1,y1,x2,y2, b.r,b.g,b.b);
    neveTopo(x1,y2,x2);
}


static void desenhaBlocoPedra(float x1,float y1,float x2,float y2){
    Cor c=corPedra[faseAtual], b=corBordaP[faseAtual];
    quad(x1,y1,x2,y2, c.r,c.g,c.b);
    bordaLinha(x1,y1,x2,y2, b.r,b.g,b.b);
    glColor3f(b.r*0.8f,b.g*0.8f,b.b*0.8f);
    glLineWidth(0.8f);
    float my=(y1+y2)*0.5f, mx=(x1+x2)*0.5f;
    glBegin(GL_LINES);
        glVertex2f(x1,my);glVertex2f(x2,my);
        glVertex2f(mx,y1);glVertex2f(mx,y2);
    glEnd();
    neveTopo(x1,y2,x2);
}
static void desenhaSaida(float x1,float y1,float x2,float y2){
    float t=sinf(tempoEstrela*3.0f)*0.1f;
    if(faseAtual==3){
        quad(x1,y1,x2,y2, 0.7f+t,0.6f+t,0.1f);
        bordaLinha(x1,y1,x2,y2, 1.0f,0.9f,0.2f);
    } else {
        quad(x1,y1,x2,y2, 0.04f,0.28f+t,0.12f);
        bordaLinha(x1,y1,x2,y2, 0.08f,0.75f,0.30f);
    }
    neveTopo(x1,y2,x2);
    float cx=(x1+x2)*0.5f, cy=(y1+y2)*0.5f;
    glColor3f(0.3f,1.0f,0.5f);
    glPointSize(7.0f);
    glBegin(GL_POINTS); glVertex2f(cx,cy); glEnd();
    glColor3f(1.0f,1.0f,1.0f);
    glRasterPos2f(cx-0.06f, y2+0.02f);
    const char *txt=(faseAtual==3)?"TOPO!":"NEXT >";
    while(*txt){glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,*txt);txt++;}
}

// ============================================================
// FUNCOES PUBLICAS
// ============================================================
bool ehEstrutura(int i){
    if(faseAtual==0) return (i==0||i==1||i==2||i==7||i==8||i==9||i==14||i==15||i==16||i==19||i==20);
    if(faseAtual==1) return (i==0||i==1||i==2||i==6||i==7||i==8||i==9||i==14||i==15||i==16||i==18||i==19);
    if(faseAtual==2) return (i==0||i==1||i==2||i==19);
    if(faseAtual==3) return (i==0||i==1||i==2);
    return false;
}

int indiceSaida(){
    if(faseAtual==0) return 25;
    if(faseAtual==1) return 25;
    if(faseAtual==2) return 20;
    if(faseAtual==3) return 25;
    return -1;
}

void initMapa(){
    faseAtual    = 0;
    blocos       = blocosFase0;
    numBlocos    = sizeof(blocosFase0)/sizeof(Bloco);
    neveIniciada = false;
    initEstrelas();
}

void renderMapa(){
    // fundo colorido por fase
   
    Cor bg = corFundo[faseAtual];
    glClearColor(bg.r,bg.g,bg.b,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    desenhaEstrelas();
    if((faseAtual==2||faseAtual==3) && neveIniciada) desenhaNeveCaindo();

    // desenha todos os blocos
    int i, idxS=indiceSaida();
    for(i=0;i<numBlocos;i++){
        Bloco b=blocos[i];
        if(i==idxS)             desenhaSaida(b.x1,b.y1,b.x2,b.y2);
        else if(ehEstrutura(i)) desenhaBlocoPedra(b.x1,b.y1,b.x2,b.y2);
        else                    desenhaBloco(b.x1,b.y1,b.x2,b.y2);
    }
}

void updateMapa(){
    if((faseAtual==2||faseAtual==3) && neveIniciada) atualizaNeve();
}

// chamado pelo game.c quando o jogador pisa na saida
void avancarFase(){
    faseAtual++;
    if(faseAtual>3) return;
    switch(faseAtual){
        case 1: blocos=blocosFase1; numBlocos=sizeof(blocosFase1)/sizeof(Bloco); break;
        case 2: blocos=blocosFase2; numBlocos=sizeof(blocosFase2)/sizeof(Bloco); break;
        case 3: blocos=blocosFase3; numBlocos=sizeof(blocosFase3)/sizeof(Bloco); initNeve(); break;
    }
}

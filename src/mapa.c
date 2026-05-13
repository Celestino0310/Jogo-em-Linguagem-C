#define GLUT_DISABLE_ATEXIT_HACK
#define STB_IMAGE_IMPLEMENTATION
#include "../include/audio.h"
#include "../include/espinho.h"
#include "../include/stb_image.h"
#include "../include/GL/glut.h"
#include "../include/mapa.h"
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>




Bloco *blocos    = NULL;
int    numBlocos = 0;
int    faseAtual = 0;











#define NUM_TEX 6
typedef enum {
    TEX_TERRA  = 0,
    TEX_METAL  = 1,
    TEX_PEDRA  = 2,
    TEX_GELO   = 3,
    TEX_ROCHA  = 4,
    TEX_GRAMA  = 5
} TexID;

static unsigned int texIDs[NUM_TEX];
static bool texCarregada[NUM_TEX];

static const char *texCaminhos[NUM_TEX] = {
    "../assets/textures/terra.png",
    "../assets/textures/metal.jpg",
    "../assets/textures/pedra.png",
    "../assets/textures/gelo.png",
    "../assets/textures/rocha.png",
    "../assets/textures/grama.png",
};

static void carregaTextura(int idx) {
    int w, h, ch;
    stbi_set_flip_vertically_on_load(1);
    unsigned char *dados = stbi_load(texCaminhos[idx], &w, &h, &ch, 4);
    if (!dados) {
        printf("[TEX] Nao encontrou: %s (usando cor solida)\n", texCaminhos[idx]);
        texCarregada[idx] = false;
        return;
    }
    glGenTextures(1, &texIDs[idx]);
    glBindTexture(GL_TEXTURE_2D, texIDs[idx]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, dados);
    stbi_image_free(dados);
    texCarregada[idx] = true;
    printf("[TEX] OK: %s\n", texCaminhos[idx]);
}

static void initTexturas() {
    int i;
    for (i = 0; i < NUM_TEX; i++) carregaTextura(i);
}



static void quadTex(float x1,float y1,float x2,float y2,
     int texIdx, float tileX, float tileY,
    float r,float g,float b) {
    if (texCarregada[texIdx]) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texIDs[texIdx]);
        glColor3f(r, g, b); 
        glBegin(GL_QUADS);
            glTexCoord2f(0,      0);      glVertex2f(x1, y1);
            glTexCoord2f(tileX,  0);      glVertex2f(x2, y1);
            glTexCoord2f(tileX,  tileY);  glVertex2f(x2, y2);
            glTexCoord2f(0,      tileY);  glVertex2f(x1, y2);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    } else {
        
        glColor3f(r, g, b);
        glBegin(GL_QUADS);
            glVertex2f(x1,y1); glVertex2f(x2,y1);
            glVertex2f(x2,y2); glVertex2f(x1,y2);
        glEnd();
    }
}


static void bordaLinha(float x1,float y1,float x2,float y2,
                       float r,float g,float b, float lw) {
    glColor3f(r,g,b);
    glLineWidth(lw);
    glBegin(GL_LINE_LOOP);
        glVertex2f(x1,y1); glVertex2f(x2,y1);
        glVertex2f(x2,y2); glVertex2f(x1,y2);
    glEnd();
}



static float tileCount(float a, float b, float baseSize) {
    float sz = (b - a);
    if (sz < 0) sz = -sz;
    float t = sz / baseSize;
    if (t < 0.2f) t = 0.2f;
    return t;
}




static void neveTopo(float x1,float y2,float x2, bool isGelo){
    float esp = isGelo ? 0.045f : 0.028f;
    
    glColor3f(isGelo ? 0.80f : 0.88f,
              isGelo ? 0.90f : 0.93f,
              1.00f);
    glBegin(GL_QUADS);
        glVertex2f(x1,y2);       glVertex2f(x2,y2);
        glVertex2f(x2,y2+esp);   glVertex2f(x1,y2+esp);
    glEnd();
    
    glColor3f(0.95f, 0.97f, 1.0f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_STRIP);
        glVertex2f(x1, y2+esp); glVertex2f(x2, y2+esp);
    glEnd();
}

static void gramaTopo(float x1, float y2, float x2) {
    
    float esp = 0.025f;
    
    glColor3f(0.10f, 0.38f, 0.08f);
    glBegin(GL_QUADS);
        glVertex2f(x1, y2);     glVertex2f(x2, y2);
        glVertex2f(x2,y2+esp);  glVertex2f(x1,y2+esp);
    glEnd();
    
    glColor3f(0.18f, 0.55f, 0.12f);
    glLineWidth(1.5f);
    float step = 0.035f;
    float xx;
    for (xx = x1+0.01f; xx < x2-0.005f; xx += step) {
        glBegin(GL_LINES);
            glVertex2f(xx, y2);
            glVertex2f(xx + 0.005f, y2 + esp + 0.015f);
        glEnd();
    }
}




#define MAX_ESTRELAS 120
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
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for(i=0;i<MAX_ESTRELAS;i++){
        float b = estrelas[i].brilho*(0.6f+0.4f*sinf(tempoEstrela*1.5f+i));
        glColor3f(b, b, b*1.15f);
        glVertex2f(estrelas[i].x, estrelas[i].y);
    }
    glEnd();
}




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
        neve[i].x -= neve[i].vel*0.4f;
        if(neve[i].y < -1.0f){
            neve[i].y = 1.05f;
            neve[i].x = ((float)rand()/RAND_MAX)*2.2f-1.1f;
        }
    }
}
static void desenhaNeveCaindo(){
    int i;
    for(i=0;i<MAX_NEVE;i++){
        glColor3f(0.88f,0.93f,1.0f);
        glPointSize(neve[i].tam);
        glBegin(GL_POINTS); glVertex2f(neve[i].x,neve[i].y); glEnd();
    }
}




static Bloco blocosFase0[] = {
    {-1.00f,-1.00f,-0.95f, 1.00f},
    { 0.975f,-1.00f, 1.00f, 1.00f},
    {-1.00f, 0.95f, 0.55f, 1.00f},
    {-0.95f,-1.00f,-0.67f,-0.82f},
    {-0.67f,-1.00f,-0.30f,-0.98f},
    {-0.30f,-1.00f,-0.10f,-0.60f},
    {-0.10f,-1.00f, 0.30f,-0.82f},
    { 0.30f,-0.50f, 0.35f,-0.15f},
    { 0.45f,-0.50f, 0.50f,-0.15f},
    { 0.30f,-0.10f, 0.50f,-0.15f},
    {0.843f,-0.30f, 0.975f, 0.25f},
    { 0.30f,-1.00f, 0.50f,-0.50f},
    {0.860f,-0.60f, 0.975f,-0.30f},
    {0.55f,  0.95f, 0.80f,  1.00f},
    {-0.875f,0.95f,-0.85f,  0.60f},
    {-0.675f,0.95f,-0.65f,  0.60f},
    {-0.875f,0.60f,-0.65f,  0.55f},
    {-0.95f, 0.95f,-0.875f, 0.30f},
    {0.885f,-1.00f, 0.975f,-0.60f},
    { 0.45f, 0.55f, 0.50f,  0.95f},
    { 0.35f,-0.35f, 0.45f, -0.30f},
    {0.87f,  0.25f, 0.97f,  0.70f},
    {0.15f,  0.95f, 0.45f,  0.88f},
    {-0.05f, 0.95f, 0.15f,  0.80f},
    {-0.25f, 0.95f,-0.05f,  0.60f},
    { 0.87f, 0.70f, 0.97f,  0.75f},
};

static Bloco blocosFase1[] = {
    {-1.00f,-1.0f,-0.92f, 1.00f}, 
    { 0.95f,-1.00f, 1.00f, 0.75f}, 
    {-1.00f, 0.92f, 0.65f, 1.00f}, 
    
    {-0.92f,-1.00f,-0.55f,-0.82f}, 
  
    
    {-0.25f,-0.50f,-0.10f,-0.05f}, 
	{-0.10f,-0.20f, 0.00f,-0.05f}, 
	{ 0.00f,-0.30f, 0.05f,-0.05f}, 
    
    {-0.75f, 0.10f,-0.50f, 0.30f}, 
    {-0.60f,-0.20f,-0.50f, 0.10f}, 
    
    {-0.70f, 0.85f,-0.50f, 1.00f}, 
    {-0.20f, 0.80f, 0.00f, 1.05f}, 
    
    {-0.93f, 0.8f,-0.70f, 1.0f}, 
    
    { 0.10f,-1.00f, 0.25f,-0.90f}, 
    { 0.25f,-1.00f, 0.40f,-0.650f}, 
    
    { 0.65f, 0.55f, 0.70f, 1.00f}, 
    { 0.35f, 0.75f, 0.40f, 1.00f}, 
    
    { 0.40f,-1.00f, 0.70f,-0.80f}, 
    { 0.70f,-1.00f, 0.90f,-0.90f}, 
    { 0.90f,-1.00f, 0.95f,-0.85f}, 
    
    { 0.85f,-0.20f, 0.90f,0.05f}, 
    { 0.90f, -0.40f, 0.95f, 0.25f}, 
    
    { 0.45f, -0.25f, 0.50f, 0.20f}, 
    { 0.50f, -0.075f, 0.55f, 0.22f}, 
    { 0.40f,-0.15f, 0.45f,0.20f}, 
    
    {-0.55f,-1.00f,-0.50f,-0.87f}, 
    
    { 0.95f, 0.75f, 1.00f, 0.80f}, 
};
static Bloco blocosFase2[] = {
    {-1.00f, 0.10f,-0.90f, 1.00f}, 
    {-1.00f, 0.05f,-0.80f, 0.10f}, 
    {-1.00f,-1.00f,-0.92f,-0.35f}, 
    {-0.92f,-0.50f,-0.80f,-0.35f}, 

    {-0.75f,-1.00f,-0.50f,-0.92f}, 
    {-0.55f,-1.00f,-0.45f,-0.75f}, 

    {-0.45f,-1.00f,-0.25f, 0.20f}, 
    {-0.25f,-0.50f, -0.20f, 0.20f}, 
    { -0.20f,-0.30f,-0.15f, 0.20f}, 

    {-0.250f,-1.00f, 0.10f,-0.90f}, 
    { 0.10f,-1.00f, 0.20f,-0.80f}, 
    { 0.20f,-1.00f, 0.50f,-0.75f}, 

    { 0.50f,-1.00f, 1.00f, 0.00f}, 

    { 0.12f, 0.30f, 0.37f, 0.38f}, 
    { 0.10f, 0.10f, 0.20f, 0.30f}, 

    { 0.35f,-0.20f, 0.50f,-0.15f}, 
    { 0.40f,-0.30f, 0.50f,-0.15f}, 
    
    { 0.35f,-0.65f, 0.50f,-0.60f}, 
  

    { 0.950f,-1.00f, 1.0f, 1.0f}, 
    
    {-0.92f,-1.00f,-0.75f,-0.92f}, 

    { 0.85f, 0.00f, 0.950f, 0.10f}, 
};
static Bloco blocosFase3[] = {
    {-1.00f,-1.00f,-0.92f, 1.00f},
    { 0.92f,-1.00f, 1.00f, 1.00f},
    {-1.00f, 0.92f, 1.00f, 1.00f},
    {-0.92f,-1.00f,-0.70f,-0.78f},
    {-1.00f, 0.10f,-0.90f, 1.00f},
    
    {-0.92f,-0.20f,-0.80f,-0.10f},
  
    {-0.92f,-0.20f,-0.80f,-0.10f},
    
    {-0.40f,-1.00f,-0.30,-0.78f},
    {0.10f,-1.00f, 0.20f,-0.78f},
	{0.65f,-1.00f, 0.92f,-0.78f},
	{-0.92f,-1.00f,-0.70f,-0.78f},
	 
	{-0.50f, -0.10f,-0.10f,0.10f},
	{0.60f,-0.30f, 0.650f, 0.30f},
	
    {-0.60f, -0.10f,-0.50f, 0.30f},
    {-0.10f, -0.10f,0.00f, 0.30f},
	{ 0.40f, -0.10f,0.50f, 0.30f},
	
	{-0.40f,0.70f,-0.45f,0.92f},
	{-0.45f,0.80f,-0.50f,0.92f},
	
	{ 0.85f,0.80f,0.92f,0.92f},

	
    {0.50f,-0.10f,0.60f,0.10f},
    
    
    {-0.10f, 0.30f,-0.05f, 0.45f},
    {-0.05f, 0.30f, 0.00f, 0.45f},

    
    { 0.00f, -0.10f, 0.40f, 0.10f},
    

    
    {-0.60f, 0.30f,-0.55f, 0.45f},
	
	
	
	{-0.92f, -0.10f,-0.60f,0.4f},
	{-0.92f, 0.40f,-0.60f,0.45f},
	

    
};




typedef enum {
    TIPO_TERRA,   
    TIPO_METAL,   
    TIPO_PEDRA,   
    TIPO_GELO,    
    TIPO_ROCHA,   
    TIPO_SAIDA,   
} TipoBloco;

static TipoBloco tipoFase0[26] = {
    TIPO_PEDRA,  
    TIPO_PEDRA,  
    TIPO_METAL,  
    TIPO_TERRA,  
    TIPO_TERRA,  
    TIPO_TERRA,  
    TIPO_TERRA,  
    TIPO_METAL,  
    TIPO_METAL,  
    TIPO_METAL,  
    TIPO_TERRA,  
    TIPO_TERRA,  
    TIPO_TERRA,  
    TIPO_METAL,  
    TIPO_METAL,  
    TIPO_METAL,  
    TIPO_METAL,  
    TIPO_TERRA,  
    TIPO_TERRA,  
    TIPO_METAL,  
    TIPO_METAL,  
    TIPO_TERRA,  
    TIPO_TERRA,  
    TIPO_TERRA,  
    TIPO_TERRA,  
    TIPO_SAIDA,  
};

static TipoBloco tipoFase1[26] = {
    TIPO_PEDRA, TIPO_PEDRA, TIPO_PEDRA, 
    TIPO_PEDRA, TIPO_PEDRA, TIPO_PEDRA, 
    TIPO_PEDRA, TIPO_PEDRA,             
    TIPO_METAL, TIPO_METAL,             
    TIPO_PEDRA, TIPO_PEDRA, TIPO_PEDRA, TIPO_PEDRA, 
    TIPO_PEDRA, TIPO_PEDRA, TIPO_PEDRA, 
    TIPO_PEDRA,                         
    TIPO_PEDRA, TIPO_PEDRA,             
    TIPO_PEDRA, TIPO_PEDRA, TIPO_PEDRA, TIPO_PEDRA, TIPO_PEDRA, 
    TIPO_SAIDA, 
};

static TipoBloco tipoFase2[21] = {
    TIPO_GELO, TIPO_GELO, TIPO_GELO,  
    TIPO_GELO, TIPO_GELO, TIPO_GELO,  
    TIPO_GELO, TIPO_GELO, TIPO_GELO, TIPO_GELO, 
    TIPO_GELO, TIPO_GELO, TIPO_GELO, TIPO_GELO, 
    TIPO_GELO, TIPO_GELO, TIPO_GELO, 
    TIPO_GELO, TIPO_GELO,            
    TIPO_PEDRA,                       
    TIPO_SAIDA,                       
};

static TipoBloco tipoFase3[26] = {
    TIPO_ROCHA, TIPO_ROCHA, TIPO_ROCHA, 
    TIPO_ROCHA, TIPO_ROCHA, TIPO_ROCHA, TIPO_ROCHA, TIPO_ROCHA, 
    TIPO_ROCHA, TIPO_ROCHA, TIPO_ROCHA, TIPO_ROCHA, 
    TIPO_ROCHA, TIPO_ROCHA, TIPO_ROCHA, TIPO_ROCHA, 
    TIPO_ROCHA, TIPO_ROCHA, TIPO_ROCHA, TIPO_ROCHA, TIPO_ROCHA, 
    TIPO_ROCHA, TIPO_ROCHA, TIPO_ROCHA, TIPO_ROCHA, 
    TIPO_SAIDA, 
};

static TipoBloco *tipoAtual = NULL;




static void renderTerra(float x1,float y1,float x2,float y2) {
    float tx = tileCount(x1,x2,0.18f);
    float ty = tileCount(y1,y2,0.18f);
    quadTex(x1,y1,x2,y2, TEX_TERRA, tx,ty, 1.0f,1.0f,1.0f);
    bordaLinha(x1,y1,x2,y2, 0.12f,0.06f,0.02f, 1.5f);
    gramaTopo(x1,y2,x2);
}

static void renderMetal(float x1,float y1,float x2,float y2) {
    float tx = tileCount(x1,x2,0.15f);
    float ty = tileCount(y1,y2,0.15f);
    quadTex(x1,y1,x2,y2, TEX_METAL, tx,ty, 0.85f,0.88f,0.95f);
    bordaLinha(x1,y1,x2,y2, 0.30f,0.35f,0.50f, 1.5f);
    
    float rsize = 0.012f;
    glColor3f(0.50f,0.55f,0.70f);
    glPointSize(3.5f);
    glBegin(GL_POINTS);
        glVertex2f(x1+rsize, y1+rsize);
        glVertex2f(x2-rsize, y1+rsize);
        glVertex2f(x1+rsize, y2-rsize);
        glVertex2f(x2-rsize, y2-rsize);
    glEnd();
    neveTopo(x1,y2,x2, false);
}

static void renderPedra(float x1,float y1,float x2,float y2) {
    float tx = tileCount(x1,x2,0.20f);
    float ty = tileCount(y1,y2,0.20f);
    quadTex(x1,y1,x2,y2, TEX_PEDRA, tx,ty, 0.80f,0.84f,1.00f);
    bordaLinha(x1,y1,x2,y2, 0.28f,0.32f,0.55f, 1.5f);
    
    glColor3f(0.22f,0.25f,0.42f);
    glLineWidth(0.8f);
    float my=(y1+y2)*0.5f, mx=(x1+x2)*0.5f;
    glBegin(GL_LINES);
        glVertex2f(x1,my); glVertex2f(x2,my);
        glVertex2f(mx,y1); glVertex2f(mx,y2);
    glEnd();
    neveTopo(x1,y2,x2, false);
}

static void renderGelo(float x1,float y1,float x2,float y2) {
    float tx = tileCount(x1,x2,0.22f);
    float ty = tileCount(y1,y2,0.22f);
    
    quadTex(x1,y1,x2,y2, TEX_GELO, tx,ty, 0.75f,0.88f,1.00f);
    bordaLinha(x1,y1,x2,y2, 0.30f,0.55f,0.85f, 1.5f);
    
    glColor3f(0.70f,0.88f,1.00f);
    glLineWidth(0.8f);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0xAAAA);
    float step = 0.06f, xx;
    for(xx=x1; xx<x2; xx+=step){
        glBegin(GL_LINES);
            glVertex2f(xx, y1);
            glVertex2f(xx+0.04f, y2);
        glEnd();
    }
    glDisable(GL_LINE_STIPPLE);
    neveTopo(x1,y2,x2, true);
}

static void renderRocha(float x1,float y1,float x2,float y2) {
    float tx = tileCount(x1,x2,0.20f);
    float ty = tileCount(y1,y2,0.20f);
    quadTex(x1,y1,x2,y2, TEX_ROCHA, tx,ty, 0.75f,0.75f,0.82f);
    bordaLinha(x1,y1,x2,y2, 0.18f,0.18f,0.25f, 1.5f);
    
    glColor3f(0.12f,0.12f,0.18f);
    glLineWidth(1.0f);
    float mx=(x1+x2)*0.5f, my=(y1+y2)*0.5f;
    glBegin(GL_LINE_STRIP);
        glVertex2f(mx-0.01f, y2);
        glVertex2f(mx+0.015f, my+0.01f);
        glVertex2f(mx-0.01f, my-0.01f);
        glVertex2f(mx+0.005f, y1);
    glEnd();
    neveTopo(x1,y2,x2, true);
}

static void renderSaida(float x1,float y1,float x2,float y2) {
    float t = sinf(tempoEstrela*3.0f)*0.08f;
    if(faseAtual==3){
        
        glColor3f(0.75f+t, 0.65f+t, 0.10f);
        glBegin(GL_QUADS);
            glVertex2f(x1,y1);glVertex2f(x2,y1);
            glVertex2f(x2,y2);glVertex2f(x1,y2);
        glEnd();
        bordaLinha(x1,y1,x2,y2, 1.0f,0.9f,0.2f, 2.0f);
    } else {
        
        glColor3f(0.05f, 0.30f+t, 0.12f);
        glBegin(GL_QUADS);
            glVertex2f(x1,y1);glVertex2f(x2,y1);
            glVertex2f(x2,y2);glVertex2f(x1,y2);
        glEnd();
        bordaLinha(x1,y1,x2,y2, 0.10f,0.85f,0.35f, 2.0f);
    }
    
    float cx=(x1+x2)*0.5f, cy=(y1+y2)*0.5f;
    float ds=0.025f+t*0.5f;
    glColor3f(0.40f, 1.00f, 0.60f);
    glBegin(GL_QUADS);
        glVertex2f(cx,    cy+ds);
        glVertex2f(cx+ds, cy);
        glVertex2f(cx,    cy-ds);
        glVertex2f(cx-ds, cy);
    glEnd();
    glColor3f(1.0f,1.0f,1.0f);
    glRasterPos2f(cx-0.07f, y2+0.025f);
    const char *txt=(faseAtual==3)?"TOPO!":"NEXT >";
    while(*txt){glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,*txt);txt++;}
}




static void renderBloco(float x1,float y1,float x2,float y2, TipoBloco tipo){
    switch(tipo){
        case TIPO_TERRA: renderTerra(x1,y1,x2,y2); break;
        case TIPO_METAL: renderMetal(x1,y1,x2,y2); break;
        case TIPO_PEDRA: renderPedra(x1,y1,x2,y2); break;
        case TIPO_GELO:  renderGelo (x1,y1,x2,y2); break;
        case TIPO_ROCHA: renderRocha(x1,y1,x2,y2); break;
        case TIPO_SAIDA: renderSaida(x1,y1,x2,y2); break;
    }
}




bool ehEstrutura(int i){
    
    if(!tipoAtual) return false;
    int n = numBlocos;
    if(i<0||i>=n) return false;
    return tipoAtual[i] != TIPO_TERRA;
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
    tipoAtual    = tipoFase0;
    neveIniciada = false;
    initEstrelas();
    initTexturas();
    resetMoedas();
}


static void desenhaCeu(){
    
    float topo[3], base[3];
    if(faseAtual==0){
        
        topo[0]=0.04f; topo[1]=0.08f; topo[2]=0.14f;
        base[0]=0.01f; base[1]=0.02f; base[2]=0.04f;
    } else if(faseAtual==1){
        
        topo[0]=0.05f; topo[1]=0.06f; topo[2]=0.18f;
        base[0]=0.01f; base[1]=0.01f; base[2]=0.06f;
    } else if(faseAtual==2){
        
        topo[0]=0.04f; topo[1]=0.10f; topo[2]=0.22f;
        base[0]=0.01f; base[1]=0.03f; base[2]=0.08f;
    } else {
        
        topo[0]=0.08f; topo[1]=0.05f; topo[2]=0.14f;
        base[0]=0.02f; base[1]=0.01f; base[2]=0.04f;
    }
    glBegin(GL_QUADS);
        glColor3f(base[0],base[1],base[2]); glVertex2f(-1,-1);
        glColor3f(base[0],base[1],base[2]); glVertex2f( 1,-1);
        glColor3f(topo[0],topo[1],topo[2]); glVertex2f( 1, 1);
        glColor3f(topo[0],topo[1],topo[2]); glVertex2f(-1, 1);
    glEnd();
}

void renderMapa(){
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);

    
    desenhaCeu();

    
    desenhaEstrelas();

    
    if((faseAtual==2||faseAtual==3) && neveIniciada) desenhaNeveCaindo();

    
    int i;
    for(i=0; i<numBlocos; i++){
        Bloco b = blocos[i];
        TipoBloco tipo = tipoAtual[i];
        renderBloco(b.x1,b.y1,b.x2,b.y2, tipo);
    }

    
    renderEspinhos(faseAtual);

    
    renderMoedas();
}

void updateMapa(){
    tempoEstrela += 0.016f;
    if((faseAtual==2||faseAtual==3) && neveIniciada) atualizaNeve();
}

void avancarFase(){
    faseAtual++;
    if(faseAtual > 3) return;
    switch(faseAtual){
        case 1:
            blocos    = blocosFase1;
            numBlocos = sizeof(blocosFase1)/sizeof(Bloco);
            tipoAtual = tipoFase1;
            
            break;
        case 2:
            blocos    = blocosFase2;
            numBlocos = sizeof(blocosFase2)/sizeof(Bloco);
            tipoAtual = tipoFase2;
            
            audioTrocarMusica(MUSICA_2);
          
            break;
        case 3:
            blocos    = blocosFase3;
            numBlocos = sizeof(blocosFase3)/sizeof(Bloco);
            tipoAtual = tipoFase3;
            initNeve();
            break;
		
    }
}




#define MAX_MOEDAS 2

typedef struct { float x, y; bool ativa; } Moeda;

static Moeda moedasFase[4][MAX_MOEDAS] = {
    { {-0.20f, -0.10f, true}, { 0.40f,  0.05f, true} }, 
    { {-0.75f, -0.70f, true}, {-0.35f,  0.15f, true} }, 
    { { 0.0f,  -0.20f, true}, { 0.40f, -0.50f, true} }, 
    { {-0.05f,  0.60f, true}, { 0.75f,  0.65f, true} }, 
};

static int moedasPegas = 0;

void resetMoedas() {
    int f, m;
    for(f=0;f<4;f++)
        for(m=0;m<MAX_MOEDAS;m++)
            moedasFase[f][m].ativa = true;
    moedasPegas = 0;
}

int getMoedasPegas() { return moedasPegas; }
int getMoedasTotal() { return 4 * MAX_MOEDAS; }

int checaMoedas(float px, float py, float pw, float ph) {
    int m, pegou = 0;
    float raio = 0.06f;
    for(m = 0; m < MAX_MOEDAS; m++) {
        Moeda *mo = &moedasFase[faseAtual][m];
        if(!mo->ativa) continue;
        if(px+pw > mo->x-raio && px-pw < mo->x+raio &&
           py+ph > mo->y-raio && py    < mo->y+raio) {
            mo->ativa = false;
            moedasPegas++;
            pegou = 1;
        }
    }
    return pegou;
}

void renderMoedas() {
    int m;
    float raio = 0.04f;
    float t = tempoEstrela;
    for(m = 0; m < MAX_MOEDAS; m++) {
        Moeda mo = moedasFase[faseAtual][m];
        if(!mo.ativa) continue;
        float bob = 0.012f * (float)sin(t * 3.0f + m * 1.5f);
        float cx = mo.x;
        float cy = mo.y + bob;
        float giro = (float)fabs(sin(t * 4.0f + m * 2.0f));
        float raioX = raio * giro;
        float raioY = raio;
        glColor3f(0.4f, 0.3f, 0.0f);
        glBegin(GL_TRIANGLE_FAN);
        int k;
        for(k=0;k<16;k++){
            float a = k * 3.14159f * 2.0f / 16.0f;
            glVertex2f(cx + (raioX+0.004f)*(float)cos(a),
                       cy + (raioY+0.004f)*(float)sin(a));
        }
        glEnd();
        if(sin(t * 4.0f + m * 2.0f) > 0)
            glColor3f(1.0f, 0.82f, 0.0f);
        else
            glColor3f(0.8f, 0.55f, 0.0f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for(k=0;k<=16;k++){
            float a = k * 3.14159f * 2.0f / 16.0f;
            glVertex2f(cx + raioX*(float)cos(a),
                       cy + raioY*(float)sin(a));
        }
        glEnd();
        if(giro > 0.3f){
            glColor3f(1.0f, 1.0f, 0.6f);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(cx - raioX*0.2f, cy + raioY*0.2f);
            for(k=0;k<=10;k++){
                float a = k * 3.14159f * 2.0f / 10.0f;
                glVertex2f(cx - raioX*0.2f + raioX*0.45f*(float)cos(a),
                           cy + raioY*0.2f + raioY*0.45f*(float)sin(a));
            }
            glEnd();
            glColor3f(1.0f, 1.0f, 0.9f);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(cx + raioX*0.1f, cy - raioY*0.1f);
            for(k=0;k<=8;k++){
                float a = k * 3.14159f * 2.0f / 8.0f;
                glVertex2f(cx + raioX*0.1f + raioX*0.25f*(float)cos(a),
                           cy - raioY*0.1f + raioY*0.25f*(float)sin(a));
            }
            glEnd();
        }
    }
}

#define GLUT_DISABLE_ATEXIT_HACK
#include "../include/espinho.h"
#include "../include/GL/glut.h"
#include <stdbool.h>
#include <math.h>








#define CEL_W  0.014f   
#define CEL_H  0.011f   
#define ESP_W  (CEL_W * 3.0f)   
#define ESP_H  (CEL_H * 6.0f)   













static const int padrao[6][3] = {
    {1, 1, 1},   
    {1, 1, 1},   
    {1, 1, 1},   
    {1, 1, 1},   
    {0, 1, 0},   
    {0, 1, 0}    
};





static void desenhaEspinhoUnit(float bx, float by,
                                float r, float g, float b) {
    int row, col;

    
    for (row = 0; row < 6; row++) {
        for (col = 0; col < 3; col++) {
            if (!padrao[row][col]) continue;

            float x1 = bx + col * CEL_W;
            float y1 = by + row * CEL_H;
            float x2 = x1 + CEL_W;
            float y2 = y1 + CEL_H;

            glColor3f(r, g, b);
            glBegin(GL_QUADS);
                glVertex2f(x1, y1);
                glVertex2f(x2, y1);
                glVertex2f(x2, y2);
                glVertex2f(x1, y2);
            glEnd();
        }
    }

    
    
    float px = bx + CEL_W;                    
    float py = by + 5 * CEL_H;                

    
    glColor3f(r * 0.95f, g * 0.95f, b * 0.95f);
    glBegin(GL_TRIANGLES);
        glVertex2f(px + CEL_W*0.1f, py);                    
        glVertex2f(px + CEL_W*0.9f, py);                    
        glVertex2f(px + CEL_W*0.5f, py + CEL_H*1.15f);      
    glEnd();

    
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
        glVertex2f(px + CEL_W*0.35f, py + CEL_H*0.4f);
        glVertex2f(px + CEL_W*0.65f, py + CEL_H*0.4f);
        glVertex2f(px + CEL_W*0.5f, py + CEL_H*0.95f);
    glEnd();
}





static void desenhaZona(ZonaEspinho z, float r, float g, float b) {
    float largura = z.x2 - z.x1;
    int qtd = (int)(largura / ESP_W);
    if (qtd < 1) qtd = 1;

    
    float totalW = qtd * ESP_W;
    float offset = (largura - totalW) * 0.5f;

    int i;
    for (i = 0; i < qtd; i++) {
        float bx = z.x1 + offset + i * ESP_W;
        desenhaEspinhoUnit(bx, z.y, r, g, b);
    }
}







static ZonaEspinho espFase0[] = {
    {-0.67f, -0.30f, -0.98f}, 
    {-0.10f,  0.30f, -0.82f}, 
};
static int nEspFase0 = sizeof(espFase0)/sizeof(ZonaEspinho);


static ZonaEspinho espFase1[] = {
    { 0.10f,  0.25f, -0.90f}, 
    { 0.25f,  0.40f, -0.65f}, 
    { 0.40f,  0.70f, -0.80f}, 
    { 0.70f,  0.90f, -0.90f}, 
    { 0.90f,  0.95f, -0.85f}, 
};
static int nEspFase1 = sizeof(espFase1)/sizeof(ZonaEspinho);


static ZonaEspinho espFase2[] = {
    {-0.25f,  0.10f, -0.90f}, 
    { 0.10f,  0.20f, -0.80f}, 
    { 0.20f,  0.50f, -0.75f}, 
    { 0.12f,  0.37f,  0.38f}, 
    { 0.10f,  0.20f,  0.30f}, 
};
static int nEspFase2 = sizeof(espFase2)/sizeof(ZonaEspinho);



static ZonaEspinho espFase3[] = {
    {-0.50f, -0.10f, 0.10f}, 
    { 0.00f,  0.40f, 0.10f}, 
    { 0.50f,  0.60f, 0.10f}, 
};
static int nEspFase3 = sizeof(espFase3)/sizeof(ZonaEspinho);




static void corEspinho(int fase, float *r, float *g, float *b) {
    switch(fase) {
        case 0: *r=0.85f; *g=0.88f; *b=0.95f; break; 
        case 1: *r=0.80f; *g=0.82f; *b=0.92f; break; 
        case 2: *r=0.75f; *g=0.90f; *b=1.00f; break; 
        case 3: *r=0.90f; *g=0.90f; *b=0.95f; break; 
        default:*r=0.9f;  *g=0.9f;  *b=0.9f;  break;
    }
}




void renderEspinhos(int fase) {
    ZonaEspinho *zonas = NULL;
    int n = 0;
    switch(fase) {
        case 0: zonas=espFase0; n=nEspFase0; break;
        case 1: zonas=espFase1; n=nEspFase1; break;
        case 2: zonas=espFase2; n=nEspFase2; break;
        case 3: zonas=espFase3; n=nEspFase3; break;
    }
    if (!zonas) return;
    float r, g, b;
    corEspinho(fase, &r, &g, &b);
    int i;
    for (i = 0; i < n; i++) {
        desenhaZona(zonas[i], r, g, b);
    }
}






bool checaEspinhoColisao(float px, float py,
                          float pw, float ph, int fase) {
    ZonaEspinho *zonas = NULL;
    int n = 0;
    switch(fase) {
        case 0: zonas=espFase0; n=nEspFase0; break;
        case 1: zonas=espFase1; n=nEspFase1; break;
        case 2: zonas=espFase2; n=nEspFase2; break;
        case 3: zonas=espFase3; n=nEspFase3; break;
    }
    if (!zonas) return false;
    int i;
    for (i = 0; i < n; i++) {
        ZonaEspinho z = zonas[i];
        float eY1 = z.y;
        float eY2 = z.y + ESP_H;
        
        bool xOk = (px + pw > z.x1) && (px - pw < z.x2);
        bool yOk = (py + ph > eY1)  && (py      < eY2);
        if (xOk && yOk) return true;
    }
    return false;
}

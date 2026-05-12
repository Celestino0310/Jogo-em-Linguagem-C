#define GLUT_DISABLE_ATEXIT_HACK
#include "../include/espinho.h"
#include "../include/GL/glut.h"
#include <stdbool.h>
#include <math.h>

// ============================================================
// TAMANHO DE CADA UNIDADE DE ESPINHO
// Cada espinho = grade 3 colunas x 5 linhas de celulas
// ============================================================// ============================================================
// TAMANHO DE CADA UNIDADE DE ESPINHO (atualizado)
// Cada espinho = grade 3 colunas x 6 linhas de celulas
// ============================================================
#define CEL_W  0.014f   // largura de 1 célula
#define CEL_H  0.011f   // altura de 1 célula
#define ESP_W  (CEL_W * 3.0f)   // largura total = 0.042
#define ESP_H  (CEL_H * 6.0f)   // altura total aumentada = 0.066

// ============================================================
// NOVO PADRÃO DO ESPINHO (3 colunas x 6 linhas, de baixo pra cima)
// Mais parecido com Celeste: base sólida + corpo afunilando + ponta afiada
//
// col:  0  1  2
// row5: .  X  .     ? ponta mais pronunciada
// row4: .  X  .
// row3: X  X  X
// row2: X  X  X
// row1: X  X  X
// row0: X  X  X     ? base larga
// ============================================================
static const int padrao[6][3] = {
    {1, 1, 1},   // row 0 - base
    {1, 1, 1},   // row 1
    {1, 1, 1},   // row 2
    {1, 1, 1},   // row 3
    {0, 1, 0},   // row 4
    {0, 1, 0}    // row 5 - ponta
};

// ============================================================
// DESENHA UM ÚNICO ESPINHO (refatorado)
// bx, by = canto inferior esquerdo do espinho
// ============================================================
static void desenhaEspinhoUnit(float bx, float by,
                                float r, float g, float b) {
    int row, col;

    // Desenha as células do corpo principal
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

    // === MELHORIA VISUAL: Ponta mais afiada e com brilho melhor ===
    // Ponta principal (row 5, col 1)
    float px = bx + CEL_W;                    // centro da coluna do meio
    float py = by + 5 * CEL_H;                // base da ponta

    // Corpo da ponta (um pouco mais largo na base da ponta)
    glColor3f(r * 0.95f, g * 0.95f, b * 0.95f);
    glBegin(GL_TRIANGLES);
        glVertex2f(px + CEL_W*0.1f, py);                    // base esquerda
        glVertex2f(px + CEL_W*0.9f, py);                    // base direita
        glVertex2f(px + CEL_W*0.5f, py + CEL_H*1.15f);      // vértice superior (mais alto)
    glEnd();

    // Brilho na ponta (highlight)
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
        glVertex2f(px + CEL_W*0.35f, py + CEL_H*0.4f);
        glVertex2f(px + CEL_W*0.65f, py + CEL_H*0.4f);
        glVertex2f(px + CEL_W*0.5f, py + CEL_H*0.95f);
    glEnd();
}

// ============================================================
// DESENHA UMA CADEIA DE ESPINHOS sobre uma zona
// A zona vai de x1 a x2, base em y
// ============================================================
static void desenhaZona(ZonaEspinho z, float r, float g, float b) {
    float largura = z.x2 - z.x1;
    int qtd = (int)(largura / ESP_W);
    if (qtd < 1) qtd = 1;

    // centraliza a cadeia dentro da zona
    float totalW = qtd * ESP_W;
    float offset = (largura - totalW) * 0.5f;

    int i;
    for (i = 0; i < qtd; i++) {
        float bx = z.x1 + offset + i * ESP_W;
        desenhaEspinhoUnit(bx, z.y, r, g, b);
    }
}

// ============================================================
// ZONAS DE ESPINHOS POR FASE
// y = y2 (topo) do bloco marcado com //espinho
// ============================================================

// FASE 0
static ZonaEspinho espFase0[] = {
    {-0.67f, -0.30f, -0.98f}, // bloco 4
    {-0.10f,  0.30f, -0.82f}, // bloco 6
};
static int nEspFase0 = sizeof(espFase0)/sizeof(ZonaEspinho);

// FASE 1
static ZonaEspinho espFase1[] = {
    { 0.10f,  0.25f, -0.90f}, // bloco 11
    { 0.25f,  0.40f, -0.65f}, // bloco 12
    { 0.40f,  0.70f, -0.80f}, // bloco 15
    { 0.70f,  0.90f, -0.90f}, // bloco 16
    { 0.90f,  0.95f, -0.85f}, // bloco 17
};
static int nEspFase1 = sizeof(espFase1)/sizeof(ZonaEspinho);

// FASE 2
static ZonaEspinho espFase2[] = {
    {-0.25f,  0.10f, -0.90f}, // bloco 9
    { 0.10f,  0.20f, -0.80f}, // bloco 10
    { 0.20f,  0.50f, -0.75f}, // bloco 11
    { 0.12f,  0.37f,  0.38f}, // bloco 13
    { 0.10f,  0.20f,  0.30f}, // bloco 14
};
static int nEspFase2 = sizeof(espFase2)/sizeof(ZonaEspinho);

// FASE 3
// FASE 3 - apenas os 3 blocos marcados com //espinho
static ZonaEspinho espFase3[] = {
    {-0.50f, -0.10f, 0.10f}, // espinho 1 - bloco 11 (topo = y2 = 0.10)
    { 0.00f,  0.40f, 0.10f}, // espinho 2 - bloco 23 (topo = y2 = 0.10)
    { 0.50f,  0.60f, 0.10f}, // espinho 3 - bloco 20 (topo = y2 = 0.10)
};
static int nEspFase3 = sizeof(espFase3)/sizeof(ZonaEspinho);

// ============================================================
// CORES POR FASE (estilo celeste)
// ============================================================
static void corEspinho(int fase, float *r, float *g, float *b) {
    switch(fase) {
        case 0: *r=0.85f; *g=0.88f; *b=0.95f; break; // branco azulado - floresta
        case 1: *r=0.80f; *g=0.82f; *b=0.92f; break; // cinza claro - cidade
        case 2: *r=0.75f; *g=0.90f; *b=1.00f; break; // azul gelo
        case 3: *r=0.90f; *g=0.90f; *b=0.95f; break; // branco neve
        default:*r=0.9f;  *g=0.9f;  *b=0.9f;  break;
    }
}

// ============================================================
// RENDER
// ============================================================
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

// ============================================================
// COLISAO
// Retorna true se o jogador toca em qualquer zona de espinho
// px = centro X, py = base Y, pw = meia-largura, ph = altura
// ============================================================
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
        // AABB simples entre jogador e zona de espinhos
        bool xOk = (px + pw > z.x1) && (px - pw < z.x2);
        bool yOk = (py + ph > eY1)  && (py      < eY2);
        if (xOk && yOk) return true;
    }
    return false;
}

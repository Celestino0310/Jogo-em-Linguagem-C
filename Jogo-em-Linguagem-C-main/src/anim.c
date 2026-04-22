#define GLUT_DISABLE_ATEXIT_HACK
#include "../include/anim.h"
#include "../include/GL/glut.h"
#include "../include/stb_image.h"   /* implementação já está em mapa.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ─────────────────────────────────────────────────────────────
   CONFIGURAÇÃO DA SPRITESHEET
   Ajuste os valores abaixo para bater com a sua imagem.
   Use o SpriteAnalyzer (spritesheet_analyzer.html) para
   descobrir os valores corretos de col/row de cada animação.
   ──────────────────────────────────────────────────────────── */
#define SHEET_PATH    "../assets/textures/althea_sheet.png"

/* Tamanho de cada frame em pixels */
#define TILE_W   55
#define TILE_H   90

/* Offset em pixels onde os tiles começam na imagem */
#define OFFSET_X 0
#define OFFSET_Y 0

/* ─────────────────────────────────────────────────────────────
   DEFINIÇÃO DOS FRAMES (col, row na spritesheet)
   col/row contam a partir de 0, começando do OFFSET acima.
   Cada linha abaixo representa um frame: {coluna, linha}.
   AJUSTE ESSES VALORES com o SpriteAnalyzer!
   ──────────────────────────────────────────────────────────── */
typedef struct { int col, row; } Quadro;

#define F(c,r) {(c),(r)}

/* Idle lateral — 2 frames */
static Quadro qIdle[] = { F(2,0), F(3,0) };

/* Caminhada lateral — 8 frames */
static Quadro qWalk[] = {
    F(5,0), F(6,0), F(7,0), F(0,1),
    F(1,1)
};

/* Pulo (subida) — 3 frames, trava no último */
static Quadro qJump[] = { F(2,1), F(3,1), F(3,1) };

/* Queda — 2 frames, trava no último */
static Quadro qFall[] = { F(2,1), F(2,1) };

/* Rush do dash — 4 frames */
static Quadro qDash[] = { F(2,1), F(3,1), F(3,1), F(3,1) };

/* Frenagem do dash — 4 frames one-shot */
static Quadro qDashBrake[] = { F(3,1), F(2,1), F(2,1), F(2,1) };

#undef F

/* ─────────────────────────────────────────────────────────────
   DEFINIÇÃO DE CADA ANIMAÇÃO
   ──────────────────────────────────────────────────────────── */
typedef struct {
    Quadro *quadros;
    int     total;
    float   anim_spd; /* frames por segundo */
    int     loop;     /* 1 = loop, 0 = one-shot (trava no último) */
} DefAnim;

static DefAnim defs[ANIM_COUNT] = {
    /* ANIM_IDLE       */ { qIdle,      2,  6.0f, 1 },
    /* ANIM_WALK       */ { qWalk,      8, 12.0f, 1 },
    /* ANIM_JUMP       */ { qJump,      3, 10.0f, 0 },
    /* ANIM_FALL       */ { qFall,      2,  8.0f, 0 },
    /* ANIM_DASH       */ { qDash,      4, 20.0f, 1 },
    /* ANIM_DASH_BRAKE */ { qDashBrake, 4, 14.0f, 0 },
};

/* ─────────────────────────────────────────────────────────────
   ESTADO INTERNO
   ──────────────────────────────────────────────────────────── */
static unsigned int g_texID    = 0;
static int          g_texW     = 0;
static int          g_texH     = 0;
static int          g_ok       = 0;   /* 1 = spritesheet carregada */

static AnimEstado   g_estado   = ANIM_IDLE;
static int          g_quadro   = 0;
static float        g_timer    = 0.0f;
static int          g_espelhar = 0;   /* 1 = flip horizontal */
static int          g_finalizado = 0;

/* ─────────────────────────────────────────────────────────────
   CARREGA A SPRITESHEET
   ──────────────────────────────────────────────────────────── */
void animInit(void)
{
    int ch;
    stbi_set_flip_vertically_on_load(1);
    unsigned char *dados = stbi_load(SHEET_PATH, &g_texW, &g_texH, &ch, 4);

    if (!dados) {
        printf("[ANIM] Spritesheet nao encontrada: %s\n", SHEET_PATH);
        printf("[ANIM] Modo fallback ativo (retangulo colorido).\n");
        printf("[ANIM] Coloque a imagem em assets/textures/althea_sheet.png\n");
        g_ok = 0;
        return;
    }

    glGenTextures(1, &g_texID);
    glBindTexture(GL_TEXTURE_2D, g_texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 g_texW, g_texH, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, dados);
    stbi_image_free(dados);

    g_ok = 1;
    g_estado = ANIM_IDLE;
    g_quadro  = 0;
    g_timer   = 0.0f;
    printf("[ANIM] Spritesheet OK: %s (%dx%d)\n", SHEET_PATH, g_texW, g_texH);
}

int animCarregada(void) { return g_ok; }

/* ─────────────────────────────────────────────────────────────
   MUDA O ESTADO (reseta o frame se mudou)
   ──────────────────────────────────────────────────────────── */
static void _setEstado(AnimEstado novo)
{
    if (novo == g_estado) return;
    g_estado    = novo;
    g_quadro    = 0;
    g_timer     = 0.0f;
    g_finalizado = 0;
}

/* ─────────────────────────────────────────────────────────────
   UPDATE — chamado todo frame
   ──────────────────────────────────────────────────────────── */
void animUpdate(float dt,
                float velX, float velY,
                int   noChao,
                int   podeDash, float tempoDash,
                int   direcao)
{
    /* ── 1. Determina direção (flip) ── */
    if (direcao < 0) g_espelhar = 1;
    else             g_espelhar = 0;

    /* ── 2. Determina estado ── */
    AnimEstado novo = g_estado;

    if (!noChao) {
        if (velY > 0.002f)  novo = ANIM_JUMP;
        else                novo = ANIM_FALL;
    } else if (!podeDash) {
        /* Q foi pressionado: primeiro momento = dash, depois = brake */
        if (tempoDash < 0.25f) novo = ANIM_DASH;
        else                   novo = ANIM_DASH_BRAKE;
    } else {
        float absVX = velX < 0 ? -velX : velX;
        if (absVX > 0.003f) novo = ANIM_WALK;
        else                novo = ANIM_IDLE;
    }

    /* one-shot terminado → volta ao idle */
    if (g_finalizado) {
        if (novo == ANIM_DASH || novo == ANIM_DASH_BRAKE)
            novo = noChao ? ANIM_IDLE : ANIM_FALL;
    }

    _setEstado(novo);

    /* ── 3. Avança frame ── */
    if (!g_ok) return;

    DefAnim *def = &defs[g_estado];
    if (g_finalizado) return;

    g_timer += dt;
    float dur = 1.0f / def->anim_spd;

    while (g_timer >= dur) {
        g_timer -= dur;
        g_quadro++;
        if (g_quadro >= def->total) {
            if (def->loop) {
                g_quadro = 0;
            } else {
                g_quadro     = def->total - 1;
                g_finalizado = 1;
                break;
            }
        }
    }
}

/* ─────────────────────────────────────────────────────────────
   DRAW — desenha o frame atual com OpenGL
   posX/posY = posição central-inferior do personagem
   ──────────────────────────────────────────────────────────── */
void animDraw(float posX, float posY, float largura, float altura)
{
    if (!g_ok) {
        /* ── Fallback: retângulo roxo original ── */
        float hw = largura * 0.5f;

        /* corpo */
        glColor3f(0.6f, 0.2f, 0.8f);
        glBegin(GL_QUADS);
            glVertex2f(posX - hw, posY);
            glVertex2f(posX + hw, posY);
            glVertex2f(posX + hw, posY + altura);
            glVertex2f(posX - hw, posY + altura);
        glEnd();
        /* borda */
        glColor3f(0, 0, 0); glLineWidth(1.5f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(posX - hw, posY);
            glVertex2f(posX + hw, posY);
            glVertex2f(posX + hw, posY + altura);
            glVertex2f(posX - hw, posY + altura);
        glEnd();
        return;
    }

    /* ── Sprite com textura ── */
    DefAnim *def  = &defs[g_estado];
    Quadro  *quad = &def->quadros[g_quadro];

    /* Calcula UV na spritesheet */
    float u0 = (float)(OFFSET_X + quad->col * TILE_W) / (float)g_texW;
    float u1 = u0 + (float)TILE_W / (float)g_texW;
    float v0 = (float)(g_texH - OFFSET_Y - (quad->row + 1) * TILE_H) / (float)g_texH;
    float v1 = v0 + (float)TILE_H / (float)g_texH;

    /* Flip horizontal */
    if (g_espelhar) {
        float tmp = u0; u0 = u1; u1 = tmp;
    }

    float x0 = posX - largura * 0.5f;
    float x1 = posX + largura * 0.5f;
    float y0 = posY;
    float y1 = posY + altura;

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, g_texID);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
        glTexCoord2f(u0, v0); glVertex2f(x0, y0);
        glTexCoord2f(u1, v0); glVertex2f(x1, y0);
        glTexCoord2f(u1, v1); glVertex2f(x1, y1);
        glTexCoord2f(u0, v1); glVertex2f(x0, y1);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

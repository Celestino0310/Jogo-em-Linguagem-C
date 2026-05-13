#ifndef ANIM_H
#define ANIM_H

/* ─────────────────────────────────────────────────────────────
   Sistema de Animação da Althea — OpenGL / GLUT
   Compatível com o projeto existente (não usa raylib).
   stb_image já está compilado via mapa.c, então apenas
   incluímos o header aqui sem redefinir a implementação.
   ──────────────────────────────────────────────────────────── */

/* Estados possíveis do personagem */
typedef enum {
    ANIM_IDLE  = 0,
    ANIM_WALK,
    ANIM_JUMP,
    ANIM_FALL,
    ANIM_DASH,
    ANIM_DASH_BRAKE,
    ANIM_COUNT
} AnimEstado;


void animInit(void);

/* Atualiza o estado da animação a cada frame.
   Receba as variáveis internas do jogador diretamente.
   Chame no final de updateGame().                        */
void animUpdate(float dt,
                float velX, float velY,
                int   noChao,
                int   podeDash, float tempoDash,
                int   direcao);

/* Desenha o frame atual na posição do jogador.
   posX/posY = posição central-inferior do personagem
   (mesmas coordenadas usadas pela física).
   largura/altura = tamanho visual em coords GL.          */
void animDraw(float posX, float posY,
              float largura, float altura);

/* Retorna 1 se a spritesheet foi carregada com sucesso,
   0 se está em modo fallback (retângulo colorido).       */
int  animCarregada(void);

#endif /* ANIM_H */

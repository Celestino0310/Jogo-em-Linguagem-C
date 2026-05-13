#ifndef ANIM_H
#define ANIM_H









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




void animUpdate(float dt,
                float velX, float velY,
                int   noChao,
                int   podeDash, float tempoDash,
                int   direcao);





void animDraw(float posX, float posY,
              float largura, float altura);



int  animCarregada(void);

#endif 

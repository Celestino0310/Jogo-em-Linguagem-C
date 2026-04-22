#define GLUT_DISABLE_ATEXIT_HACK
#include "../include/game.h"
#include "../include/mapa.h"
#include "../include/GL/glut.h"
#include <stdbool.h>
#include <stdlib.h>

extern int gameState;

// ============================================================
// JOGADOR
// ============================================================
static int Health = 3;
static float posX = -0.75f;
static float posY = -0.80f;
static float velX = 0.0f;
static float velY = 0.0f;
static float gravidade = -0.002f;

static bool noChao = true;
static bool onLeftWall = false;
static bool onRightWall = false;
static bool isClinging = false;

static int direcao = 1;


static float jumpForce = 0.038f;
static int jumpsRestantes = 1;
static int maxJumps = 2;


static int extraJumpsAvailable = 0;
static float extraJumpCooldown = 0.0f;
const float EXTRA_JUMP_COOLDOWN_TIME = 1.5f;


static bool podeDash = true;
static float tempoDash = 0.0f;
static float cooldownDash = 3.0f;


static bool isDashing = false;
static float dashVelX = 0.0f;
static float dashVelY = 0.0f;
static float dashTimer = 0.0f;
static const float DASH_DURACAO   = 0.18f;  
static const float DASH_FORCA     = 0.045f;
static const float DASH_FORCA_DIAG = 0.032f; 

static bool kA=false, kD=false, kW=false, kSpace=false;


static bool kSpaceConsumed = false;


static float clingSlideSpeed = -0.008f;
static float wallJumpXForce = 0.27f;
static float wallJumpYForce = 0.043f;

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
    noChao = false;
    onLeftWall = false;
    onRightWall = false;

    float atrito = (faseAtual==2)?0.05f:0.28f;

    int i;
    for(i = 0; i < numBlocos; i++){
        if(!colideCom(blocos[i])) continue;

        Bloco b = blocos[i];
        float oB = b.y2 - posY;
        float oC = (posY + PH) - b.y1;
        float oD = (posX + PW) - b.x1;
        float oE = b.x2 - (posX - PW);

        float mH = oB < oC ? oB : oC;
        float mV = oD < oE ? oD : oE;

        if(mH < mV){  
            if(oB < oC){
                posY = b.y2;
                velY = 0;
                noChao = true;
                podeDash = true;
                extraJumpsAvailable = 1;
                jumpsRestantes = maxJumps;
            }
            else {
                posY = b.y1 - PH;
                velY = 0;
            }
        }
        else {  
            if(oD < oE){
                posX -= oD;
                velX = 0;
                onLeftWall = true;
            }
            else {
                posX += oE;
                velX = 0;
                onRightWall = true;
            }
        }
    }

    if(noChao) velX *= (1.0f - atrito);
}

static void checaSaida(){
    int idx=indiceSaida();
    if(idx<0||idx>=numBlocos) return;
    Bloco s=blocos[idx];
    if(posX+PW>s.x1 && posX-PW<s.x2 &&
       posY>=s.y2-0.08f && posY<=s.y2+0.08f && noChao){
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
    isDashing=false; dashVelX=0; dashVelY=0; dashTimer=0;
    kA=kD=kW=kSpace=false;
    kSpaceConsumed=false;
    initMapa();
}

void renderGame(){
    renderMapa();

   
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
    
    glColor3f(1.0f,0.8f,0.6f);
    glBegin(GL_QUADS);
        glVertex2f(posX-0.03f,posY+0.09f); glVertex2f(posX+0.03f,posY+0.09f);
        glVertex2f(posX+0.03f,posY+0.14f); glVertex2f(posX-0.03f,posY+0.14f);
    glEnd();
  
    float ox=(direcao>0)?posX+0.017f:posX-0.017f;
    glColor3f(0,0,0);
    glBegin(GL_QUADS);
        glVertex2f(ox-0.009f,posY+0.119f); glVertex2f(ox+0.009f,posY+0.119f);
        glVertex2f(ox+0.009f,posY+0.125f); glVertex2f(ox-0.009f,posY+0.125f);
    glEnd();
   
    float oax=(direcao>0)?posX+0.015f:posX-0.015f;
    glColor3f(0.3f,0.0f,0.5f);
    glBegin(GL_QUADS);
        glVertex2f(oax-0.009f,posY+0.018f); glVertex2f(oax+0.009f,posY+0.018f);
        glVertex2f(oax+0.009f,posY+0.068f); glVertex2f(oax-0.009f,posY+0.068f);
    glEnd();

    
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

    
    const char *nomes[4]={"Mapa 1 - Floresta","Mapa 2 - Cidade","Mapa 3 - Geleiras","Mapa Final - Pico"};
    glColor3f(0.5f,0.6f,0.9f);
    glRasterPos2f(0.25f,0.92f);
    const char *fn=nomes[faseAtual]; while(*fn){glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,*fn);fn++;}

    
    glColor3f(0.20f,0.22f,0.35f);
    glRasterPos2f(-0.98f,-0.96f);
    const char *hud="WASD/SPACE=mover/pular  Q=dash  Q+D=dash dir  Q+A=dash esq  Q+W+D/A=diagonal  ESC=menu";
    while(*hud){glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,*hud);hud++;}

    glutSwapBuffers();
}

void updateGame(){
    
    if(extraJumpCooldown > 0.0f) extraJumpCooldown -= 0.016f;
    if(!podeDash){
        tempoDash += 0.016f;
        if(tempoDash >= cooldownDash){
            podeDash = true;
            tempoDash = 0.0f;
        }
    }

    
    if(isDashing){
        dashTimer -= 0.016f;
        
        velX += (dashVelX - velX) * 0.35f;
        velY += (dashVelY - velY) * 0.35f;
        if(dashTimer <= 0.0f){
            isDashing = false;
           
            velX *= 0.5f;
            velY *= 0.3f;
        }
    }

   
    isClinging = false;
    if(onLeftWall  && kA) isClinging = true;
    if(onRightWall && kD) isClinging = true;

    if(kSpace && !kSpaceConsumed){
        bool pulou = false;

        if(isClinging){
            
            float jumpX = 0.0f;
            float jumpY = wallJumpYForce;

            if(kA)      jumpX = -wallJumpXForce;
            else if(kD) jumpX =  wallJumpXForce;
            else {
                if(onLeftWall)  jumpX = 0.17f;
                if(onRightWall) jumpX = -0.17f;
            }

            velX = jumpX;
            velY = jumpY;
            extraJumpsAvailable = 0;
            pulou = true;
        }
        else if(noChao){
            velY = jumpForce;
            jumpsRestantes = maxJumps - 1;
            noChao = false;
            pulou = true;
        }
        else if(jumpsRestantes > 0){
            velY = jumpForce * 0.9f;
            jumpsRestantes--;
            pulou = true;
        }
        else if(extraJumpsAvailable > 0 && extraJumpCooldown <= 0.0f && velY < -0.005f){
            velY = jumpForce * 0.85f;
            extraJumpsAvailable = 0;
            extraJumpCooldown = EXTRA_JUMP_COOLDOWN_TIME;
            pulou = true;
        }

        if(pulou){
            kSpaceConsumed = true;  
        }
    }

    
    float velAlvo = 0.0f;
    float lerp = (faseAtual==2)?0.10f:0.25f;

    if(kA){ velAlvo = -0.018f; direcao = -1; }
    if(kD){ velAlvo =  0.018f; direcao =  1; }

    velX += (velAlvo - velX) * lerp;

    
    if(isDashing){
        
    } else if(isClinging){
        
        if(velY < clingSlideSpeed) velY = clingSlideSpeed;
        velX *= 0.6f;
        jumpsRestantes = maxJumps;
    } else {
        velY += gravidade;
        if(velY < -0.065f) velY = -0.065f;
    }

    posX += velX;
    posY += velY;

    resolveColisoes();
    checaSaida();
    updateMapa();

  
    if(posY < -1.2f){
        Health--;
        if(Health <= 0){
            gameState = 0;
            initGame();
            return;
        }
        posX = -0.75f;
        posY = -0.70f;
        velX = 0;
        velY = 0;
        noChao = true;
        extraJumpsAvailable = 1;
    }
}

void handleGameInput(unsigned char tecla){
    switch(tecla){
        case 'a':case 'A': kA=true;     break;
        case 'd':case 'D': kD=true;     break;
        case 'w':case 'W': kW=true;     break;
        case ' ':          kSpace=true; break;
        case 'q':case 'Q':
            if(podeDash){
                
                bool diagDir  = kW && kD;
                bool diagEsq  = kW && kA;
                bool horizDir = !kW && kD;
                bool horizEsq = !kW && kA;

                if(diagDir){
                    dashVelX =  DASH_FORCA_DIAG;
                    dashVelY =  DASH_FORCA_DIAG;
                } else if(diagEsq){
                    dashVelX = -DASH_FORCA_DIAG;
                    dashVelY =  DASH_FORCA_DIAG;
                } else if(horizDir){
                    dashVelX =  DASH_FORCA;
                    dashVelY =  0.0f;
                } else if(horizEsq){
                    dashVelX = -DASH_FORCA;
                    dashVelY =  0.0f;
                } else {
                    
                    dashVelX = DASH_FORCA * direcao;
                    dashVelY = 0.0f;
                }

                isDashing  = true;
                dashTimer  = DASH_DURACAO;
                podeDash   = false;
                tempoDash  = 0.0f;
            }
            break;
        case 27: gameState=0; initGame(); break;
    }
}

void handleGameInputUp(unsigned char tecla){
    switch(tecla){
        case 'a':case 'A': kA=false;    break;
        case 'd':case 'D': kD=false;    break;
        case 'w':case 'W': kW=false;    break;
        case ' ':
            kSpace=false;
            kSpaceConsumed=false; 
            break;
    }
}

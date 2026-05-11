#define GLUT_DISABLE_ATEXIT_HACK
#include "../include/game.h"
#include "../include/mapa.h"
#include "../include/espinho.h"
#include "../include/GL/glut.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

extern int gameState;

// ============================================================
// ESTADO GLOBAL
// ============================================================
bool pausado       = false;
static int  opcaoPause   = 0;
static bool vitoriaFinal = false;
static float timerFase   = 0.0f;
static float timerTotal  = 0.0f;

// ============================================================
// JOGADOR
// ============================================================
static int   Health    = 3;
static float posX      = -0.75f;
static float posY      = -0.80f;
static float velX      =  0.0f;
static float velY      =  0.0f;
static float gravidade = -0.002f;
static int   direcao   =  1;

// Chao / paredes
static bool noChao      = true;
static bool onLeftWall  = false;
static bool onRightWall = false;
static bool isClinging  = false;

// Respawn
static float respawnX = -0.75f;
static float respawnY = -0.70f;

// Pulos
static float jumpForce      = 0.038f;
static int   jumpsRestantes = 1;
static int   maxJumps       = 2;  // chao + 1 extra no ar

// Dash suave + direcional
static bool  podeDash        = true;   // dash horizontal/solo
static bool  podeDashDiag    = true;   // dash diagonal — cooldown proprio
static float tempoDash       = 0.0f;
static float tempoDashDiag   = 0.0f;
static float cooldownDash    = 3.0f;   // cooldown dash normal
static float cooldownDashDiag= 0.0f;  // 0 = diagonal infinito; 3.0f = com cooldown
static bool  isDashing       = false;
static float dashVelX        = 0.0f;
static float dashVelY        = 0.0f;
static float dashTimer       = 0.0f;
static const float DASH_DURACAO    = 0.18f;
static const float DASH_FORCA      = 0.045f;
static const float DASH_FORCA_DIAG = 0.032f;

// Teclas
static bool kA=false, kD=false, kW=false, kSpace=false;
static bool kSpaceConsumed  = false;
static bool noChaoAnterior  = true;
static float clingCooldownL = 0.0f;   // cooldown parede ESQUERDA
static float clingCooldownR = 0.0f;   // cooldown parede DIREITA

// Wall Cling
static float clingSlideSpeed = 0.0f;     // parado na parede
static float wallJumpXForce  = 0.045f;
static float wallJumpYForce  = 0.028f;

// ============================================================
// COLISAO COM BLOCOS
// ============================================================
#define PW 0.055f
#define PH 0.090f

static bool colideCom(Bloco b){
    return posX+PW>b.x1 && posX-PW<b.x2 &&
           posY+PH>b.y1 && posY<b.y2;
}

// Sensor de parede: detecta bloco colado ao lado sem depender da colisao do frame
#define WALL_SENSOR 0.012f
static void detectaParedes(){
    int i;
    onLeftWall  = false;
    onRightWall = false;
    for(i = 0; i < numBlocos; i++){
        Bloco b = blocos[i];
        bool alturaOk = (posY + PH > b.y1 + 0.01f) && (posY < b.y2 - 0.01f);
        if(!alturaOk) continue;
        if(b.x1 >= posX+PW - 0.002f && b.x1 <= posX+PW + WALL_SENSOR) onRightWall = true;
        if(b.x2 <= posX-PW + 0.002f && b.x2 >= posX-PW - WALL_SENSOR) onLeftWall  = true;
    }
}

static void resolveColisoes(){
    noChao = false;
    float atrito = (faseAtual==2)?0.18f:0.28f;
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
            if(oB < oC){ posY=b.y2; velY=0; noChao=true; jumpsRestantes=maxJumps; }
            else        { posY=b.y1-PH; velY=0; }
        } else {
            if(oD < oE) posX -= oD; else posX += oE;
            velX = 0;
        }
    }
    if(noChao) velX *= (1.0f - atrito);
    detectaParedes();
}

// ============================================================
// MORTE / RESPAWN
// ============================================================
static void morrer(){
    Health--;
    if(Health <= 0){ gameState=3; return; }  // gameState 3 = tela de derrota
    posX=respawnX; posY=respawnY;
    velX=0; velY=0;
    noChao=true;
    isDashing=false;
    jumpsRestantes=maxJumps;
}

// ============================================================
// SAIDA DE FASE
// ============================================================
static void checaSaida(){
    int idx = indiceSaida();
    if(idx < 0 || idx >= numBlocos) return;
    Bloco s = blocos[idx];
    if(posX+PW>s.x1 && posX-PW<s.x2 &&
       posY>=s.y2-0.08f && posY<=s.y2+0.08f && noChao){
        kA=false; kD=false; kW=false; kSpace=false;
        if(faseAtual < 3){
            vitoriaFinal = false;
            gameState = 2;   // gameState 2 = tela de vitoria de fase
        } else {
            vitoriaFinal = true;
            gameState = 2;   // gameState 2 = tela de vitoria final
        }
    }
}

// ============================================================
// HELPER TEXTO
// ============================================================
static void drawText(float x, float y, const char *txt, void *fonte){
    glRasterPos2f(x, y);
    while(*txt){ glutBitmapCharacter(fonte, *txt); txt++; }
}

// ============================================================
// PUBLICAS
// ============================================================
void initGame(){
    Health = 3;
    posX=-0.75f; posY=-0.80f;
    respawnX=-0.75f; respawnY=-0.70f;
    velX=0; velY=0;
    noChao=true;
    podeDash=true; podeDashDiag=true;
    tempoDash=0; tempoDashDiag=0;
    isDashing=false; dashVelX=0; dashVelY=0; dashTimer=0;
    jumpsRestantes=maxJumps;
    clingCooldownL=0.0f; clingCooldownR=0.0f;
    kA=kD=kW=kSpace=false;
    kSpaceConsumed=false;
    pausado=false; opcaoPause=0;
    vitoriaFinal=false;
    timerFase=0.0f; timerTotal=0.0f;
    initMapa();
}

// ============================================================
// RENDER GAME
// ============================================================
void renderGame(){
    renderMapa();
    renderEspinhos(faseAtual);

    // personagem
    glColor3f(0.6f,0.2f,0.8f);
    glBegin(GL_QUADS);
        glVertex2f(posX-(PW-0.025f),posY); glVertex2f(posX+(PW-0.025f),posY);
        glVertex2f(posX+(PW-0.025f),posY+PH); glVertex2f(posX-(PW-0.025f),posY+PH);
    glEnd();
    glColor3f(0,0,0); glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(posX-(PW-0.025f),posY); glVertex2f(posX+(PW-0.025f),posY);
        glVertex2f(posX+(PW-0.025f),posY+PH); glVertex2f(posX-(PW-0.025f),posY+PH);
    glEnd();
    // cabeca
    glColor3f(1.0f,0.8f,0.6f);
    glBegin(GL_QUADS);
        glVertex2f(posX-0.03f,posY+0.09f); glVertex2f(posX+0.03f,posY+0.09f);
        glVertex2f(posX+0.03f,posY+0.14f); glVertex2f(posX-0.03f,posY+0.14f);
    glEnd();
    // olho
    float ox = (direcao>0)?posX+0.017f:posX-0.017f;
    glColor3f(0,0,0);
    glBegin(GL_QUADS);
        glVertex2f(ox-0.009f,posY+0.119f); glVertex2f(ox+0.009f,posY+0.119f);
        glVertex2f(ox+0.009f,posY+0.125f); glVertex2f(ox-0.009f,posY+0.125f);
    glEnd();
    // braco
    float oax = (direcao>0)?posX+0.015f:posX-0.015f;
    glColor3f(0.3f,0.0f,0.5f);
    glBegin(GL_QUADS);
        glVertex2f(oax-0.009f,posY+0.018f); glVertex2f(oax+0.009f,posY+0.018f);
        glVertex2f(oax+0.009f,posY+0.068f); glVertex2f(oax-0.009f,posY+0.068f);
    glEnd();

    // HUD vidas
    int h;
    for(h=0;h<3;h++){
        float cor = (h<Health)?0.9f:0.2f;
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
        float prog = tempoDash/cooldownDash;
        glColor3f(0.2f,0.2f,0.5f);
        glBegin(GL_QUADS);
            glVertex2f(-0.98f,0.80f); glVertex2f(-0.98f+prog*0.38f,0.80f);
            glVertex2f(-0.98f+prog*0.38f,0.84f); glVertex2f(-0.98f,0.84f);
        glEnd();
        glColor3f(0.5f,0.5f,1.0f);
        drawText(-0.98f, 0.85f, "DASH", GLUT_BITMAP_HELVETICA_12);
    }

    // nome fase
    const char *nomes[4]={"Mapa 1","Mapa 2","Mapa 3 - Geleiras","Mapa Final"};
    glColor3f(0.5f,0.6f,0.9f);
    drawText(0.25f, 0.92f, nomes[faseAtual], GLUT_BITMAP_HELVETICA_12);

    // timer — canto superior direito
    char bufTimer[32];
    int min  = (int)(timerFase / 60.0f);
    int seg  = (int)(timerFase) % 60;
    int cent = (int)((timerFase - (int)timerFase) * 100);
    sprintf(bufTimer, "%02d:%02d.%02d", min, seg, cent);
    glColor3f(0.0f,0.0f,0.0f);
    glBegin(GL_QUADS);
        glVertex2f(0.60f,0.83f); glVertex2f(0.99f,0.83f);
        glVertex2f(0.99f,0.97f); glVertex2f(0.60f,0.97f);
    glEnd();
    glColor3f(0.3f,0.3f,0.3f); glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(0.60f,0.83f); glVertex2f(0.99f,0.83f);
        glVertex2f(0.99f,0.97f); glVertex2f(0.60f,0.97f);
    glEnd();
    glColor3f(0.7f,0.9f,0.5f);
    drawText(0.63f, 0.88f, bufTimer, GLUT_BITMAP_HELVETICA_18);

    // moedas HUD
    {
        char bufMoeda[32];
        sprintf(bufMoeda, "Moedas: %d/%d", getMoedasPegas(), getMoedasTotal());
        glColor3f(1.0f, 0.82f, 0.0f);
        drawText(-0.98f, 0.75f, bufMoeda, GLUT_BITMAP_HELVETICA_12);
    }

    // dica
    glColor3f(0.20f,0.22f,0.35f);
    drawText(-0.98f,-0.96f,
             "WASD/SPACE=pular  Q=dash  Q+D/A=horiz  Q+W+D/A=diag  P=pause  M=mudo  ESC=menu",
             GLUT_BITMAP_HELVETICA_12);

    glutSwapBuffers();
}

// ============================================================
// UPDATE GAME
// ============================================================
void updateGame(){
    if(pausado) return;

    timerFase  += 0.016f;
    timerTotal += 0.016f;

    // ==================== COOLDOWNS ====================
    if(clingCooldownL > 0.0f) clingCooldownL -= 0.016f;
    if(clingCooldownR > 0.0f) clingCooldownR -= 0.016f;
    if(!podeDash){
        tempoDash += 0.016f;
        if(tempoDash >= cooldownDash){ podeDash=true; tempoDash=0.0f; }
    }
    if(!podeDashDiag && cooldownDashDiag > 0.0f){
        tempoDashDiag += 0.016f;
        if(tempoDashDiag >= cooldownDashDiag){ podeDashDiag=true; tempoDashDiag=0.0f; }
    } else if(!podeDashDiag){
        podeDashDiag = true;  // cooldown zero = recarrega na hora
    }
    // Reset dash so na transicao queda->chao
    if(noChao && !noChaoAnterior){ podeDash=true; podeDashDiag=true; }
    noChaoAnterior = noChao;

    // ==================== DASH SUAVE ====================
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

    // ==================== WALL CLING ====================
    isClinging = false;
    // Cling so ativa se estiver caindo (velY negativo) — bloqueia subida infinita
    if(velY <= -0.005f){
        if(onRightWall && kD && clingCooldownR <= 0.0f) isClinging = true;
        if(onLeftWall  && kA && clingCooldownL <= 0.0f) isClinging = true;
    }

    // ==================== PULO / WALL JUMP ====================
    if(kSpace && !kSpaceConsumed){
        bool pulou = false;
        if(isClinging){
            float jumpX = 0.0f;
            float jumpY = wallJumpYForce;
            if(onRightWall){ jumpX = -wallJumpXForce; direcao = -1; }
            else if(onLeftWall){ jumpX = wallJumpXForce; direcao = 1; }
            // Desativa cling ANTES de aplicar velocidade — evita velY ser sobrescrito
            isClinging = false;
            velX = jumpX; velY = jumpY;
            kA = false; kD = false;
            if(onRightWall){ clingCooldownR=0.55f; clingCooldownL=0.0f; }
            if(onLeftWall) { clingCooldownL=0.55f; clingCooldownR=0.0f; }
            jumpsRestantes = 0;
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
        if(pulou) kSpaceConsumed = true;
    }

    // ==================== MOVIMENTO LATERAL ====================
    float velAlvo = 0.0f;
    float lerp = (faseAtual==2)?0.18f:0.20f;
    if(kA){ velAlvo = -0.013f; direcao = -1; }
    if(kD){ velAlvo =  0.013f; direcao =  1; }
    if(!isClinging) velX += (velAlvo - velX) * lerp;

    // ==================== GRAVIDADE / CLING ====================
    if(isDashing){
        // sem gravidade durante o dash
    } else if(isClinging){
        velY = clingSlideSpeed;
        velX *= 0.6f;
    } else {
        velY += gravidade;
        if(velY < -0.065f) velY = -0.065f;
    }

    posX += velX;
    posY += velY;

    resolveColisoes();
    checaSaida();
    checaMoedas(posX, posY, PW, PH);
    updateMapa();

    if(checaEspinhoColisao(posX, posY, PW, PH, faseAtual)){
        morrer(); return;
    }
    if(posY < -1.2f){
        morrer(); return;
    }
}

// ============================================================
// INPUT GAME
// ============================================================
void handleGameInput(unsigned char tecla){
    switch(tecla){
        case 'a':case 'A': kA=true;     break;
        case 'd':case 'D': kD=true;     break;
        case 'w':case 'W': kW=true;     break;
        case ' ':          kSpace=true; break;
        case 'q':case 'Q':{
            bool diagDir = kW && kD;
            bool diagEsq = kW && kA;
            bool horizDir= !kW && kD;
            bool horizEsq= !kW && kA;
            bool ehDiag  = diagDir || diagEsq;
            bool podeUsarDash = ehDiag ? podeDashDiag : podeDash;
            if(podeUsarDash){
                if(diagDir){       dashVelX= DASH_FORCA_DIAG; dashVelY= DASH_FORCA_DIAG; }
                else if(diagEsq){  dashVelX=-DASH_FORCA_DIAG; dashVelY= DASH_FORCA_DIAG; }
                else if(horizDir){ dashVelX= DASH_FORCA;      dashVelY=0.0f; }
                else if(horizEsq){ dashVelX=-DASH_FORCA;      dashVelY=0.0f; }
                else {             dashVelX= DASH_FORCA*direcao; dashVelY=0.0f; }
                isDashing = true;
                dashTimer = DASH_DURACAO;
                if(ehDiag){ podeDashDiag=false; tempoDashDiag=0.0f; }
                else       { podeDash=false;     tempoDash=0.0f; }
            }
            break;
        }
        case 'p':case 'P':
            pausado=!pausado; opcaoPause=0;
            break;
        case 'm':case 'M':
            { extern void audioMutar(); audioMutar(); }
            break;
        case 27:
            kA=false; kD=false; kW=false; kSpace=false;
            gameState=0; initGame();
            break;
    }
}

void handleGameInputUp(unsigned char tecla){
    switch(tecla){
        case 'a':case 'A': kA=false; break;
        case 'd':case 'D': kD=false; break;
        case 'w':case 'W': kW=false; break;
        case ' ':
            kSpace=false;
            kSpaceConsumed=false;
            break;
    }
}

// ============================================================
// PAUSE
// ============================================================
void renderPause(){
    renderMapa();
    renderEspinhos(faseAtual);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f,0.0f,0.0f,0.5f);
    glBegin(GL_QUADS);
        glVertex2f(-1,-1); glVertex2f(1,-1);
        glVertex2f(1,1);   glVertex2f(-1,1);
    glEnd();
    glDisable(GL_BLEND);

    glColor3f(1.0f,0.85f,0.0f);
    drawText(-0.12f, 0.40f, "PAUSE", GLUT_BITMAP_TIMES_ROMAN_24);

    if(opcaoPause==0) glColor3f(1.0f,0.85f,0.0f); else glColor3f(0.6f,0.6f,0.6f);
    drawText(-0.20f, 0.10f, "> CONTINUAR", GLUT_BITMAP_HELVETICA_18);

    if(opcaoPause==1) glColor3f(1.0f,0.85f,0.0f); else glColor3f(0.6f,0.6f,0.6f);
    drawText(-0.20f,-0.10f, "  MENU PRINCIPAL", GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.4f,0.4f,0.4f);
    drawText(-0.40f,-0.80f,"W/S = navegar   ENTER = confirmar   ESC = continuar",
             GLUT_BITMAP_HELVETICA_12);

    glutSwapBuffers();
}

void handlePauseInput(unsigned char tecla){
    switch(tecla){
        case 'w':case 'W': opcaoPause=(opcaoPause-1+2)%2; break;
        case 's':case 'S': opcaoPause=(opcaoPause+1)%2;   break;
        case '\r':case '\n':
            if(opcaoPause==0){ pausado=false; }
            else { pausado=false; gameState=0; initGame(); }
            opcaoPause=0;
            break;
        case 27: pausado=false; opcaoPause=0; break;
    }
}

// ============================================================
// VITORIA
// ============================================================
static int opcaoVitoria = 0;

void renderVitoria(){
    glClearColor(0.02f,0.12f,0.02f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if(vitoriaFinal){
        glColor3f(1.0f,0.85f,0.0f);
        drawText(-0.30f, 0.60f, "VOCE VENCEU!", GLUT_BITMAP_TIMES_ROMAN_24);
        glColor3f(0.7f,1.0f,0.7f);
        drawText(-0.40f, 0.40f, "Parabens! Voce completou o jogo!", GLUT_BITMAP_HELVETICA_18);
    } else {
        glColor3f(1.0f,0.85f,0.0f);
        drawText(-0.28f, 0.60f, "FASE CONCLUIDA!", GLUT_BITMAP_TIMES_ROMAN_24);
    }

    char bufTimer[64];
    if(vitoriaFinal){
        int min=(int)(timerTotal/60.0f);
        int seg=(int)(timerTotal)%60;
        int cent=(int)((timerTotal-(int)timerTotal)*100);
        sprintf(bufTimer,"Tempo total: %02d:%02d.%02d",min,seg,cent);
    } else {
        int min=(int)(timerFase/60.0f);
        int seg=(int)(timerFase)%60;
        int cent=(int)((timerFase-(int)timerFase)*100);
        sprintf(bufTimer,"Tempo da fase: %02d:%02d.%02d",min,seg,cent);
    }
    glColor3f(0.6f,0.9f,0.6f);
    drawText(-0.35f, 0.28f, bufTimer, GLUT_BITMAP_HELVETICA_18);

    {
        char bufM[48];
        sprintf(bufM,"Moedas coletadas: %d / %d", getMoedasPegas(), getMoedasTotal());
        glColor3f(1.0f,0.82f,0.0f);
        drawText(-0.30f, 0.08f, bufM, GLUT_BITMAP_HELVETICA_18);
    }

    if(opcaoVitoria==0) glColor3f(1.0f,0.85f,0.0f); else glColor3f(0.6f,0.6f,0.6f);
    const char *op0 = vitoriaFinal ? "> MENU PRINCIPAL" : "> PROXIMA FASE";
    drawText(-0.25f,-0.15f, op0, GLUT_BITMAP_HELVETICA_18);

    if(!vitoriaFinal){
        if(opcaoVitoria==1) glColor3f(1.0f,0.85f,0.0f); else glColor3f(0.6f,0.6f,0.6f);
        drawText(-0.25f,-0.35f, "  MENU PRINCIPAL", GLUT_BITMAP_HELVETICA_18);
    }

    glColor3f(0.3f,0.5f,0.3f);
    drawText(-0.40f,-0.80f,"W/S = navegar   ENTER = confirmar",
             GLUT_BITMAP_HELVETICA_12);

    glutSwapBuffers();
}

void handleVitoriaInput(unsigned char tecla){
    int totalOpcoes = vitoriaFinal ? 1 : 2;
    switch(tecla){
        case 'w':case 'W':
            opcaoVitoria=(opcaoVitoria-1+totalOpcoes)%totalOpcoes; break;
        case 's':case 'S':
            opcaoVitoria=(opcaoVitoria+1)%totalOpcoes; break;
        case '\r':case '\n':
            if(vitoriaFinal || opcaoVitoria==1){
                gameState=0; initGame();
            } else {
                avancarFase();
                posX=-0.75f; posY=-0.70f; velX=0; velY=0; noChao=true;
                respawnX=-0.75f; respawnY=-0.70f;
                Health=3;
                kA=false; kD=false; kW=false; kSpace=false;
                timerFase=0.0f;
                gameState=1;
            }
            opcaoVitoria=0;
            break;
        case 27:
            gameState=0; initGame(); opcaoVitoria=0; break;
    }
}

// ============================================================
// DERROTA
// ============================================================
static int opcaoDerrota = 0;

void renderDerrota(){
    glClearColor(0.12f,0.02f,0.02f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1.0f,0.2f,0.2f);
    drawText(-0.22f, 0.55f, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(0.8f,0.5f,0.5f);
    drawText(-0.30f, 0.28f, "Suas vidas acabaram...", GLUT_BITMAP_HELVETICA_18);

    if(opcaoDerrota==0) glColor3f(1.0f,0.3f,0.3f); else glColor3f(0.6f,0.6f,0.6f);
    drawText(-0.25f, 0.0f, "> TENTAR DE NOVO", GLUT_BITMAP_HELVETICA_18);

    if(opcaoDerrota==1) glColor3f(1.0f,0.3f,0.3f); else glColor3f(0.6f,0.6f,0.6f);
    drawText(-0.25f,-0.20f, "  MENU PRINCIPAL", GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.4f,0.2f,0.2f);
    drawText(-0.40f,-0.80f,"W/S = navegar   ENTER = confirmar",
             GLUT_BITMAP_HELVETICA_12);

    glutSwapBuffers();
}

void handleDerrotaInput(unsigned char tecla){
    switch(tecla){
        case 'w':case 'W': opcaoDerrota=(opcaoDerrota-1+2)%2; break;
        case 's':case 'S': opcaoDerrota=(opcaoDerrota+1)%2;   break;
        case '\r':case '\n':
            if(opcaoDerrota==0){ initGame(); gameState=1; }
            else               { gameState=0; initGame(); }
            opcaoDerrota=0;
            break;
        case 27:
            gameState=0; initGame(); opcaoDerrota=0; break;
    }
}

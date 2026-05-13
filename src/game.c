#define GLUT_DISABLE_ATEXIT_HACK
#include "../include/game.h"
#include "../include/mapa.h"
#include "../include/espinho.h"
#include "../include/anim.h"
#include "../include/screenshot.h"
#include "../include/ranking_score.h"
#include "../include/GL/glut.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern int gameState;

// ============================================================
// ESTADO GLOBAL
// ============================================================
bool pausado       = false;
static int  opcaoPause   = 0;
static bool vitoriaFinal = false;
static float timerFase   = 0.0f;
static float timerTotal  = 0.0f;
int pontos;

static bool  screenshotMessageVisible = false;
static float screenshotMessageTimer   = 0.0f;
static const float SCREENSHOT_MESSAGE_DURATION = 2.5f;

// ============================================================
// RANKING
// Arquivo: assets/ranking.txt
//
// Formato de cada linha:
//   MM:SS.CC,segundos_float,moedas_pegas,moedas_total,pontuacao
// Exemplo:
//   01:23.45,83.45,6,8,812
//
// - tempoStr  : "MM:SS.CC"  ? exibido igual � tela de vit�ria
// - tempo     : float       ? usado para calcular pontuacao
// - moedasPegas / moedasTotal ? exibidos como "X/8 moedas"
// - pontuacao : int         ? usado para ordenacao decrescente
// ============================================================
#define RANKING_PATH  "../assets/ranking.txt"
#define RANKING_MAX   20   /* m�ximo de entradas armazenadas */

typedef struct {
    float tempo;            /* segundos totais                      */
    char  tempoStr[16];     /* "MM:SS.CC"                           */
    int   moedasPegas;      /* moedas coletadas nessa partida       */
    int   moedasTotal;      /* total de moedas do jogo (sempre 8)   */
    int   pontuacao;        /* maior pontuacao = melhor ranking     */
} EntradaRanking;

static EntradaRanking ranking[RANKING_MAX];
static int            rankingCount  = 0;
static bool           rankingAberto = false;  /* sub-tela de ranking ativa */

/* ---------- rankingSalvar ------------------------------------------
   Grava UMA linha no arquivo (append).
   Campos: tempoStr , segundos_float , moedas_pegas , moedas_total , pontuacao
   ------------------------------------------------------------------ */
static void rankingSalvar(float t, int mPegas, int mTotal) {
    FILE *f = fopen(RANKING_PATH, "a");
    if (!f) return;                        /* sem permiss�o de escrita � ignora */

    int min  = (int)(t / 60.0f);
    int seg  = (int)(t) % 60;
    int cent = (int)((t - (int)t) * 100);
    int pontuacao = calcularPontuacaoRanking(t, mPegas);

    /* linha: MM:SS.CC,segundos,moedasPegas,moedasTotal,pontuacao */
    fprintf(f, "%02d:%02d.%02d,%.2f,%d,%d,%d\n",
            min, seg, cent,
            t,
            mPegas,
            mTotal,
            pontuacao);
    fclose(f);
}

/* ---------- cmpRanking --------------------------------------------
   Comparador para qsort � ordem decrescente de pontuacao.
   Em empate, menor tempo fica na frente.
   ------------------------------------------------------------------ */
static int cmpRanking(const void *a, const void *b) {
    const EntradaRanking *ea = (const EntradaRanking *)a;
    const EntradaRanking *eb = (const EntradaRanking *)b;
    if (ea->pontuacao > eb->pontuacao) return -1;
    if (ea->pontuacao < eb->pontuacao) return  1;
    if (ea->tempo < eb->tempo) return -1;
    if (ea->tempo > eb->tempo) return  1;
    return 0;
}

/* ---------- rankingCarregar ----------------------------------------
   L� todas as linhas de ranking.txt, preenche o array e ordena.
   Linhas malformadas s�o silenciosamente ignoradas.
   ------------------------------------------------------------------ */
static void rankingCarregar(void) {
    rankingCount = 0;
    FILE *f = fopen(RANKING_PATH, "r");
    if (!f) return;                        /* arquivo ainda n�o existe � ok */

    char linha[96];
    while (fgets(linha, sizeof(linha), f) && rankingCount < RANKING_MAX) {

        /* remove \n e \r finais */
        int len = (int)strlen(linha);
        while (len > 0 && (linha[len-1] == '\n' || linha[len-1] == '\r'))
            linha[--len] = '\0';
        if (len == 0) continue;

        /* --- parse manual: campo1,campo2,campo3,campo4,campo5 opcional --- */

        /* campo 1: tempoStr  (at� a primeira v�rgula) */
        char *p1 = strchr(linha, ',');
        if (!p1) continue;
        *p1 = '\0';                        /* termina tempoStr */

        /* campo 2: segundos float */
        char *p2 = strchr(p1 + 1, ',');
        if (!p2) continue;
        *p2 = '\0';

        /* campo 3: moedasPegas */
        char *p3 = strchr(p2 + 1, ',');
        if (!p3) continue;
        *p3 = '\0';

        /* campo 4: moedasTotal; campo 5 opcional: pontuacao */
        char *campoMoedasTotal = p3 + 1;
        char *p4 = strchr(p3 + 1, ',');
        if (p4) {
            *p4 = '\0';
        }

        /* preenche a entrada */
        strncpy(ranking[rankingCount].tempoStr, linha, 15);
        ranking[rankingCount].tempoStr[15]   = '\0';
        ranking[rankingCount].tempo          = (float)atof(p1  + 1);
        ranking[rankingCount].moedasPegas    = atoi(p2  + 1);
        ranking[rankingCount].moedasTotal    = atoi(campoMoedasTotal);
        ranking[rankingCount].pontuacao       = calcularPontuacaoRanking(ranking[rankingCount].tempo,
                                                                           ranking[rankingCount].moedasPegas);

        rankingCount++;
    }
    fclose(f);

    /* ordena da maior para a menor pontuacao */
    qsort(ranking, rankingCount, sizeof(EntradaRanking), cmpRanking);
}

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

static bool noChao      = true;
static bool onLeftWall  = false;
static bool onRightWall = false;
static bool isClinging  = false;

static float respawnX = -0.75f;
static float respawnY = -0.70f;

static float jumpForce      = 0.038f;
static int   jumpsRestantes = 1;
static int   maxJumps       = 2;

static bool  podeDash        = true;
static bool  podeDashDiag    = true;
static float tempoDash       = 0.0f;
static float tempoDashDiag   = 0.0f;
static float cooldownDash    = 3.0f;
static float cooldownDashDiag= 0.0f;
static bool  isDashing       = false;
static float dashVelX        = 0.0f;
static float dashVelY        = 0.0f;
static float dashTimer       = 0.0f;
static const float DASH_DURACAO    = 0.18f;
static const float DASH_FORCA      = 0.045f;
static const float DASH_FORCA_DIAG = 0.032f;

static bool kA=false, kD=false, kW=false, kSpace=false;
static bool kSpaceConsumed  = false;
static bool noChaoAnterior  = true;
static float clingCooldownL = 0.0f;
static float clingCooldownR = 0.0f;
static float clingSlideSpeed = 0.0f;
static float wallJumpXForce  = 0.045f;
static float wallJumpYForce  = 0.028f;

// ============================================================
// COLISAO
// ============================================================
#define PW 0.055f
#define PH 0.090f

static bool colideCom(Bloco b){
    return posX+PW>b.x1 && posX-PW<b.x2 &&
           posY+PH>b.y1 && posY<b.y2;
}

#define WALL_SENSOR 0.012f
static void detectaParedes(){
    int i;
    onLeftWall  = false;
    onRightWall = false;
    for(i = 0; i < numBlocos; i++){
        Bloco b = blocos[i];
        bool alturaOk = (posY+PH > b.y1+0.01f) && (posY < b.y2-0.01f);
        if(!alturaOk) continue;
        if(b.x1 >= posX+PW-0.002f && b.x1 <= posX+PW+WALL_SENSOR) onRightWall = true;
        if(b.x2 <= posX-PW+0.002f && b.x2 >= posX-PW-WALL_SENSOR) onLeftWall  = true;
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
        float oC = (posY+PH) - b.y1;
        float oD = (posX+PW) - b.x1;
        float oE = b.x2 - (posX-PW);
        float mH = oB < oC ? oB : oC;
        float mV = oD < oE ? oD : oE;
        if(mH < mV){
            if(oB < oC){ posY=b.y2; velY=0; noChao=true; jumpsRestantes=maxJumps; podeDash=true; }
            else        { posY=b.y1-PH; velY=0; }
        } else {
            if(oD < oE) posX -= oD; else posX += oE;
            velX = 0;
        }
    }
    if(noChao) velX *= (1.0f - atrito);
    detectaParedes();
}

static void morrer(){
    Health--;
    if(Health <= 0){ gameState=3; return; }
    posX=respawnX; posY=respawnY;
    velX=0; velY=0;
    noChao=true;
    isDashing=false;
    jumpsRestantes=maxJumps;
}

static void checaSaida(){
    int idx = indiceSaida();
    if(idx < 0 || idx >= numBlocos) return;
    Bloco s = blocos[idx];
    if(posX+PW>s.x1 && posX-PW<s.x2 &&
       posY>=s.y2-0.08f && posY<=s.y2+0.08f && noChao){
        kA=false; kD=false; kW=false; kSpace=false;
        if(faseAtual < 3){
            vitoriaFinal = false;
            gameState = 2;
        } else {
            /* jogo completo: salva tempo + moedas no ranking.txt */
            rankingSalvar(timerTotal, getMoedasPegas(), getMoedasTotal());
            vitoriaFinal = true;
            gameState = 2;
        }
    }
}

static void drawText(float x, float y, const char *txt, void *fonte){
    glRasterPos2f(x, y);
    while(*txt){ glutBitmapCharacter(fonte, *txt); txt++; }
}
static void renderScreenshotMessage() {
    if (!screenshotMessageVisible) return;

    /*
       A caixa e desenhada perto do canto inferior esquerdo.
       Ela fica um pouco acima da dica de controles para nao cobrir o texto do HUD.
    */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.02f, 0.02f, 0.06f, 0.78f);
    glBegin(GL_QUADS);
        glVertex2f(-0.98f, -0.82f);
        glVertex2f(-0.50f, -0.82f);
        glVertex2f(-0.50f, -0.70f);
        glVertex2f(-0.98f, -0.70f);
    glEnd();
    glDisable(GL_BLEND);

    glColor3f(0.92f, 0.95f, 1.0f);
    drawText(-0.94f, -0.775f, "Screenshot salva!", GLUT_BITMAP_HELVETICA_12);
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
    rankingAberto=false;
    timerFase=0.0f; timerTotal=0.0f;
    screenshotMessageVisible=false;
    screenshotMessageTimer=0.0f;
    initMapa();
    animInit();
}

// ============================================================
// RENDER GAME (inalterado do legado)
// ============================================================
void renderGame(){
    renderMapa();
    renderEspinhos(faseAtual);

    animDraw(posX, posY, (PW-0.025f)*3.0f, PH);

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

    if(isClinging){
        glColor3f(0.4f, 0.8f, 1.0f);
        drawText(-0.98f, 0.70f, "CLING", GLUT_BITMAP_HELVETICA_12);
    }

    if(!noChao && jumpsRestantes > 0){
        glColor3f(0.8f, 0.6f, 1.0f);
        drawText(-0.98f, 0.64f, "2x JUMP", GLUT_BITMAP_HELVETICA_12);
    }

    const char *nomes[4]={"Mapa 1 - Floresta","Mapa 2 - Cidade","Mapa 3 - Geleiras","Mapa Final - Pico"};
    glColor3f(0.5f,0.6f,0.9f);
    drawText(0.25f, 0.92f, nomes[faseAtual], GLUT_BITMAP_HELVETICA_12);

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

    {
        char bufMoeda[32];
        sprintf(bufMoeda, "Moedas: %d/%d", getMoedasPegas(), getMoedasTotal());
        glColor3f(1.0f, 0.82f, 0.0f);
        drawText(-0.98f, 0.75f, bufMoeda, GLUT_BITMAP_HELVETICA_12);
    }

    glColor3f(0.20f,0.22f,0.35f);
    drawText(-0.98f,-0.96f,
             "WASD/SPACE=pular  Q=dash  Q+D/A=horiz  Q+W+D/A=diag  P=pause  O=screenshot  M=mudo  ESC=menu",
             GLUT_BITMAP_HELVETICA_12);

    renderScreenshotMessage();

    glutSwapBuffers();
}

// ============================================================
// UPDATE GAME (inalterado do legado)
// ============================================================
void updateGame(){
    if(pausado) return;

    timerFase  += 0.016f;
    timerTotal += 0.016f;

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
        podeDashDiag = true;
    }
    if(noChao && !noChaoAnterior){ podeDash=true; podeDashDiag=true; }
    noChaoAnterior = noChao;

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
    if(velY <= -0.005f){
        if(onRightWall && kD && clingCooldownR <= 0.0f) isClinging = true;
        if(onLeftWall  && kA && clingCooldownL <= 0.0f) isClinging = true;
    }

    if(kSpace && !kSpaceConsumed){
        bool pulou = false;
        if(isClinging){
            float jumpX = 0.0f;
            float jumpY = wallJumpYForce;
            if(onRightWall){ jumpX = -wallJumpXForce; direcao = -1; }
            else if(onLeftWall){ jumpX = wallJumpXForce; direcao = 1; }
            isClinging = false;
            velX = jumpX; velY = jumpY;
            kA = false; kD = false;
            if(onRightWall){ clingCooldownR=0.55f; clingCooldownL=0.0f; }
            if(onLeftWall) { clingCooldownL=0.55f; clingCooldownR=0.0f; }
            jumpsRestantes = 0;
            pulou = true;
        } else if(noChao){
            velY = jumpForce;
            jumpsRestantes = maxJumps - 1;
            noChao = false;
            pulou = true;
        } else if(jumpsRestantes > 0){
            velY = jumpForce * 0.9f;
            jumpsRestantes--;
            pulou = true;
        }
        if(pulou) kSpaceConsumed = true;
    }

    float velAlvo = 0.0f;
    float lerp = (faseAtual==2)?0.18f:0.20f;
    if(kA){ velAlvo = -0.013f; direcao = -1; }
    if(kD){ velAlvo =  0.013f; direcao =  1; }
    if(!isClinging) velX += (velAlvo - velX) * lerp;

    if(isDashing){
        /* sem gravidade durante o dash */
    } else if(isClinging){
        velY = clingSlideSpeed;
        velX *= 0.6f;
    } else {
        velY += gravidade;
        if(velY < -0.065f) velY = -0.065f;
    }

    /* CLAMP: impede sair dos limites do mundo mesmo com dash */
    posX += velX;
    posY += velY;
    if(posX - PW < -1.00f){ posX = -1.00f + PW; velX = 0; }
    if(posX + PW >  1.00f){ posX =  1.00f - PW; velX = 0; }
    if(posY + PH >  1.00f){ posY =  1.00f - PH; velY = 0; }

    resolveColisoes();
    checaSaida();
    checaMoedas(posX, posY, PW, PH);
    updateMapa();
    /*
       Controla o tempo do aviso de screenshot.
       A cada frame o timer diminui; quando chega em zero, a mensagem some sozinha.
    */
    if(screenshotMessageVisible){
        screenshotMessageTimer-=0.016f;
        if(screenshotMessageTimer<=0.0f){
            screenshotMessageVisible=false;
            screenshotMessageTimer=0.0f;
        }
    }
    if(checaEspinhoColisao(posX, posY, PW, PH, faseAtual)){
        morrer(); return;
    }
    if(posY < -1.2f){
        morrer(); return;
    }

    animUpdate(0.016f, velX, velY, (int)noChao,
               (int)podeDash, tempoDash, direcao);
}

// ============================================================
// INPUT GAME (inalterado do legado)
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
        case 'o':case 'O':
            if(salvarScreenshot()){
                screenshotMessageVisible=true;
                screenshotMessageTimer=SCREENSHOT_MESSAGE_DURATION;
            }
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
// PAUSE (inalterado)
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
// SUB-TELA DE RANKING
// Exibe as melhores pontuacoes em ordem decrescente (maior = melhor).
// Cada linha mostra: posicao, tempo (MM:SS.CC), moedas e pontuacao.
// ============================================================
static void renderRanking(void) {
    glClearColor(0.02f, 0.05f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1.0f, 0.85f, 0.0f);
    drawText(-0.10f, 0.84f, "RANKING", GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(0.25f, 0.25f, 0.55f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        glVertex2f(-0.55f, 0.73f);
        glVertex2f( 0.55f, 0.73f);
    glEnd();

    if (rankingCount == 0) {
        glColor3f(0.6f, 0.6f, 0.6f);
        drawText(-0.33f, 0.20f, "Nenhum tempo registrado ainda.",
                 GLUT_BITMAP_HELVETICA_18);
    } else {
        const float xPosicao  = -0.50f;
        const float xTempo    = -0.34f;
        const float xMoedas   = -0.08f;
        const float xPontos   =  0.20f;
        float y = 0.59f;
        int exibir = (rankingCount < 10) ? rankingCount : 10;
        int i;

        glColor3f(0.5f, 0.8f, 1.0f);
        drawText(xPosicao, y, "#", GLUT_BITMAP_HELVETICA_18);
        drawText(xTempo,   y, "Tempo", GLUT_BITMAP_HELVETICA_18);
        drawText(xMoedas,  y, "Moedas", GLUT_BITMAP_HELVETICA_18);
        drawText(xPontos,  y, "Pontuacao", GLUT_BITMAP_HELVETICA_18);

        glColor3f(0.12f, 0.18f, 0.38f);
        glBegin(GL_LINES);
            glVertex2f(-0.55f, 0.53f);
            glVertex2f( 0.55f, 0.53f);
        glEnd();

        y = 0.43f;
        for (i = 0; i < exibir; i++) {
            char bufPos[8];
            char bufMoedas[16];
            char bufPontos[16];

            if      (i == 0) glColor3f(1.00f, 0.85f, 0.00f);
            else if (i == 1) glColor3f(0.80f, 0.80f, 0.80f);
            else if (i == 2) glColor3f(0.80f, 0.50f, 0.20f);
            else             glColor3f(0.75f, 0.75f, 0.75f);

            sprintf(bufPos, "%2d.", i + 1);
            sprintf(bufMoedas, "%d/%d", ranking[i].moedasPegas, ranking[i].moedasTotal);
            sprintf(bufPontos, "%d", ranking[i].pontuacao);

            drawText(xPosicao, y, bufPos, GLUT_BITMAP_HELVETICA_18);
            drawText(xTempo,   y, ranking[i].tempoStr, GLUT_BITMAP_HELVETICA_18);
            drawText(xMoedas,  y, bufMoedas, GLUT_BITMAP_HELVETICA_18);
            drawText(xPontos,  y, bufPontos, GLUT_BITMAP_HELVETICA_18);

            y -= 0.095f;
        }
    }

    glColor3f(0.4f, 0.6f, 1.0f);
    drawText(-0.18f, -0.84f, "ESC / ENTER = voltar",
             GLUT_BITMAP_HELVETICA_18);

    glutSwapBuffers();
}
// ============================================================
// VITORIA � 3 op��es quando vitoriaFinal == true:
//   0: MENU PRINCIPAL
//   1: VER RANKING
//   2: JOGAR NOVAMENTE
// ============================================================
static int opcaoVitoria = 0;

/* Quantas op��es existem dependendo do contexto */
static int totalOpcoesVitoria(void) {
    if (vitoriaFinal) return 3;   /* Menu | Ranking | Jogar de novo */
    return 2;                     /* Proxima fase  | Menu           */
}

void renderVitoria(void) {
    /* sub-tela de ranking ativa: delega para ela */
    if (rankingAberto) { renderRanking(); return; }

    glClearColor(0.02f, 0.12f, 0.02f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (vitoriaFinal) {
        glColor3f(1.0f, 0.85f, 0.0f);
        drawText(-0.18f, 0.78f, "VOCE VENCEU!", GLUT_BITMAP_TIMES_ROMAN_24);
        glColor3f(0.7f, 1.0f, 0.7f);
        drawText(-0.33f, 0.61f, "Parabens! Voce completou o jogo!", GLUT_BITMAP_HELVETICA_18);
    } else {
        glColor3f(1.0f, 0.85f, 0.0f);
        drawText(-0.22f, 0.78f, "FASE CONCLUIDA!", GLUT_BITMAP_TIMES_ROMAN_24);
    }

    /* Linha divisoria entre titulo e resultados. */
    glColor3f(0.12f, 0.32f, 0.12f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        glVertex2f(-0.45f, 0.52f);
        glVertex2f( 0.45f, 0.52f);
    glEnd();

    /* --- tempo --- exibido identico ao HUD: MM:SS.CC --- */
    {
        char bufTimer[64];
        float t = vitoriaFinal ? timerTotal : timerFase;
        int min  = (int)(t / 60.0f);
        int seg  = (int)(t) % 60;
        int cent = (int)((t - (int)t) * 100);
        const char *label = vitoriaFinal ? "Tempo total" : "Tempo da fase";
        sprintf(bufTimer, "%s: %02d:%02d.%02d", label, min, seg, cent);
        glColor3f(0.6f, 0.9f, 0.6f);
        drawText(-0.28f, 0.35f, bufTimer, GLUT_BITMAP_HELVETICA_18);
    }

    /* --- moedas e pontuacao --- */
    {
        char bufM[48];
        sprintf(bufM, "Moedas: %d/%d", getMoedasPegas(), getMoedasTotal());
        glColor3f(1.0f, 0.82f, 0.0f);
        drawText(-0.28f, 0.21f, bufM, GLUT_BITMAP_HELVETICA_18);
    }

    if (vitoriaFinal) {
        char bufP[48];
        sprintf(bufP, "Pontuacao: %d",
                calcularPontuacaoRanking(timerTotal, getMoedasPegas()));
        glColor3f(0.7f, 0.9f, 1.0f);
        drawText(-0.28f, 0.07f, bufP, GLUT_BITMAP_HELVETICA_18);
    }

    /* Linha divisoria entre resultados e menu. */
    glColor3f(0.12f, 0.32f, 0.12f);
    glBegin(GL_LINES);
        glVertex2f(-0.35f, -0.06f);
        glVertex2f( 0.35f, -0.06f);
    glEnd();

    /* ---- OPCOES ---- */
    if (opcaoVitoria == 0) glColor3f(1.0f, 0.85f, 0.0f); else glColor3f(0.6f, 0.6f, 0.6f);
    drawText(-0.23f, -0.20f,
             vitoriaFinal
                ? (opcaoVitoria == 0 ? "> MENU PRINCIPAL" : "  MENU PRINCIPAL")
                : (opcaoVitoria == 0 ? "> PROXIMA FASE" : "  PROXIMA FASE"),
             GLUT_BITMAP_HELVETICA_18);

    if (vitoriaFinal) {
        if (opcaoVitoria == 1) glColor3f(1.0f, 0.85f, 0.0f); else glColor3f(0.6f, 0.6f, 0.6f);
        drawText(-0.23f, -0.34f,
                 opcaoVitoria == 1 ? "> VER RANKING" : "  VER RANKING",
                 GLUT_BITMAP_HELVETICA_18);

        if (opcaoVitoria == 2) glColor3f(1.0f, 0.85f, 0.0f); else glColor3f(0.6f, 0.6f, 0.6f);
        drawText(-0.23f, -0.48f,
                 opcaoVitoria == 2 ? "> JOGAR NOVAMENTE" : "  JOGAR NOVAMENTE",
                 GLUT_BITMAP_HELVETICA_18);
    } else {
        if (opcaoVitoria == 1) glColor3f(1.0f, 0.85f, 0.0f); else glColor3f(0.6f, 0.6f, 0.6f);
        drawText(-0.23f, -0.34f,
                 opcaoVitoria == 1 ? "> MENU PRINCIPAL" : "  MENU PRINCIPAL",
                 GLUT_BITMAP_HELVETICA_18);
    }

    glColor3f(0.3f, 0.5f, 0.3f);
    drawText(-0.30f, -0.84f, "W/S = navegar   ENTER = confirmar",
             GLUT_BITMAP_HELVETICA_12);

    glutSwapBuffers();
}
void handleVitoriaInput(unsigned char tecla) {
    /* dentro do ranking: qualquer tecla confirmada volta */
    if (rankingAberto) {
        if (tecla == 27 || tecla == '\r' || tecla == '\n')
            rankingAberto = false;
        glutPostRedisplay();
        return;
    }

    int total = totalOpcoesVitoria();
    switch (tecla) {
        case 'w': case 'W':
            opcaoVitoria = (opcaoVitoria - 1 + total) % total;
            break;
        case 's': case 'S':
            opcaoVitoria = (opcaoVitoria + 1) % total;
            break;

        case '\r': case '\n':
            if (vitoriaFinal) {
                if (opcaoVitoria == 0) {
                    /* menu principal */
                    gameState = 0; initGame(); opcaoVitoria = 0;
                } else if (opcaoVitoria == 1) {
                    /* abre sub-tela de ranking: l� e ordena o arquivo */
                    rankingCarregar();
                    rankingAberto = true;
                } else {
                    /* jogar novamente do in�cio */
                    initGame(); gameState = 1; opcaoVitoria = 0;
                }
            } else {
                /* n�o � vit�ria final */
                if (opcaoVitoria == 0) {
                    /* pr�xima fase */
                    avancarFase();
                    posX = -0.75f; posY = -0.70f; velX = 0; velY = 0; noChao = true;
                    respawnX = -0.75f; respawnY = -0.70f;
                    kA = false; kD = false; kW = false; kSpace = false;
                    timerFase = 0.0f;
                    gameState = 1;
                } else {
                    gameState = 0; initGame();
                }
                opcaoVitoria = 0;
            }
            break;

        case 27:
            gameState = 0; initGame();
            opcaoVitoria = 0;
            break;
    }
    glutPostRedisplay();
}

// ============================================================
// DERROTA (inalterado do legado)
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


#define GLUT_DISABLE_ATEXIT_HACK
#include "../include/galeria_screenshot.h"
#include "../include/GL/glut.h"
#include "../include/stb_image.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREENSHOT_DIR "..\\screenshots"
#define MAX_SCREENSHOTS 100

typedef struct {
    unsigned int textura;
    int largura;
    int altura;
    char nome[260];
} ScreenshotGaleria;

static ScreenshotGaleria screenshots[MAX_SCREENSHOTS];
static int totalScreenshots = 0;
static int screenshotAtual = 0;
static int galeriaCarregada = 0;

static void desenhaTextoGaleria(float x, float y, const char *txt, void *fonte) {
    glRasterPos2f(x, y);
    while (*txt) {
        glutBitmapCharacter(fonte, *txt);
        txt++;
    }
}

static int extensaoImagemValida(const char *nomeArquivo) {
    const char *ext = strrchr(nomeArquivo, '.');
    if (!ext) return 0;

    return lstrcmpiA(ext, ".bmp") == 0 ||
           lstrcmpiA(ext, ".png") == 0 ||
           lstrcmpiA(ext, ".jpg") == 0 ||
           lstrcmpiA(ext, ".jpeg") == 0;
}

static void nomeCurto(const char *caminho, char *destino, size_t tamanho) {
    const char *barra1 = strrchr(caminho, '\\');
    const char *barra2 = strrchr(caminho, '/');
    const char *inicio = barra1;
    size_t i;

    if (!inicio || (barra2 && barra2 > barra1)) inicio = barra2;
    if (inicio) inicio++;
    else inicio = caminho;

    for (i = 0; i + 1 < tamanho && inicio[i] != '\0'; i++) {
        destino[i] = inicio[i];
    }
    destino[i] = '\0';
}

static int carregarImagemComoTextura(const char *caminho) {
    int largura, altura, canais;
    unsigned char *dados;
    unsigned int textura;

    if (totalScreenshots >= MAX_SCREENSHOTS) return 0;

    /*
       stb_image carrega a imagem salva no disco.
       O flip vertical deixa a textura na orientacao correta para o OpenGL.
    */
    stbi_set_flip_vertically_on_load(1);
    dados = stbi_load(caminho, &largura, &altura, &canais, 4);
    if (!dados) {
        printf("[GALERIA] Nao foi possivel carregar: %s\n", caminho);
        return 0;
    }

    /*
       Aqui a imagem carregada vira uma textura OpenGL.
       Depois disso ela pode ser desenhada como um retangulo texturizado.
    */
    glGenTextures(1, &textura);
    glBindTexture(GL_TEXTURE_2D, textura);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, largura, altura, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, dados);
    stbi_image_free(dados);

    screenshots[totalScreenshots].textura = textura;
    screenshots[totalScreenshots].largura = largura;
    screenshots[totalScreenshots].altura = altura;
    nomeCurto(caminho, screenshots[totalScreenshots].nome,
              sizeof(screenshots[totalScreenshots].nome));
    totalScreenshots++;
    return 1;
}

void liberarTexturasScreenshots(void) {
    int i;

    for (i = 0; i < totalScreenshots; i++) {
        if (screenshots[i].textura != 0) {
            glDeleteTextures(1, &screenshots[i].textura);
            screenshots[i].textura = 0;
        }
    }

    totalScreenshots = 0;
    screenshotAtual = 0;
    galeriaCarregada = 0;
}

void carregarScreenshots(void) {
    WIN32_FIND_DATAA dadosBusca;
    HANDLE busca;
    char padraoBusca[512];

    liberarTexturasScreenshots();

    /*
       Acessa a pasta screenshots. Se ela nao existir, a busca falha e
       a tela da galeria mostra "Nenhuma screenshot encontrada.".
    */
    snprintf(padraoBusca, sizeof(padraoBusca), SCREENSHOT_DIR "\\*.*");
    busca = FindFirstFileA(padraoBusca, &dadosBusca);

    if (busca == INVALID_HANDLE_VALUE) {
        galeriaCarregada = 1;
        return;
    }

    do {
        if (!(dadosBusca.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            extensaoImagemValida(dadosBusca.cFileName)) {
            char caminhoCompleto[512];
            snprintf(caminhoCompleto, sizeof(caminhoCompleto),
                     SCREENSHOT_DIR "\\%s", dadosBusca.cFileName);
            carregarImagemComoTextura(caminhoCompleto);
        }
    } while (FindNextFileA(busca, &dadosBusca));

    FindClose(busca);
    galeriaCarregada = 1;
}

void atualizarMenuScreenshots(void) {
    if (!galeriaCarregada) {
        carregarScreenshots();
    }
}

void galeriaScreenshotAnterior(void) {
    /*
       Navegacao para a screenshot anterior.
       Se estiver na primeira, volta para a ultima para facilitar o uso.
    */
    if (totalScreenshots <= 0) return;
    screenshotAtual = (screenshotAtual - 1 + totalScreenshots) % totalScreenshots;
}

void galeriaScreenshotProxima(void) {
    /*
       Navegacao para a proxima screenshot.
       Se estiver na ultima, volta para a primeira.
    */
    if (totalScreenshots <= 0) return;
    screenshotAtual = (screenshotAtual + 1) % totalScreenshots;
}

static void desenharScreenshotAtual(void) {
    ScreenshotGaleria *shot = &screenshots[screenshotAtual];
    float maxW = 1.55f;
    float maxH = 1.05f;
    float proporcao = (float)shot->largura / (float)shot->altura;
    float w = maxW;
    float h = w / proporcao;
    float x1, x2, y1, y2;

    if (h > maxH) {
        h = maxH;
        w = h * proporcao;
    }

    x1 = -w * 0.5f;
    x2 =  w * 0.5f;
    y1 = -0.15f - h * 0.5f;
    y2 = -0.15f + h * 0.5f;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, shot->textura);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(x1, y1);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(x2, y1);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(x2, y2);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(x1, y2);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    glColor3f(0.15f, 0.18f, 0.32f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(x1, y1);
        glVertex2f(x2, y1);
        glVertex2f(x2, y2);
        glVertex2f(x1, y2);
    glEnd();
}

void desenharGaleriaScreenshots(void) {
    char contador[80];

    glClearColor(0.04f, 0.04f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1.0f, 0.85f, 0.0f);
    desenhaTextoGaleria(-0.28f, 0.86f, "SCREENSHOTS", GLUT_BITMAP_TIMES_ROMAN_24);

    if (totalScreenshots <= 0) {
        glColor3f(0.9f, 0.9f, 0.9f);
        desenhaTextoGaleria(-0.38f, 0.05f, "Nenhuma screenshot encontrada.",
                            GLUT_BITMAP_HELVETICA_18);
        glColor3f(0.4f, 0.6f, 1.0f);
        desenhaTextoGaleria(-0.24f, -0.82f, "ESC = voltar", GLUT_BITMAP_HELVETICA_18);
        glutSwapBuffers();
        return;
    }

    desenharScreenshotAtual();

    glColor3f(0.9f, 0.9f, 0.9f);
    desenhaTextoGaleria(-0.38f, 0.66f, screenshots[screenshotAtual].nome,
                        GLUT_BITMAP_HELVETICA_12);

    snprintf(contador, sizeof(contador), "%d / %d", screenshotAtual + 1, totalScreenshots);
    glColor3f(1.0f, 0.85f, 0.0f);
    desenhaTextoGaleria(-0.04f, -0.82f, contador, GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.4f, 0.6f, 1.0f);
    desenhaTextoGaleria(-0.82f, -0.90f,
                        "A/Seta esquerda = anterior   D/Seta direita = proxima   ESC = voltar",
                        GLUT_BITMAP_HELVETICA_12);

    glutSwapBuffers();
}

#define GLUT_DISABLE_ATEXIT_HACK
#include "../include/screenshot.h"
#include "../include/GL/glut.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef GL_BGR_EXT
#define GL_BGR_EXT 0x80E0
#endif

#define SCREENSHOT_DIR "..\\screenshots"

static void escreveU16(FILE *arquivo, unsigned short valor) {
    fputc(valor & 0xFF, arquivo);
    fputc((valor >> 8) & 0xFF, arquivo);
}

static void escreveU32(FILE *arquivo, unsigned int valor) {
    fputc(valor & 0xFF, arquivo);
    fputc((valor >> 8) & 0xFF, arquivo);
    fputc((valor >> 16) & 0xFF, arquivo);
    fputc((valor >> 24) & 0xFF, arquivo);
}

static void escreveI32(FILE *arquivo, int valor) {
    escreveU32(arquivo, (unsigned int)valor);
}

static int garantePastaScreenshots(void) {
    DWORD atributos = GetFileAttributesA(SCREENSHOT_DIR);

    /* if: se a pasta screenshots ja existe, podemos salvar dentro dela. */
    if (atributos != INVALID_FILE_ATTRIBUTES &&
        (atributos & FILE_ATTRIBUTE_DIRECTORY)) {
        return 1;
    }

    /* else: se nao existe, criamos a pasta antes de salvar a imagem. */
    else {
        if (CreateDirectoryA(SCREENSHOT_DIR, NULL)) {
            return 1;
        }
        printf("[SCREENSHOT] Nao foi possivel criar a pasta %s\n", SCREENSHOT_DIR);
        return 0;
    }
}

static void criaNomeUnico(char *destino, size_t tamanho) {
    time_t agora = time(NULL);
    struct tm *info = localtime(&agora);
    char base[220];
    int contador = 1;

    if (info) {
        snprintf(base, sizeof(base),
                 "screenshot_%04d-%02d-%02d_%02d-%02d-%02d",
                 info->tm_year + 1900, info->tm_mon + 1, info->tm_mday,
                 info->tm_hour, info->tm_min, info->tm_sec);
    } else {
        snprintf(base, sizeof(base), "screenshot_%lu", (unsigned long)agora);
    }

    /*
       O horario no nome ja evita quase todas as repeticoes.
       O contador abaixo garante que apertar O duas vezes no mesmo segundo
       nao sobrescreva uma screenshot antiga.
    */
    snprintf(destino, tamanho, SCREENSHOT_DIR "\\%s.bmp", base);
    while (GetFileAttributesA(destino) != INVALID_FILE_ATTRIBUTES) {
        snprintf(destino, tamanho, SCREENSHOT_DIR "\\%s_%d.bmp", base, contador);
        contador++;
    }
}

static int salvaBmp24(const char *nomeArquivo,
                      const unsigned char *pixelsTopDown,
                      int largura,
                      int altura) {
    FILE *arquivo = fopen(nomeArquivo, "wb");
    int bytesPorPixel = 3;
    int bytesLinha = largura * bytesPorPixel;
    int padding = (4 - (bytesLinha % 4)) % 4;
    int linhaComPadding = bytesLinha + padding;
    int tamanhoImagem = linhaComPadding * altura;
    int tamanhoArquivo = 14 + 40 + tamanhoImagem;
    unsigned char zeros[3] = {0, 0, 0};
    int y;

    if (!arquivo) {
        printf("[SCREENSHOT] Nao foi possivel criar o arquivo %s\n", nomeArquivo);
        return 0;
    }

    /* Salva em BMP 24 bits porque e simples, funciona no Windows e nao exige biblioteca extra. */
    fputc('B', arquivo);
    fputc('M', arquivo);
    escreveU32(arquivo, (unsigned int)tamanhoArquivo);
    escreveU16(arquivo, 0);
    escreveU16(arquivo, 0);
    escreveU32(arquivo, 14 + 40);

    escreveU32(arquivo, 40);
    escreveI32(arquivo, largura);
    escreveI32(arquivo, -altura);
    escreveU16(arquivo, 1);
    escreveU16(arquivo, 24);
    escreveU32(arquivo, 0);
    escreveU32(arquivo, (unsigned int)tamanhoImagem);
    escreveI32(arquivo, 2835);
    escreveI32(arquivo, 2835);
    escreveU32(arquivo, 0);
    escreveU32(arquivo, 0);

    for (y = 0; y < altura; y++) {
        fwrite(pixelsTopDown + y * bytesLinha, 1, bytesLinha, arquivo);
        fwrite(zeros, 1, padding, arquivo);
    }

    fclose(arquivo);
    return 1;
}

int salvarScreenshot(void) {
    int largura = glutGet(GLUT_WINDOW_WIDTH);
    int altura = glutGet(GLUT_WINDOW_HEIGHT);
    int bytesLinha = largura * 3;
    int y;
    char nomeArquivo[260];
    unsigned char *pixelsOpenGL;
    unsigned char *pixelsInvertidos;

    if (largura <= 0 || altura <= 0) {
        printf("[SCREENSHOT] Janela invalida para captura.\n");
        return 0;
    }

    if (!garantePastaScreenshots()) {
        return 0;
    }

    pixelsOpenGL = (unsigned char *)malloc(bytesLinha * altura);
    pixelsInvertidos = (unsigned char *)malloc(bytesLinha * altura);

    if (!pixelsOpenGL || !pixelsInvertidos) {
        printf("[SCREENSHOT] Memoria insuficiente para capturar a tela.\n");
        free(pixelsOpenGL);
        free(pixelsInvertidos);
        return 0;
    }

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_FRONT);

    /*
       glReadPixels captura os pixels da janela atual do OpenGL.
       Usamos GL_BGR_EXT para gravar direto na ordem de cores esperada pelo BMP.
    */
    glReadPixels(0, 0, largura, altura, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixelsOpenGL);

    /*
       O OpenGL le a imagem de baixo para cima. Sem essa inversao vertical,
       o print pode ficar de cabeca para baixo em formatos comuns de imagem.
    */
    for (y = 0; y < altura; y++) {
        unsigned char *linhaDestino = pixelsInvertidos + y * bytesLinha;
        unsigned char *linhaOrigem = pixelsOpenGL + (altura - 1 - y) * bytesLinha;
        memcpy(linhaDestino, linhaOrigem, bytesLinha);
    }

    criaNomeUnico(nomeArquivo, sizeof(nomeArquivo));

    /* A imagem e salva em BMP dentro da pasta screenshots. */
    if (salvaBmp24(nomeArquivo, pixelsInvertidos, largura, altura)) {
        printf("[SCREENSHOT] Screenshot salva em: %s\n", nomeArquivo);
        free(pixelsOpenGL);
        free(pixelsInvertidos);
        return 1;
    }

    free(pixelsOpenGL);
    free(pixelsInvertidos);
    return 0;
}


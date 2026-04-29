#ifndef AUDIO_H
#define AUDIO_H

// ============================================================
// SISTEMA DE AUDIO - ALTHEA
// Biblioteca: BASS (un4seen.com/bass.html)
//
// TUTORIAL DE USO:
//
// 1. MUSICAS (streams - arquivos grandes MP3/OGG):
//    audioTocarMusica(MUSICA_1);   // toca em loop automatico
//    audioTrocarMusica(MUSICA_2);  // troca com fade out/in
//    audioPararMusica();           // para tudo
//
// 2. EFEITOS SONOROS (samples - arquivos curtos WAV):
//    audioTocarEfeito(SFX_PULO);   // toca uma vez
//    audioTocarEfeito(SFX_DASH);
//    audioTocarEfeito(SFX_MORTE);
//
// 3. VOLUME:
//    audioSetVolume(0.8f);         // 0.0 = mudo, 1.0 = maximo
//    audioMutar();                 // liga/desliga mudo
//    float v = audioGetVolume();   // le volume atual
//    bool m  = audioGetMutado();   // esta mutado?
//
// 4. ADICIONAR NOVO EFEITO:
//    - Coloque o .wav em assets/sounds/
//    - Adicione uma entrada em SfxID abaixo
//    - Adicione o caminho em sfxCaminhos[] em audio.c
//    - Chame audioTocarEfeito(SEU_SFX) onde quiser
// ============================================================

#include <stdbool.h>

// IDs das musicas
typedef enum {
    MUSICA_1 = 0,  // assets/sounds/musica1.mp3  (fases 1 e 2)
    MUSICA_2 = 1,  // assets/sounds/musica2.mp3  (fase 3 - gelo)
    MUSICA_TOTAL
} MusicaID;

// IDs dos efeitos sonoros
// Para adicionar novo sfx: coloque aqui e em sfxCaminhos[] em audio.c
typedef enum {
    SFX_PULO  = 0,  // assets/sounds/pulo.wav
    SFX_DASH  = 1,  // assets/sounds/dash.wav
    SFX_MORTE = 2,  // assets/sounds/morte.wav
    SFX_TOTAL
} SfxID;

// Inicializa o sistema de audio (chamar no main antes do glutMainLoop)
bool audioInit();

// Libera recursos (chamar ao fechar o jogo)
void audioFechar();

// Toca uma musica em loop (para a atual automaticamente)
void audioTocarMusica(MusicaID id);

// Troca de musica (para a atual e toca a nova)
void audioTrocarMusica(MusicaID id);

// Para toda musica
void audioPararMusica();

// Toca um efeito sonoro (nao interrompe a musica)
void audioTocarEfeito(SfxID id);

// Volume geral: 0.0 = mudo, 1.0 = maximo
void  audioSetVolume(float vol);
float audioGetVolume();

// Liga/desliga mudo (preserva o volume para quando desmutar)
void audioMutar();
bool audioGetMutado();

// Volume so da musica (independente do volume geral)
void  audioSetVolumMusica(float vol);
float audioGetVolumMusica();

#endif

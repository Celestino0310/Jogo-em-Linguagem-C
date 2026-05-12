#include "../include/audio.h"
#include "../include/bass.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// ============================================================
// ESTADO INTERNO
// ============================================================
static HSTREAM  musicaAtual   = 0;
static MusicaID musicaAtualID = -1;
static float    volumeGeral   = 0.10f;  // 0.0 a 1.0
static float    volumeMusica  = 0.9f;  // 0.0 a 1.0
static bool     mutado        = false;
static float    volumeAnteMute = 0.8f; // guarda volume antes de mutar

// Handles dos samples de efeito
static HSAMPLE sfxSamples[SFX_TOTAL];
static bool    sfxCarregado[SFX_TOTAL];

// Caminhos dos arquivos
static const char *musicaCaminhos[MUSICA_TOTAL] = {
    "../assets/sounds/MusicaBase.mp3",  // MUSICA_1
    "../assets/sounds/musica2.mp3",  // MUSICA_2
};

// ============================================================
// CAMINHOS DOS EFEITOS SONOROS
// Para adicionar novo efeito: coloque o .wav em assets/sounds/
// e adicione o caminho aqui na mesma ordem do enum SfxID
// ============================================================
static const char *sfxCaminhos[SFX_TOTAL] = {
    "assets/sounds/pulo.wav",   // SFX_PULO
    "assets/sounds/dash.wav",   // SFX_DASH
    "assets/sounds/morte.wav",  // SFX_MORTE
};

// ============================================================
// INIT
// ============================================================
bool audioInit() {
    // Inicializa BASS com dispositivo padrao, 44100Hz
    if (!BASS_Init(-1, 44100, 0, 0, NULL)) {
        printf("[AUDIO] Erro ao inicializar BASS: %d\n", BASS_ErrorGetCode());
        return false;
    }
    printf("[AUDIO] BASS inicializado OK\n");

    // Carrega samples de efeito sonoro
    int i;
    for (i = 0; i < SFX_TOTAL; i++) {
        sfxSamples[i] = BASS_SampleLoad(FALSE, sfxCaminhos[i],
                                        0, 0, 3, BASS_SAMPLE_OVER_POS);
        if (!sfxSamples[i]) {
            printf("[AUDIO] SFX nao encontrado: %s (ok, sera silencioso)\n",
                   sfxCaminhos[i]);
            sfxCarregado[i] = false;
        } else {
            printf("[AUDIO] SFX OK: %s\n", sfxCaminhos[i]);
            sfxCarregado[i] = true;
        }
    }

    // Volume global do BASS
    BASS_SetVolume(volumeGeral);
    return true;
}

// ============================================================
// FECHAR
// ============================================================
void audioFechar() {
    if (musicaAtual) BASS_StreamFree(musicaAtual);
    BASS_Free();
}

// ============================================================
// MUSICA
// ============================================================
void audioTocarMusica(MusicaID id) {
    if (id < 0 || id >= MUSICA_TOTAL) return;

    // Para musica anterior se houver
    if (musicaAtual) {
        BASS_StreamFree(musicaAtual);
        musicaAtual = 0;
    }

    // Cria stream com loop automatico
    musicaAtual = BASS_StreamCreateFile(FALSE, musicaCaminhos[id],
                                        0, 0,
                                        BASS_STREAM_AUTOFREE |
                                        BASS_SAMPLE_LOOP);
    if (!musicaAtual) {
        printf("[AUDIO] Musica nao encontrada: %s (erro %d)\n",
               musicaCaminhos[id], BASS_ErrorGetCode());
        return;
    }

    // Aplica volume da musica
    BASS_ChannelSetAttribute(musicaAtual, BASS_ATTRIB_VOL,
                             mutado ? 0.0f : volumeMusica);

    BASS_ChannelPlay(musicaAtual, FALSE);
    musicaAtualID = id;
    printf("[AUDIO] Tocando: %s\n", musicaCaminhos[id]);
}

void audioTrocarMusica(MusicaID id) {
    if (id == musicaAtualID) return; // ja esta tocando essa
    audioTocarMusica(id);
}

void audioPararMusica() {
    if (musicaAtual) {
        BASS_StreamFree(musicaAtual);
        musicaAtual   = 0;
        musicaAtualID = -1;
    }
}

// ============================================================
// EFEITOS SONOROS
// ============================================================
void audioTocarEfeito(SfxID id) {
    if (id < 0 || id >= SFX_TOTAL) return;
    if (!sfxCarregado[id]) return;
    if (mutado) return;

    HCHANNEL ch = BASS_SampleGetChannel(sfxSamples[id], FALSE);
    BASS_ChannelSetAttribute(ch, BASS_ATTRIB_VOL, volumeGeral);
    BASS_ChannelPlay(ch, FALSE);
}

// ============================================================
// VOLUME
// ============================================================
void audioSetVolume(float vol) {
    if (vol < 0.0f) vol = 0.0f;
    if (vol > 1.0f) vol = 1.0f;
    volumeGeral = vol;
    if (!mutado) {
        BASS_SetVolume(vol);
        if (musicaAtual)
            BASS_ChannelSetAttribute(musicaAtual, BASS_ATTRIB_VOL,
                                     vol * volumeMusica);
    }
}

float audioGetVolume() { return volumeGeral; }

void audioMutar() {
    mutado = !mutado;
    if (mutado) {
        volumeAnteMute = volumeGeral;
        BASS_SetVolume(0.0f);
        if (musicaAtual)
            BASS_ChannelSetAttribute(musicaAtual, BASS_ATTRIB_VOL, 0.0f);
    } else {
        BASS_SetVolume(volumeAnteMute);
        volumeGeral = volumeAnteMute;
        if (musicaAtual)
            BASS_ChannelSetAttribute(musicaAtual, BASS_ATTRIB_VOL,
                                     volumeAnteMute * volumeMusica);
    }
}

bool audioGetMutado() { return mutado; }

void audioSetVolumMusica(float vol) {
    if (vol < 0.0f) vol = 0.0f;
    if (vol > 1.0f) vol = 1.0f;
    volumeMusica = vol;
    if (musicaAtual && !mutado)
        BASS_ChannelSetAttribute(musicaAtual, BASS_ATTRIB_VOL,
                                 volumeGeral * vol);
}

float audioGetVolumMusica() { return volumeMusica; }

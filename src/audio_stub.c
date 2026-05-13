#include "../include/audio.h"
#include <stdbool.h>

static float volumeGeral = 0.10f;
static float volumeMusica = 0.90f;
static bool mutado = false;

bool audioInit(void) { return false; }
void audioFechar(void) {}
void audioTocarMusica(MusicaID id) { (void)id; }
void audioTrocarMusica(MusicaID id) { (void)id; }
void audioPararMusica(void) {}
void audioTocarEfeito(SfxID id) { (void)id; }

void audioSetVolume(float vol) {
    if (vol < 0.0f) vol = 0.0f;
    if (vol > 1.0f) vol = 1.0f;
    volumeGeral = vol;
}

float audioGetVolume(void) { return volumeGeral; }

void audioMutar(void) { mutado = !mutado; }
bool audioGetMutado(void) { return mutado; }

void audioSetVolumMusica(float vol) {
    if (vol < 0.0f) vol = 0.0f;
    if (vol > 1.0f) vol = 1.0f;
    volumeMusica = vol;
}

float audioGetVolumMusica(void) { return volumeMusica; }

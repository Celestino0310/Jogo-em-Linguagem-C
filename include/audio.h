#ifndef AUDIO_H
#define AUDIO_H






























#include <stdbool.h>


typedef enum {
    MUSICA_1 = 0,  
    MUSICA_2 = 1,  
    MUSICA_TOTAL
} MusicaID;



typedef enum {
    SFX_PULO  = 0,  
    SFX_DASH  = 1,  
    SFX_MORTE = 2,  
    SFX_TOTAL
} SfxID;


bool audioInit();


void audioFechar();


void audioTocarMusica(MusicaID id);


void audioTrocarMusica(MusicaID id);


void audioPararMusica();


void audioTocarEfeito(SfxID id);


void  audioSetVolume(float vol);
float audioGetVolume();


void audioMutar();
bool audioGetMutado();


void  audioSetVolumMusica(float vol);
float audioGetVolumMusica();

#endif

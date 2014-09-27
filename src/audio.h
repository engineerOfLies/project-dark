#ifndef __Audio__
#define __Audio__

#include <SDL_mixer.h>
enum S_GROUPS   {FX_NULL,FX_Bullets,FX_Impacts,FX_Monsters,FX_Player};

typedef struct SOUND_T
{
  Mix_Chunk *sound;
  char filename[80];
  int used;
  int volume;
}Sound;


void Init_Audio();
void InitSoundList();
void ClearSoundList();
void FreeSound(Sound *sound);
void FXPlaySound(Sound *sound,int channel);
Sound *LoadSound(char filename[80],int volume);
void FXPlaySoundFor(Sound *sound,int channel,Uint32 ticks);
void FXFadeInSoundFor(Sound *sound,int channel,Uint32 fade,Uint32 ticks);
Mix_Music *Load_Music(char *filename);/*loads music through  physFS*/
Mix_Chunk *Load_Sound(char* filename);/*Loads waves through physFS*/


#endif

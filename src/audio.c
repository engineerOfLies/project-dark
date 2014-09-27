#include <stdlib.h>
#include <string.h>
#include <physfs.h>
#include "audio.h"

#define MAXSOUNDCLIPS 128
int NumSounds = 0;
Sound sounds[MAXSOUNDCLIPS];

void FXPlaySound(Sound *sound,int channel)
{
  if((sound == NULL)||(sound->sound == NULL))return;
  Mix_PlayChannel(channel, sound->sound, 0);
}

void FXPlaySoundFor(Sound *sound,int channel,Uint32 ticks)
{
  if((sound == NULL)||(sound->sound == NULL))return;
  Mix_PlayChannelTimed(channel, sound->sound, 0,ticks);
}

void FXFadeInSoundFor(Sound *sound,int channel,Uint32 fade,Uint32 ticks)
{
  if((sound == NULL)||(sound->sound == NULL))return;
  Mix_FadeInChannelTimed(channel, sound->sound,0,fade,ticks);
}

void Init_Audio()
{                                           /*was using 4096*/
  if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,AUDIO_S16SYS,2,1024) == -1)
  {
    fprintf(stderr, "Unable to init SDL Mixer: %s\n", Mix_GetError());
    return;
  }
  atexit(Mix_CloseAudio);
  if(Mix_AllocateChannels(32) != 32)
  {
    fprintf(stderr, "Unable to allocate enough channels: %s\n", Mix_GetError());
    return;
  }
  Mix_VolumeMusic(MIX_MAX_VOLUME - 64);

  Mix_ReserveChannels(31);
  Mix_GroupChannels(0,3,FX_Bullets); /*bullet spawns*/
  Mix_GroupChannels(4,15,FX_Impacts); /*bullet impacts*/
  Mix_GroupChannels(16,19,FX_Monsters);/*monsters*/
  Mix_GroupChannels(20,23,FX_Player); /*player*/
}

void InitSoundList()
{
  int i;
  for(i = 0;i < MAXSOUNDCLIPS;i++)
  {
    sounds[i].sound = NULL;
    strcpy(sounds[i].filename,"\0");
    sounds[i].used = 0;
    sounds[i].volume = 0;
  }
  NumSounds = 0;
}

Sound *LoadSound(char filename[80],int volume)
{
  int i;
  /*first search to see if the requested sound is alreday loaded*/
  for(i = 0; i < MAXSOUNDCLIPS; i++)
  {
    if((strncmp(filename,sounds[i].filename,80)==0)&&(sounds[i].used >= 1))
    {
      sounds[i].used++;
      return &sounds[i];
    }
  }
  /*makesure we have the room for a new sound*/
  if(NumSounds + 1 >= MAXSOUNDCLIPS)
  {
        fprintf(stderr, "Maximum Sound clips Reached.\n");
        return NULL;
  }
  /*if its not already in memory, then load it.*/
  NumSounds++;
  for(i = 0;i < MAXSOUNDCLIPS;i++)
  {
    if(!sounds[i].used)break;
  }
  sounds[i].sound = Load_Sound(filename);
  if(sounds[i].sound == NULL)
  {
    fprintf(stderr, "FAILED TO LOAD A VITAL SOUND: %s.\n",filename);
        return NULL;
  }
  strcpy(sounds[i].filename,filename);
  sounds[i].volume = volume;
  sounds[i].used = 1;
  Mix_VolumeChunk(sounds[i].sound,volume);
  return &sounds[i];
}

void FreeSound(Sound *sound)
{
  if(!sound)return;
  if(sound->used > 1)
  {
    sound->used--;
    return;
  }
  NumSounds--;
  sound->used--;
  Mix_FreeChunk(sound->sound);
  sound->sound = NULL;
}

void ClearSoundList()
{
  int i;
  for(i = 0;i < MAXSOUNDCLIPS;i++)
  {
    if(sounds[i].sound != NULL)
    {
      Mix_FreeChunk(sounds[i].sound);
      sounds[i].sound = NULL;
    }
    strcpy(sounds[i].filename,"\0");
    sounds[i].used = 0;
    sounds[i].volume = 0;
  }
  NumSounds = 0;
}

Mix_Chunk *Load_Sound(char* filename)
{
  Mix_Chunk *music = NULL;
  SDL_RWops *rw;
  PHYSFS_sint64 size;
  PHYSFS_File *file;
  char *buffer = NULL;
  file = PHYSFS_openRead(filename);
  if(file == NULL)
  {
    fprintf(stderr,"unable to load file %s\n",filename);
    return NULL;
  }
  size = PHYSFS_fileLength(file);
  buffer = (char *)malloc(size);
  if(buffer == NULL)
  {
    fprintf(stderr,"Unable to allocate space for loading music: %s\n",filename);
    PHYSFS_close(file);
    return NULL;
  }
  PHYSFS_read(file,buffer,size,1);
  rw = SDL_RWFromMem(buffer, size);
  if(rw == NULL)
  {
    fprintf(stderr,"Unable to create RW reference to data loaded for %s\n",filename);
    free(buffer);
    PHYSFS_close(file);
    return NULL;
  }
  music = Mix_LoadWAV_RW(rw,0);
  SDL_FreeRW(rw);
  free(buffer);
  PHYSFS_close(file);
  return music;
}

Mix_Music *Load_Music(char *filename)
{
  Mix_Music *music = NULL;
  SDL_RWops *rw;
  PHYSFS_sint64 size;
  PHYSFS_File *file;
  char *buffer = NULL;
  file = PHYSFS_openRead(filename);
  if(file == NULL)
  {
    fprintf(stderr,"unable to load file %s\n",filename);
    return NULL;
  }
  size = PHYSFS_fileLength(file);
  buffer = (char *)malloc(size);
  if(buffer == NULL)
  {
    fprintf(stderr,"Unable to allocate space for loading music: %s\n",filename);
    PHYSFS_close(file);
    return NULL;
  }
  PHYSFS_read(file,buffer,size,1);
  rw = SDL_RWFromMem(buffer, size);
  if(rw == NULL)
  {
    fprintf(stderr,"Unable to create RW reference to data loaded for %s\n",filename);
    free(buffer);
    PHYSFS_close(file);
    return NULL;
  }
  music = Mix_LoadMUS_RW(rw);
  SDL_FreeRW(rw);
  free(buffer);
  PHYSFS_close(file);
  return music;
}


/*end of file*/

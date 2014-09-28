#include "levels.h"
#include "particle.h"
#include "entity.h"
#include "enemies.h"
#include "hud.h"
#include "levelmesh.h"
#include "levelobj.h"
#include "player.h"
#include "region.h"
#include "menus.h"

extern SDL_Rect Camera;
extern Mouse_T Mouse;
extern Entity *PlayerEnt;
extern PlayerSave PDat;
extern LevelHist *ThisLevel;

int selection = -1;   /*nothing selected*/
Level level;
int LevelOutside = 0;

Sprite *levelbg = NULL;
Sprite *leveltiles[MAXTILES];
Sprite *torchlight = NULL;
Sprite *Torch = NULL;
ObjTypes ObjTypeList[MAXOBJTYPES];
Sprite *ObjImages[MAXOBJTYPES];
int NumObjTypes = 0;
GLuint NightMask = 0;
Sprite *NightOverlay = NULL;

TileInfo TileList[MAXTILES];
int NumTiles = 0;

cpSpace *LevelSpace = NULL;   /*for use with 2D physics engine*/
cpShape **LevelShapes = NULL;
int     NumLevelShapes = 0;

/*

  LEVEL SECTION

*/

char *GetLevelName()
{
  return level.name;
}

void LoadLevelBackground()
{
  if(levelbg != NULL)FreeSprite(levelbg);
  levelbg = LoadSprite(level.background,level.w*TILEW,level.h*TILEH);
}

void SetupNightMask()
{
  glGenTextures(1, &NightMask);
  glBindTexture(GL_TEXTURE_2D,NightMask);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
}

void GenerateNightMask()
{
  int x,y;
  if(NightOverlay == NULL)
  {
    NightOverlay = NewSprite();
    if(NightOverlay == NULL)return;
  }
  if(NightMask == 0)
  {
    SetupNightMask();
    /*so it didn;t get initialized, initialize it*/
    if(NightMask == 0)return;
    /*well I guess its not working, give up.*/
    NightOverlay->image = NightMask;
  }
  if(PlayerEnt == NULL)
  {
    x = Camera.w/2;
    y = Camera.h/2;
  }
  else
  {
    x = PlayerEnt->p.x- Camera.x;
    y = PlayerEnt->p.y- Camera.y;
  }
  /*set rendering to back buffer*/
  glPushMatrix();
  glDrawBuffer(torchlight->image);
  glClearColor(0,0,0,1);
  glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
  /*fill it with solid black*/
  if(Torch != NULL)DrawSpriteAlpha( Torch, x, y, 0, 1,1, 0, 0.5 );
  /*for each entity that generates light*/
    /*draw the light mask to the Back  buffer*/
  /*read back buffer into a texture*/
  glReadBuffer(GL_AUX0);
  glBindTexture(GL_TEXTURE_2D,NightMask);
  glCopyTexImage2D(GL_TEXTURE_2D,0,0,0,0,0,Camera.w,Camera.h);
  glBindTexture(GL_TEXTURE_2D,0);
  /*set rendering back to FRONT*/
  glDrawBuffer(GL_FRONT | GL_BACK);
  glPopMatrix();
  /*draw Night Mask over everything*/
}

void DrawNightMask()
{
  int x,y;
  int minute;
  float torchlevel = 1;
  float nightlevel = 0.5;
  if(!LevelOutside)nightlevel = 0.8;
  else
  {
    minute = PDat.minute;
    /*make the zero hour the darkest part of the night*/
    if(minute >= 1080)
    {
      minute -= 1080;
    }
    else minute = 1440 + (minute - 1080);
    nightlevel = minute / (float)1440;
  }
  if(PlayerEnt == NULL)
  {
    x = Camera.w/2;
    y = Camera.h/2;
  }
  else
  {
    x = PlayerEnt->p.x- Camera.x;
    y = PlayerEnt->p.y- Camera.y;
  }
  DrawSpriteStretchRot( torchlight, x , y, 0,torchlevel,torchlevel, 0, nightlevel );
}

int CheckForLevel(char name[80])
{
  char path[80];
  FILE *file;
  sprintf(path,"levels/%s",name);
  file = fopen(path,"rb");
  if(file == NULL)
  {
    fprintf(stderr,"Failed to Load level %s\n",path);
    return 0;
  }
  fclose(file);
  return 1;
}

Object *GetNthObject(char obj[80],int index)
{
  int i;
  int n  = 0;
  for(i = 0;i < level.numobjects;i++)
  {
    if(strcmp(level.objlist[i].name,obj)==0)
    {
      if(n++ == index)
      {
        return &level.objlist[i];
      }
    }
  }
  return NULL;
}

Object *GetObjectByName(char obj[80])
{
  int i;
  for(i = 0;i < level.numobjects;i++)
  {
    if(strcmp(level.objlist[i].name,obj)==0)
    {
      return &level.objlist[i];
    }
  }
  return NULL;
}

void QuitGame()
{
  HideMouse();
  DrawSplashScreen();
  FreeEntity(PlayerEnt);
  ClearLevel();
  ClearPlayer();
  InitWindowStack();
  SetupBaseWindow();
}

/*parses the obj data for a level transition*/
void ExitLevel(char data[MAXOBJKEYS][80])
{
  char newlevel[80];
  char targobj[80];
  int index;
  char *txt;
  if(GetObjKey(data,"quit",&txt))
  {
    ShowMouse();
    YesNo(txt,QuitGame,HideMouse);
    return;
  }
  if(GetObjKey(data,"level",&txt))
  {
    sscanf(txt,"%s",newlevel);
  }
  else
  {
    fprintf(stderr,"unable to find level key\n");
    return;
  }
  if(GetObjKey(data,"target",&txt))
  {
    sscanf(txt,"%s %i",targobj,&index);
  }
  else
  {
    fprintf(stderr,"unable to find target object\n");
    return;
  }
  LevelTransition(newlevel,targobj,index);
}

void LevelTransition(char newlevel[80],char targetobj[80],int index)
{
  Object *obj;
  int offx = 0,offy = 0;
  char path[80];
  char *txt;
  if(!CheckForLevel(newlevel))
  {
    NewMessage("Unable to Load Level",IndexColor(LightRed));
    return;
  }
  /*Load new Level*/
  sprintf(path,"levels/%s",newlevel);
  LoadLevel(path);
  LevelOutside = 0;
  ClearEdges();
  BuildLattice();
  SelectLevelHistoryByName(newlevel);
  SpawnLevelEnts();
  /*find target object's locations and update player properties.*/
  obj = GetNthObject(targetobj,index);
  if(obj == NULL)
  {
    NewMessage("Failed to find destination!",IndexColor(LightRed));
    fprintf(stderr,"Unable to find %s # %i\n",targetobj,index);
    return;
  }
  if(PlayerEnt == NULL)
  {
    NewMessage("Player Ent is no longer here!",IndexColor(Red));
    return;
  }
  if(GetObjKey(obj->data,"offset",&txt))
  {
    sscanf(txt,"%i %i",&offx,&offy);
  }
  PlayerEnt->p.x = ((obj->x + offx) * TILEW) + TILEW/2;
  PlayerEnt->p.y = ((obj->y + offy) * TILEH) + TILEH/2;
  UpdateSeenMask(5);
}

void ConvertToTileXY(int inx,int iny,int *outx,int *outy)
{
  if(outx == NULL)return;
  if(outy == NULL)return;
  *outx = Mouse.x + Camera.x;
  *outy = Mouse.y + Camera.y;
  *outx /= TILEW;
  *outy /= TILEH;
}

void ClearLevel()
{
  /*clear out any previously loaded level data*/
  if(level.tilelist != NULL)free(level.tilelist);
  if(level.objlist != NULL)free(level.objlist);
  memset(&level,0,sizeof(level));
  if (PlayerEnt == NULL)
  {
    ClearEntities();
  }
  else
  {
    ClearEntitiesExcept(PlayerEnt);
  }
}

void NewLevel(char name[80],int x, int y, char background[80],char tileset[80], int fill, int deftile)
{
  int i,j;
  int size = x * y;
  int tile;
  char *mlist[] = {"knight","heavyknight"};
  tile = TileList[deftile].tileId;
  ClearLevel();
  level.tilelist = (Uint32 *)malloc(sizeof(Uint32) * size);
  if(level.tilelist == NULL)
  {
    fprintf(stderr,"Unable to allocate level tile list!\n");
    return;
  }
  level.w = x;
  level.h = y;
  level.numtiles = size;
  strcpy(level.background,background);
  strcpy(level.tileset,tileset);
  strcpy(level.name,name);
  if(fill == 3)
  {
    CreateRandomLevel(&level,Rect(0,0,x,y),tile,NULL,NULL,2,mlist);
  }
  else if(fill == 1)
  {
    for(i = 0;i < size;i++)
    {
      level.tilelist[i] = tile;
    }
  }
  else if(fill == 0)
  {
    memset(level.tilelist,0,sizeof(Uint32)*size);
    for(i = 0;i < x;i++)
    {
      level.tilelist[i] = tile;
      level.tilelist[((y - 1) * x) +  i] = tile;
    }
    for(j = 0;j < y;j++)
    {
      level.tilelist[j * x] = tile;
      level.tilelist[(j * x) + (x - 1)] = tile;
    }
  }
  else
  {
    memset(level.tilelist,0,sizeof(Uint32)*size);
  }
  level.inuse = 1;
  SetupLevel();
}

void SetLevelTileById(int x,int y, int id)
{
  if((x < 0) || (x >= level.w))return;
  if((y < 0) || (y >= level.h))return;
  level.tilelist[(y * level.w) + x] = id;
}


void ClearLevelTile(int x,int y)
{
  SetLevelTileById(x,y, 0);
}

int GetLevelTileXY(int x, int y)
{
  int i;
  i = (y * level.w) + x;
/*  fprintf(stdout,"tile %i: %i, %i!\n",i,x,y);*/
  if((i < 0)||(i > level.numtiles))return 0;
  return level.tilelist[i];
}

TileInfo *GetTileInforByID(int id)
{
  int i;
  for(i = 0;i < NumTiles;i++)
  {
    if(TileList[i].tileId == id)return &TileList[i];
  }
  return NULL;
}

void SetLevelTile(int x,int y, Uint32 tileindex)
{
  if((tileindex < 0)||(tileindex >= NumTiles))return;
  SetLevelTileById(x,y, TileList[tileindex].tileId);
}

/*Uses the lev
el data already loaded and builds the assets needed*/
void SetupLevel()
{
  if(!level.inuse)return;
  LoadTileSet(level.tileset);
  LoadLevelBackground();
  LoadObjTypes();
  if(torchlight == NULL)torchlight = LoadSprite("images/masks/torch.png",2048,2048);
  if(Torch == NULL)Torch= LoadSprite("images/UI/torchlight.png",128,128);
}

void DrawLevel(int edit)
{
  if(levelbg != NULL)
  {
    DrawSprite(levelbg,-Camera.x,-Camera.y,0,1);
  }
  drawAllEntities(edit,1);
  DrawTiles(edit);
  if(edit)
  {
    DrawLevelObjects();
  }
  else
  {
    drawAllEntities(edit,0);
    DrawAllParticles();
//    GenerateNightMask();
//    DrawNightMask();
    DrawLevelUnknown();
  }
  //DrawMesh();
/*  DrawAllParticles();*/
  glPopMatrix();
}

/*save the current level data to disk*/
void SaveLevel(char name[80])
{
  int i = 0;
  FILE *file;
  file = fopen(name,"wb");
  if(file == NULL)
  {
    fprintf(stderr,"Unable to save level %s file\n",name);
    return;
  }
  if(fwrite(level.name,80*sizeof(char),1,file) != 1)
  {
    fprintf(stderr,"unable to save level data!\n");
    fclose(file);
    return;
  }
  if(fwrite(level.background,80*sizeof(char),1,file) != 1)
  {
    fprintf(stderr,"unable to save level data!\n");
    fclose(file);
    return;
  }
  if(fwrite(level.tileset,80*sizeof(char),1,file) != 1)
  {
    fprintf(stderr,"unable to save level data!\n");
    fclose(file);
    return;
  }
  if(fwrite(&level.numtiles,sizeof(Uint32),1,file) != 1)
  {
    fprintf(stderr,"unable to save level data!\n");
    fclose(file);
    return;
  }
  if(fwrite(&level.w,sizeof(Uint32),1,file) != 1)
  {
    fprintf(stderr,"unable to save level data!\n");
    fclose(file);
    return;
  }
  if(fwrite(&level.h,sizeof(Uint32),1,file) != 1)
  {
    fprintf(stderr,"unable to save level data!\n");
    fclose(file);
    return;
  }
  if(fwrite(&level.numobjects,sizeof(Uint32),1,file) != 1)
  {
    fprintf(stderr,"unable to save level data!\n");
    fclose(file);
    return;
  }
  if(fwrite(&level.keychain,sizeof(Uint32),1,file) != 1)
  {
    fprintf(stderr,"unable to save level data!\n");
    fclose(file);
    return;
  }
  for(i = 0;i < level.numtiles;i++)
  {
    if(fwrite(&level.tilelist[i],sizeof(Uint32),1,file) != 1)
    {
      fprintf(stderr,"unable to save level tile list data!\n");
      fclose(file);
      return;
    }
  }
  for(i = 0;i < level.numobjects;i++)
  {
    if(fwrite(&level.objlist[i],sizeof(Object),1,file) != 1)
    {
      fprintf(stderr,"unable to save level object list data!\n");
      fclose(file);
      return;
    }
  }
}

void LoadLevel(char name[80])
{
  int i = 0;
  FILE *file;
  file = fopen(name,"rb");
  if(file == NULL)
  {
    fprintf(stderr,"Unable to open level %s file\n",name);
    return;
  }
  ClearLevel();
  if(fread(level.name,80*sizeof(char),1,file) != 1)
  {
    fprintf(stderr,"unable to load level data!\n");
    fclose(file);
    return;
  }
  if(fread(level.background,80*sizeof(char),1,file) != 1)
  {
    fprintf(stderr,"unable to load level data!\n");
    fclose(file);
    return;
  }
  if(fread(level.tileset,80*sizeof(char),1,file) != 1)
  {
    fprintf(stderr,"unable to load level data!\n");
    fclose(file);
    return;
  }
  if(fread(&level.numtiles,sizeof(Uint32),1,file) != 1)
  {
    fprintf(stderr,"unable to load level data!\n");
    fclose(file);
    return;
  }
  if(fread(&level.w,sizeof(Uint32),1,file) != 1)
  {
    fprintf(stderr,"unable to load level data!\n");
    fclose(file);
    return;
  }
  if(fread(&level.h,sizeof(Uint32),1,file) != 1)
  {
    fprintf(stderr,"unable to load level data!\n");
    fclose(file);
    return;
  }
  if(fread(&level.numobjects,sizeof(Uint32),1,file) != 1)
  {
    fprintf(stderr,"unable to load level data!\n");
    fclose(file);
    return;
  }
  if(fread(&level.keychain,sizeof(Uint32),1,file) != 1)
  {
    fprintf(stderr,"unable to load level data!\n");
    fclose(file);
    return;
  }
  level.tilelist = (Uint32 *)malloc(sizeof(Uint32)*level.numtiles);
  if(level.tilelist == NULL)
  {
    fprintf(stderr,"Unablle to allocate tile map!\n");
    return;
  }
  for(i = 0;i < level.numtiles;i++)
  {
    if(fread(&level.tilelist[i],sizeof(Uint32),1,file) != 1)
    {
      fprintf(stderr,"unable to load level tile list data!\n");
      fclose(file);
      return;
    }
  }
  level.objlist = (Object *)malloc(sizeof(Object)*level.numobjects);
  if(level.objlist == NULL)
  {
    fprintf(stderr,"Unablle to allocate object list!\n");
    return;
  }
  for(i = 0;i < level.numobjects;i++)
  {
    if(fread(&level.objlist[i],sizeof(Object),1,file) != 1)
    {
      fprintf(stderr,"unable to load level object list data!\n");
      fclose(file);
      return;
    }
  }
  level.inuse = 1;
  SetupLevel();
}

/*chipmunk physics interfacing*/

inline int GetLevelShapeCount()
{
  return NumLevelShapes;
}

inline cpShape *GetLevelShapeByIndex(int i)
{
  if((LevelShapes != NULL)&&(i >= 0)&&(i < NumLevelShapes))
  {
    return LevelShapes[i];
  }
  return NULL;
}

void BuildLattice()
{
  int i;
  int s = 0;
  if(LevelSpace == NULL)LevelSpace = cpSpaceNew();
  cpSpaceSetCollisionSlop(LevelSpace, 0.2);
  ClearLevelShapes();
  LevelShapes = (cpShape **)malloc(sizeof(cpShape *) * level.numtiles * 4);
  memset(LevelShapes,0,sizeof(cpShape *) * level.numtiles * 4);
  for(i = 0;i < level.numtiles;i++)
  {
    if(level.tilelist[i] != 0)
    {
      s = AddTileToMesh(i % level.w,i / level.w,s);
    }
  }
  NumLevelShapes = s;
}

int GetLevelSize(int *w, int *h)
{
  if(!level.inuse)return 0;
  if(w != NULL)*w = level.w;
  if(h != NULL)*h = level.h;
  return 1;
}

inline cpSpace *GetLevelSpace()
{
  return LevelSpace;
}

void ClearLevelShapes()
{
  int i;
  for(i = 0;i < NumLevelShapes;i++)
  {
    if(LevelShapes[i] != NULL)
    {
      cpSpaceRemoveStaticShape(LevelSpace, LevelShapes[i]);
      LevelShapes[i] = NULL;
    }
  }
  free(LevelShapes);
  NumLevelShapes = 0;
}

/*

  OBJECT SECTION

*/

void SpawnLevelEnts()
{
  int i;
  for(i = 0;i < level.numobjects;i++)
  {
    /*TODO: if it is still here!*/
    SpawnGenericObject(&level.objlist[i]);
  }
}


void RemoveKeyFromObject(Object *Object,int key)
{
  int i;
  if(Object != NULL)
  {
    if(key < Object->numkeys)
    {
      for(i = key;i < (Object->numkeys - 1);i++)
      {
        memcpy(&Object->data[i],&Object->data[i + 1],sizeof(Object->data[i]));
      }
      memset(&Object->data[Object->numkeys - 1],0,sizeof(Object->data[Object->numkeys - 1]));
      Object->numkeys--;
    }
  }
}

void AddDataKeyToObject(Object *obj,char key[80])
{
  if(obj == NULL)return;
  if(obj->numkeys >= MAXOBJKEYS)return;
  strcpy(obj->data[obj->numkeys],key);
  obj->numkeys++;
}

Object *GetObjectByXY(int x, int y)
{
  int i;
  for(i = 0;i < level.numobjects;i++)
  {
    if((level.objlist[i].x == x) && (level.objlist[i].y == y))return &level.objlist[i];
  }
  return NULL;
}

void RemoveObjectFromLevel(int x,int y)
{
  Object *obj;
  Object *next;
  obj = GetObjectByXY(x,y);
  if(obj == NULL)return;  /*nothing to remove*/
  for(;obj < &level.objlist[level.numobjects - 1];obj++)
  {
    next = (obj + 1);
    memcpy(obj,next,sizeof(Object));
  }
  level.numobjects--;
}

void AddObjectToLevel(int objindex,int x,int y)
{
  Object *obj;
  ObjTypes *otype;
  otype = GetObjTypeByIndex(objindex);
  if(otype == NULL)return;
  obj = GetObjectByXY(x,y);
  if(obj == NULL)
  { /*add*/
    obj = NewLevelObject();
    if(obj == NULL)return;  /*out of space or some error*/
  }
  strcpy(obj->name,otype->name);
  obj->x = x;
  obj->y = y;
}

void CloneObjectAt(Object *doner,int x,int y)
{
  Object *obj;
  Uint32 objnum;
  if(doner == NULL)return;
  obj = GetObjectByXY(x,y);
  if(obj == NULL)
  { /*add*/
    obj = NewLevelObject();
    if(obj == NULL)return;  /*out of space or some error*/
  }
  objnum = obj->id;
  memcpy(obj,doner,sizeof(Object));
  obj->id = objnum;
  obj->x = x;
  obj->y = y;
}

int GetObjTypeIndexByName(char name[80])
{
  int i;
  for(i = 0; i < NumObjTypes;i++)
  {
    if(strcmp(name, ObjTypeList[i].name) == 0)return i;
  }
  return -1;  /*not found*/
}

Object *GetObjectById(int id)
{
  int i;
  for(i = 0; i < level.numobjects;i++)
  {
    if(level.objlist[i].id == id)return &level.objlist[i];
  }
  return NULL;
}

void DrawObjToScreen(int id,int x,int y,int scale)
{
  float fscale = 1;
  if((id < 0)||(id >= NumObjTypes))return;
  if(scale)
  {
    fscale = ObjTypeList[id].scale;
  }
  if(ObjImages[id] != NULL)
  {
    DrawSpriteStretchRot( ObjImages[id], x + TILEW/2, y + TILEH/2, 0,fscale,fscale, 0, 1);
  }
}

void DrawObjectToLevel(int id,int x,int y)
{
  DrawObjToScreen(id,x - Camera.x,y - Camera.y,0);
}

void DrawLevelObjects()
{
  int i;
  int index;
  for(i = 0;i < level.numobjects;i++)
  {
    index = GetObjTypeIndexByName(level.objlist[i].name);
    DrawObjectToLevel(index,level.objlist[i].x * TILEW,level.objlist[i].y * TILEH);
  }
}

int GetNumberOfObjTypes()
{
  return NumObjTypes;
}

Object *NewLevelObject()
{
  Object *obj;
  obj = (Object *)malloc(sizeof(Object) * (level.numobjects + 1));
  if(obj == NULL)return NULL;
  memset(obj,0,sizeof(Object)* (level.numobjects + 1));
  memcpy(obj,level.objlist,sizeof(Object) * level.numobjects);
  free(level.objlist);/*out with the old*/
  level.objlist = obj;
  obj = &level.objlist[level.numobjects];
  obj->id = level.keychain++;
  level.numobjects++;
  return obj;
}

ObjTypes *GetObjTypeByIndex(int id)
{
  if((id < 0) || (id >= NumObjTypes))return NULL;
  return &ObjTypeList[id];
}

void LoadObjTypes()
{
  int i = 0;
  FILE *file;
  char *c;
  char buf[512];
  file = fopen("system/mapobjects.def","r");
  if(file == NULL)
  {
    fprintf(stderr,"Unable to open map object definition file");
    return;
  }
  memset(&ObjTypeList,0,sizeof(ObjTypeList));
  for(i = 0;i < NumObjTypes;i++)
  {
    if(ObjImages[i] != NULL)FreeSprite(ObjImages[i]);
  }
  memset(&ObjImages,0,sizeof(ObjImages));
  NumObjTypes = 0;
  i = 0;
  while(fscanf(file, "%s", buf) != EOF)
  {
    if(strcmp(buf,"#") ==0)
    {
      fgets(buf, sizeof(buf), file);
      continue;/*ignore the rest of the line.*/
    }
    if(strcmp(buf,"<end>") ==0)
    {
      ObjImages[i] = LoadSprite(ObjTypeList[i].sprite,ObjTypeList[i].w,ObjTypeList[i].h);
      i++;
      NumObjTypes++;
      if(i >= MAXOBJTYPES)
      {
        fprintf(stderr,"maximum number of map object types loaded!");
        return;
      }
      continue;
    }
    if(strcmp(buf,"<object>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(ObjTypeList[i].name, 80, file);
      c = strchr(ObjTypeList[i].name, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<model>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(ObjTypeList[i].sprite, 80, file);
      c = strchr(ObjTypeList[i].sprite, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<scale>") ==0)
    {
      fscanf(file, "%f",&ObjTypeList[i].scale);
      continue;
    }
    if(strcmp(buf,"<size>") ==0)
    {
      fscanf(file, "%i %i",&ObjTypeList[i].w,&ObjTypeList[i].h);
      continue;
    }
  }
}

/*

    TILE SECTION

*/


void LoadTileSet(char tileset[80])
{
  /*clear any previous and load the specific tile set*/
  int i = 0;
  FILE *file;
  char *c;
  char buf[512];
  file = fopen(tileset,"r");
  if(file == NULL)
  {
    fprintf(stderr,"Unable to open map object definition file");
    return;
  }
  memset(&TileList,0,sizeof(TileList));
  for(i = 0;i < NumTiles;i++)
  {
    if(leveltiles[i] != NULL)FreeSprite(leveltiles[i]);
  }
  memset(&leveltiles,0,sizeof(leveltiles));
  NumTiles = 0;
  i =  0;
  while(fscanf(file, "%s", buf) != EOF)
  {
    if(strcmp(buf,"#") ==0)
    {
      fgets(buf, sizeof(buf), file);
      continue;/*ignore the rest of the line.*/
    }
    if(strcmp(buf,"<end>") ==0)
    {
      leveltiles[i] = LoadSprite(TileList[i].tile,TILEW,TILEH);
      i++;
      NumTiles++;
      if(i >= MAXTILES)
      {
        fprintf(stderr,"maximum number of tile types loaded!");
        break;
      }
      continue;
    }
    if(strcmp(buf,"<tile>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(TileList[i].name, 80, file);
      c = strchr(TileList[i].name, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      TileList[i].arindex = i;
        continue;
    }
    if(strcmp(buf,"<image>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(TileList[i].tile, 80, file);
      c = strchr(TileList[i].tile, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<tileId>") ==0)
    {
      fscanf(file, "%i", &TileList[i].tileId);
      continue;
    }
    if(strcmp(buf,"<shape>") ==0)
    {
      fscanf(file, "%i", &TileList[i].shape );
      continue;
    }
  }
}

int GetTileIndex(int tileId)
{
  int i;
  if(tileId <= 0)return -1;
  for(i = 0;i < NumTiles;i++)
  {
    if(TileList[i].tileId == tileId)return TileList[i].arindex;
  }
  fprintf(stderr,"can't find tile with index %i!\n",tileId);
  return -1;
}

void DrawTileToScreen(int id,int x,int y)
{
  if((id < 0)||(id >= NumTiles))return;
  if((x + TILEW < 0)||(y + TILEH < 0)||(x > Camera.w)||(y > Camera.h))return;
  if(leveltiles[id] != NULL)
  {
    DrawSprite(leveltiles[id],x,y,0,1);
  }
}

void DrawTileByIndex(int id,int x,int y)
{
  DrawTileToScreen(id,x - Camera.x,y - Camera.y);
}

void DrawTile(int tileId,int x,int y)
{
  int id;
  /*draws the specified tile form the current tileset to the tilebuffer*/
  id = GetTileIndex(tileId);
  DrawTileByIndex(id,x,y);
}

void DrawTiles(int edit)
{
  int i,j;
  /*draws the tilemap for the loaded level*/
  if(!level.inuse)
    return;/*can't draw nothing*/
  if(level.tilelist == NULL)
    return;/*ditto*/
  for(j = 0;j < level.h;j++)
  {
    for(i = 0;i < level.w;i++)
    {
      if((edit)||(ThisLevel->ldata[i + (j * level.w)]))
      DrawTile(level.tilelist[j * level.w + i],i * TILEW,j * TILEH);
    }
  }
}

int GetNumberOfTiles()
{
  return NumTiles;
}


/*eol @ eof*/

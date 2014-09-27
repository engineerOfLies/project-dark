#ifndef __REGION__
#define __REGION__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include "levels.h"

#define RMINX 3
#define RMINY 3
#define RMAXX 7
#define RMAXY 5

enum C_Directions {D_N,D_E,D_S,D_W,D_NONE};
enum RoomTypes {R_ROOM,R_HALL};
enum DoorTypes {D_NoDoor,D_Door,D_Open,D_Locked,D_Barred,D_Trapped,D_Secret};

typedef struct D_Region
{
  int index;
  int depth;
  int color;
  SDL_Rect r;
  SDL_Point exits[4];
  int exitsused[4];
  int exitdir[4];     /*N,S,E,W*/
  int exitdoor[4];    /*NoDoor,Door,Locked,Trapped*/
  int exitcount;
  struct D_Region *exitreg[4];
  int rtype;      /*region type: hallway or room*/
  struct D_Region *parent;
  struct D_Region *next;
}D_Region;

typedef struct
{
  D_Region *regions;    /*list of regions*/
  int     numregions; /*how many regions are in the map*/
  SDL_Rect d;         /*level bounding rect*/
  SDL_Rect maxd;      /*level maximum bounds*/
  Uint32    *tiles;      /*tiles of the map*/
  int    *tilecolor;  /**/
  int     numtiles;   /*number of tiles in the list*/
  int     entrance;       /*enter at head region or deepest region*/
  D_Region  *up,*down;    /*level exists*/
}Dungeon;

void CreateRandomLevel(Level *level,SDL_Rect dim,Uint32 deftile,char *up, char *down,int numonsters,char *monsterlist[]);
void ClearDungeon();
void CreateDungeon(Uint32 deftile,int maxw, int maxh);
D_Region *NewD_Region();
void FreeD_Region(D_Region *r);
char *GetDirName(int i);
int RectOverlap(SDL_Rect r1,SDL_Rect r2);
int GetDirRectToPoint(SDL_Rect r,SDL_Point p);
int GetDirRectToRect(SDL_Rect r1, SDL_Rect r2);
int RelationDistance(D_Region *r1, D_Region *r2);
int D_RegionsLinked(D_Region *r1, D_Region *r2);
int GetCommonPoint(D_Region *r1, D_Region *r2, SDL_Point *p);
SDL_Rect GetD_RegionDimByDir(SDL_Rect r1,int dir);
SDL_Point GetD_RegionExitByDir(SDL_Rect r,int dir);
SDL_Rect Rect(int x, int y, int w, int h);
SDL_Point MkPoint(int x,int y);
int rand_int(int i);
void CalcDungeonDimensions();
void GetDungeonSize(int *w, int *h);
int GetRandomEmptyTile(SDL_Point *point);
void AddMonsters(int count,int typecount, char *mlist[]);

#endif


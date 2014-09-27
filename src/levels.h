#ifndef __LEVELS__
#define __LEVELS__

#include <chipmunk/chipmunk.h>
#include "glmodel.h"
#include "modelLoader.h"

#define MAXOBJTYPES 64
#define MAXTILES  32
#define TILEW 32
#define TILEH 32
#define MAXOBJKEYS 16

enum MoveTypes {M_Hit,M_Bounce,M_Slide};

typedef struct
{
  char    name[80];     /*name of level object*/
  Uint32  id;           /*unique ID for this item on this level*/
  int     numkeys;      /*how many data keys are on this object*/
  char    data[MAXOBJKEYS][80]; /*config data for the object*/
  Uint32  x,y;          /*position of object*/
  float   r;            /*rotation of object*/
}Object;

typedef struct
{
  char  name[80];         /*searchable name*/
  char  sprite[80];        /*model*/
  int   w,h;              /*dimensions of the sprite*/
  float scale;            /*default scale factor*/
}ObjTypes;

typedef struct
{
  char name[80];    /*tile name*/
  char tile[80];    /*tile image*/
  int  shape;       /*0 - square,  */
  int  tileId;       /*unique to each tileset*/
  int  arindex;     /*calculated when loaded*/
}TileInfo;

typedef struct
{
  Uint32  inuse;           /*makes sure there is something here before loading*/
  char name[80];        /*name of the map file without the path, but with extension.*/
  char background[80];  /*path to the model*/
  char tileset[80];     /*path to file with list of tiles*/
  Uint32  *tilelist;       /*tiles in the map*/
  Uint32  numtiles;        /*number of tiles in the map*/
  Uint32  w,h;              /*dimensions of the map*/
  Uint32  keychain;         /*new Ids are created from this*/
  Uint32  numobjects;      /*how many objects are placed in the map*/
  Object *objlist;      /*what objects are placed in the map*/
}Level;

void QuitGame();
char *GetLevelName();             /*returns the name of the current level loaded*/
void LoadLevel(char name[80]);     /*Load a level by filename*/
void SaveLevel(char name[80]);     /*Saves a level by filename*/
void DrawLevel(int edit);          /*draws the level each frame*/
void ClearLevel();                 /*clears data from the current level*/
void LoadTileSet(char tileset[80]);/*Loads a tile set*/
void ExitLevel(char data[MAXOBJKEYS][80]);  /*exits to the next  level based on exit's data*/
void LevelTransition(char newlevel[80],char targetobj[80],int index);
void BuildMaskFromTiles(int x,int y,int w, int h);  /*build a mesh mask for world collisions*/
void DrawMask();    /*draws the clip mask for the level (for debugging)*/
int  WorldTrace(Coord x, Coord v, float rad,int mtype, Coord *rv, Coord *norm);  /*checks for collision vs the world clip mask.  Returns 1 on collision and fills rv with the allowable change in position*/
void DrawTiles();         /*draws the tile map for the loaded level*/
void SetLevelTile(int x,int y, Uint32 tileindex); /*passed the index of the tile from the tile list*/
void ClearLevelTile(int x,int y); /*removes the tile in question*/
int GetLevelTileXY(int x, int y); /*returns the TileId at position x,y*/
TileInfo *GetTileInforByID(int id);
void NewLevel(char name[80],int x, int y, char background[80],char tileset[80], int fill, int deftile);
/*used by the map editor to create a bare template to making a level*/
void SetupLevel();    /*after a level is loaded/created sets it up for viewing / clipping*/
void BuildLattice();
int CheckForLevel(char name[80]);
void LoadLevelBackground();   /*clears old annd loads new sprite for the background.*/


/*chipmunk section*/
void ClearLevelShapes();
inline cpSpace *GetLevelSpace();
inline cpShape *GetLevelShapeByIndex(int i);
inline int GetLevelShapeCount();



/*Object Section*/
void CloneObjectAt(Object *doner,int x,int y);  /*creates a copy of doner at (x,y)*/
Object *GetObjectByName(char obj[80]);
Object *GetObjectById(int id);
Object *GetNthObject(char obj[80],int index);
void SpawnLevelEnts();    /*Spawn all level entities from the current level.*/
Object *GetObjectByXY(int x, int y);
Object *NewLevelObject(); /*creaates a new level object for map editing.*/
void LoadObjectTypes();   /*loads the object definition file*/
void DrawObjects();       /*draws the objects placed in the map for the map editor*/
int GetNumberOfObjTypes();  /*returns the number of loaded Obj Types*/
void DrawObjToScreen(int id,int x,int y,int scale); /*draws the object as the specified locaiton*/
void LoadObjTypes();
ObjTypes *GetObjTypeByIndex(int id);
void AddObjectToLevel(int objindex,int x,int y);
void DrawLevelObjects();
int GetObjTypeIndexByName(char name[80]);
void RemoveObjectFromLevel(int x,int y);
void AddDataKeyToObject(Object *obj,char key[80]);
void RemoveKeyFromObject(Object *Object,int key);

/*Tiles section*/
int GetTileIndex(int tileId);
int GetNumberOfTiles();   /*returns the number of tiles loaded*/
int GetLevelSize(int *w, int *h);      /*sets the dimensions of the currently loaded level*/
void ConvertToTileXY(int inx,int iny,int *outx,int *outy);  /*given screen coordinates, converts to tile position (takes into consideration the camera position)*/
void DrawTileByIndex(int id,int x,int y);
void DrawTileToScreen(int id,int x,int y);
#endif

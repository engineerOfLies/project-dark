#ifndef __LEVELOBJECTS__
#define __LEVELOBJECTS__

#include "levels.h"
#include "entity.h"

/* Walking movement falloff factor*/
#define WDAMP 0.5
enum Directions {D_Left, D_Up, D_Right, D_Down};
enum ObjectState {OS_Open, OS_Closed, OS_Disabled};

void SpawnGenericObject(Object *obj);
int GetObjKey(char data[MAXOBJKEYS][80],char *key,char **keyval);   /*if found, returns 1 and sets keyval*/
int GetNextObjKey(char data[MAXOBJKEYS][80],char *key,char **keyval, int *last);
Entity *GetClosestFoe(Entity *self,Entity *from);
Entity *AcquireTarget(Entity *self);
int CanSeeTarget(Entity *self, Entity *target);
void VectorToTarget(Entity *self,Coord *v); /*returns a normal vector in the direction of the target*/
void Walk(Entity *self);    /*updates entity vector based on movement vector*/
int EntityOnTile(Entity *ent,int x, int y);
int TileWalkable(int x, int y,Entity *ignore);
int InRangeOfTile(Entity *self,int x, int y);
void WalkTo(Entity *self,Point p);
/*interfacing with chipmunk*/
void AddEntityToSpace(Entity *ent);
void RemoveEntityFromSpace(Entity *ent);

#endif

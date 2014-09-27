#ifndef __GTRACE__
#define __GTRACE__

#include "levelmesh.h"
#include "entity.h"

#define F_SOLID     1
#define F_SIGHT     2
#define F_TDAMAGE   4
#define F_NODEAD    8

#define TRACESTRENGTH 50

enum HitType {HT_Wall, HT_Entity, HT_Bounds};

typedef struct
{
  int hit;        /*if this trace resulted in a hit*/
  int hittype;    /*what was hit 0 wall, 1 entity 2 level edge*/
  Entity *other;  /*if  it was an entity that was hit*/
  Point tile;     /*if it was a tile*/
  Coord POC;      /*point of collision.  Where the trace ends colliding*/
  Coord FV;       /*final Vector after the trace*/
  Coord normal;   /*normal of the edge it hit.*/
}Trace;

int MovingTowardsPoint(Coord p, Coord v, Coord t);    /*useful for a LOT of things*/
int GTrace(Coord s,Coord v, float radius, int clipmask, Trace *trace, Entity *self);  /*traces against the level and entities. checking for a hit*/
/*calls normal GTrace, after calculating the vector from s to g.*/
int GTracePtP(Coord s,Coord g, float radius, int clipmask, Trace *trace, Entity *self);
int EntTrace(Coord s,Coord v, float radius, int clipmask, Trace *trace, Entity *self);  /*mask takes a bit mask of the F_* defines from above.*/
int BoundTrace(Coord s,Coord v, float radius, int clipmask, Trace *trace);/*traces versus the level bounds*/
int LevelTrace(Coord s,Coord v, float radius, int clipmask, int ignore, Trace *trace);
int LookingTowardsPoint(Coord p, Coord v, Coord t,float range);
float AngleBetweenVectors2D(Coord v1,Coord v2);

int CanWalkTo(Coord s, Coord G,float radius,Entity *self);/*just a test, does not change anything*/

#endif

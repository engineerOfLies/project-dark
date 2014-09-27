#ifndef __PATHFIND__
#define __PATHFIND__

#include "levels.h"
#include "gtrace.h"

typedef struct
{
  int    used;  /*for record keeping*/
  Point  s;     /*this point*/
  Point  p;     /*the parent that lead us here*/
  float  d;     /*distance to goal*/
  int    steps; /*how many steps it took to get here*/
}PNode;


/*
  Final Path recovered is in order from [0] start.  Returned value is the length of the path found or -1 on failure.

*/
int GetPath(int sx,int sy,int gx,int gy,int team,Point *path,int pathlen,Entity *target);
/*using a quick A* path find to get a crude path form tile to tile.  Works, but results are ugly*/
void CleanPath(Point *path,int *pathlen,Entity *self);/*cleans up the path from GetPath to avoid erroneus tiles*/
#endif


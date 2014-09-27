#include "levelmesh.h"

extern SDL_Rect Camera;
extern cpShape **LevelShapes;

LevelMesh lmesh;

void DeleteEdge(LEdge *edge)
{
  if(edge == NULL)return;
  if(edge->next != NULL)DeleteEdge(edge->next);
  free(edge);
}

void ClearEdges()
{
  DeleteEdge(lmesh.edge);
  lmesh.numedges = 0;
  lmesh.edge = NULL;
}

Point XYtoPoint(int x, int y)
{
  Point xy;
  xy.x = x;
  xy.y = y;
  xy.z = 0;
  return xy;
}

int edgecomp(Point e1a,Point e1b,Point e2a, Point e2b)
{
  if((e1a.x == e2b.x) &&
    (e1a.y == e2b.y) &&
    (e1b.x == e2a.x) &&
    (e1b.y == e2a.y))return 1;
  if((e1a.x == e2a.x) &&
    (e1a.y == e2a.y) &&
    (e1b.x == e2b.x) &&
    (e1b.y == e2b.y))return 1;
  return 0;
}

void AddEdgeToMesh(int x1,int y1,int x2,int y2)
{
  LEdge *c = NULL;
  LEdge *empty = NULL;
  LEdge *last = NULL;
  if(lmesh.edge == NULL)/*first one*/
  {
    lmesh.edge = (LEdge *)malloc(sizeof(LEdge));
    if(lmesh.edge == NULL)
    {
      fprintf(stderr,"failed to create level collision mesh!\n");
    }
    lmesh.edge->usecount = 1;
    lmesh.edge->p[0].x = x1;
    lmesh.edge->p[0].y = y1;
    lmesh.edge->p[1].x = x2;
    lmesh.edge->p[1].y = y2;
    lmesh.edge->next = NULL;
    return;
  }
  for(c = lmesh.edge;c != NULL;c = c->next)
  {
    /*go through the list for the first occurance of the referenced edge, if found, increase its usecount*/
    if((c->usecount == 0)&&(empty == NULL))
    { /*get the first empty slot in case we need it.*/
      empty = c;
    }
    if(c->usecount)
    {
      if(edgecomp(c->p[0],c->p[1],XYtoPoint(x1,y1),XYtoPoint( x2, y2)))
      {
        c->usecount++;
        return; /*found it already!*/
      }
    }
    last = c;
  }
  if(empty != NULL)c = empty;
  else
  {
    if(last == NULL)
    {
      fprintf(stderr,"for some strange reason last is null.  levelmesh.c\n");
      return;
    }
    last->next = (LEdge *)malloc(sizeof(LEdge));
    c = last->next;
    memset(c,0,sizeof(LEdge));
  }
  c->usecount = 1;
  c->p[0].x = x1;
  c->p[0].y = y1;
  c->p[1].x = x2;
  c->p[1].y = y2;
}

void RemoveEdge(int x1,int y1,int x2,int y2)
{
  LEdge *e;
  for(e = lmesh.edge;e != NULL;e = e->next)
  {
    if(e->usecount > 0)
    {
      if(edgecomp(e->p[0],e->p[1],XYtoPoint(x1,y1),XYtoPoint( x2, y2)))
      {
        e->usecount--;/*we leave em in place.  removal is probably due to a door opening / closing*/
      }
    }
  }
}

cpShape *AddSegmentToSpace(int sx,int sy,int ex,int ey,cpSpace *space)
{
  cpShape *shape;
  if(space == NULL)return NULL;
  shape = cpSegmentShapeNew(NULL, cpv(sx,sy), cpv(ex,ey), 32);
  if(shape != NULL)cpSpaceAddStaticShape(space, shape);
  return shape;
}

int AddTileToMesh(int tx,int ty,int i)
{
  cpSpace *space;
  space = GetLevelSpace();
  /*old*/
  AddEdgeToMesh(tx,ty,tx + 1,ty);
  AddEdgeToMesh(tx,ty,tx,ty + 1);
  AddEdgeToMesh(tx,ty + 1,tx + 1,ty + 1);
  AddEdgeToMesh(tx + 1,ty,tx + 1,ty + 1);
  /*chipmunk*/
  tx *= TILEW;
  ty *= TILEH;
  LevelShapes[i++] = AddSegmentToSpace(tx,ty,tx + TILEW,ty,space);
  LevelShapes[i++] = AddSegmentToSpace(tx + TILEW,ty,tx + TILEW,ty + TILEH,space);
  LevelShapes[i++] = AddSegmentToSpace(tx + TILEW,ty + TILEH,tx,ty + TILEH,space);
  LevelShapes[i++] = AddSegmentToSpace(tx,ty + TILEH,tx,ty,space);
  return i;
}

void RemoveTile(int tx,int ty)
{
  RemoveEdge(tx,ty,tx + 1,ty);
  RemoveEdge(tx,ty,tx,ty + 1);
  RemoveEdge(tx,ty + 1,tx + 1,ty + 1);
  RemoveEdge(tx + 1,ty,tx + 1,ty + 1);
}

void DrawMesh()
{
  LEdge *c;
  for(c = lmesh.edge;c != NULL;c = c->next)
  {
    if(c->usecount == 1)
    {
      DrawLine2D(c->p[0].x * TILEW - Camera.x, c->p[0].y * TILEH - Camera.y, c->p[1].x * TILEW - Camera.x, c->p[1].y * TILEH - Camera.y,3, 1, 0, 0, 1);
    }
    if(c->usecount == 2)
    {
      DrawLine2D(c->p[0].x * TILEW - Camera.x, c->p[0].y * TILEH - Camera.y, c->p[1].x * TILEW - Camera.x, c->p[1].y * TILEH - Camera.y,3, 0, 1, 0, 1);
    }
    if(c->usecount >  2)
    {
      DrawLine2D(c->p[0].x * TILEW - Camera.x, c->p[0].y * TILEH - Camera.y, c->p[1].x * TILEW - Camera.x, c->p[1].y * TILEH - Camera.y,3, 0, 0, 1, 1);
    }
  }
}

/*eol@eof*/

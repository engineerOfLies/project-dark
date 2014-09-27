#ifndef __C_COLLIDE__
#define __C_COLLIDE__

#include "entity.h"
#include "modelLoader.h"
#include "gtrace.h"
/*hitmask bits*/
#define H_NONE    0
#define H_WALL    1
#define H_BOUNDS  2
#define H_ENTITY  4

typedef struct
{
  float p1[2];    /*xy of one point*/
  float p2[2];    /*xy of two point*/
}Edge2D;

typedef struct 
{
  float p[2];
}Point2D;

void ReflectVector(Coord v, Coord n,Coord *out);
void Normalize2D(float *vx, float *vy);
Point2D M_Point(float x1, float y1);
float PointLength(float x, float y);
Edge2D M_Edge(float x1, float y1, float x2, float y2);
int PointHitCircle(Point2D p, Point2D v, Point2D c, float rad, Point2D *out);
void GetEdgeBounding(Edge2D in, float rad, Edge2D *out1, Edge2D *out2);
float dot2D (Point2D a, Point2D b);
int CircleHitEdge(Point2D p,float rad,Point2D v,Edge2D e, Point2D *out, Coord *n);
face3i *GetNextOutSideEdge(face3i *face,int *edge,zfmModel *model,float x,float y,float minx,float maxx,float miny,float maxy);
int UpdateEntityPositionOld(Entity *ent,Coord *norm,Entity **other);
int UpdateEntityPosition(Entity *ent,Trace *rtrace);
void GetEdgeNormal(Edge2D e,Point2D p,Point2D *out);/*gets the normal of e relative to the point p*/
void GetClipEdge(Edge2D in, Coord p, float rad, Edge2D *out,Coord *norm);/*gets only 1 edge*/
int  PointInRect(Coord p,float x, float y,float w, float h);  /*2 point in rectangle check*/
#endif

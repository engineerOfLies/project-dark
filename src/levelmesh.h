#ifndef __LEVELMESH__
#define __LEVELMESH__

#include "levels.h"

typedef struct S_Edge
{
  int usecount;     /*how many tiles are using this edge.  if only 1, then its an outside edge*/
  Point p[2];       /*the end points for this edge*/
  struct S_Edge *next;
}LEdge;

typedef struct
{
  int numedges;     /*how many edges are in the list*/
  LEdge *edge;      /*list of edges*/
}LevelMesh;

void ClearEdges();
void AddEdgeToMesh(int x1,int y1,int x2,int y2); /*adds an edge if it isn't already there or increases the usecount*/
int AddTileToMesh(int tx,int ty,int i);  /*adds the edges of the tile to the mesh*/
void RemoveEdge(int x1,int y1,int x2,int y2);   /*reduces the edge count of the edge*/
void RemoveTile(int tx,int ty); /*removes the edges of the tile from the mesh*/
void DrawMesh();

#endif

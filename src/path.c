#include "path.h"
#include "levelobj.h"

extern Level level;
Entity *TargetEnt = NULL;
Sint32  CheckSize = 0;   /*reset for each level size*/
PNode *Checked = NULL;        /*for path building*/
PNode *Unchecked = NULL;      /*ditto*/

/*

  Path Optimizing
  Testing to see which tiles can be ignored in the list.

*/

/*returns new pathlen*/
int EliminateBetween(Point *path,int pathlen,int start,int target)
{
  int i;
  int dif = target - start;
  for(i = start + 1;(i < target - 1)&&(i + dif < pathlen);i++)
  {
    path[i].x = path[i + dif].x;
    path[i].y = path[i + dif].y;
  }
  return pathlen - dif;
}

void CleanPath(Point *path,int *pathlen,Entity *self)
{
  int i,t;  /*current test, target*/
  if(pathlen == NULL)return;
  if(self == NULL)return;
  for(i = 0;i < *pathlen - 1;i++)
  {
    for(t = *pathlen;t > i;--t)
    {
      if(CanWalkTo(M_Coord(path[i].x,path[i].y,0),M_Coord(path[t].x,path[t].y,0),self->radius,self))
      {
        /*eliminate the folk between*/
        *pathlen = EliminateBetween(path,*pathlen,i,t);
        break;
      }
    }
  }
}

/*

  Path Finding!

 */

int IsInList(PNode *list,int px,int py,int *index)
{
  int i;
  for(i = 0;i < CheckSize;i++)
  {
    if(list[i].used)
    {
      if((list[i].s.x == px)&&(list[i].s.y == py))
      {
        if(index != NULL)*index = i;
        return 1;
      }
    }
  }
  return 0;
}

int NewtoList(PNode *list,int sx,int sy,int px,int py,int d,int st)
{
  int i;
  for(i = 0;i < CheckSize;i++)
  {
    if(list[i].used == 0)
    {
      list[i].used = 1;
      list[i].steps = st;
      list[i].s.x = sx;
      list[i].s.y = sy;
      list[i].p.y = py;
      list[i].p.x = px;
      list[i].d = d;
      return 1;
    }
  }
  return 0;
}

int AddtoList(PNode *list,PNode node)
{
  int i;
  for(i = 0;i < CheckSize;i++)
  {
    if(list[i].used == 0)
    {
      memcpy(&list[i],&node,sizeof(PNode));
      return 1;
    }
  }
  return 0;
}

int GetBestUnchecked()
{
  int i;
  int best = -1;
  int bdist = 999999999;
  for(i = 0;i < CheckSize;i++)
  {
    if(Unchecked[i].used)
    {
      if(Unchecked[i].d < bdist)
      {
        bdist = Unchecked[i].d;
        best = i;
      }
    }
  }
  return best;
}

int TileValid(int tx,int ty,int team)
{
  return TileWalkable(tx, ty,TargetEnt);
}

/*Get Path will return -1 on failure or a number on distance to path
if a path is found, the array pointed to by path will be set with the path in Points*/
int GetPath(int sx,int sy,int gx,int gy,int team,Point *path,int pathlen,Entity *target)
{
  int u,i,p;
  int lsize;
  if((sx == gx)&&(sy == gy))
  {
    return -1;
  }
  /*self is the path*/
  if((sx < 0)||(sy < 0)||(gx < 0)||(gy < 0)||(sx >= level.w)||(gx >= level.w)||(sy >= level.h)||(gy >= level.h))
  {
    return -1;
  }
  if((abs(sx - gx) + abs(sy - gy)) > pathlen)
  {
    return -1;
  }
  lsize = level.numtiles;
  if((CheckSize != lsize)||(Checked == NULL)||(Unchecked == NULL))
  {
    CheckSize = lsize;
    if(Checked != NULL)free(Checked);
    if(Unchecked != NULL)free(Unchecked);
    Checked = (PNode *)malloc(sizeof(PNode)*lsize);
    Unchecked = (PNode *)malloc(sizeof(PNode)*lsize);
    if((Checked == NULL)||(Unchecked == NULL))
    {
      fprintf(stderr,"ran out of memory for path finding!\n");
      return -1;
    }
  }
  memset(Checked,0,sizeof(PNode)*lsize);
  memset(Unchecked,0,sizeof(PNode)*lsize);
  Unchecked[0].s.x = sx;
  Unchecked[0].s.y = sy;
  Unchecked[0].d = fabs(sx - gx) + fabs(sy - gy);
  Unchecked[0].p.x = -1;
  Unchecked[0].p.y = -1;
  Unchecked[0].used = 1;
  Unchecked[0].steps = 0;
  TargetEnt = target;
  do
  {
    u = GetBestUnchecked();
    if(u == -1)
    {
      /*none left to check!*/
      return -1;
    }
    /*add */
    AddtoList(Checked,Unchecked[u]);
    if((Unchecked[u].s.x == gx)&&(Unchecked[u].s.y == gy))
    {
      break;
    }
      /*tile to the left*/
      if((Unchecked[u].s.x - 1) >= 0)/*first, make sure we're on the map*/
      {
        if((IsInList(Unchecked,Unchecked[u].s.x - 1,Unchecked[u].s.y,NULL) == 0)&&(IsInList(Checked,Unchecked[u].s.x - 1,Unchecked[u].s.y,NULL) == 0))/*make sure we don't repeat a search*/
        {
          if(TileValid(Unchecked[u].s.x - 1,Unchecked[u].s.y,team))
            NewtoList(Unchecked,Unchecked[u].s.x - 1,Unchecked[u].s.y, Unchecked[u].s.x, Unchecked[u].s.y, abs((Unchecked[u].s.x - 1) - gx) + abs(Unchecked[u].s.y - gy), Unchecked[u].steps + 1);
        }
      }
      /*tile to the right*/
      if((Unchecked[u].s.x + 1) < level.w)/*first, make sure we're on the map*/
      {
        if((IsInList(Unchecked,Unchecked[u].s.x + 1,Unchecked[u].s.y,NULL) == 0)&&(IsInList(Checked,Unchecked[u].s.x + 1,Unchecked[u].s.y,NULL) == 0))/*make sure we don't repeat a search*/
        {
          if(TileValid(Unchecked[u].s.x + 1,Unchecked[u].s.y,team))
            NewtoList(Unchecked,Unchecked[u].s.x + 1,Unchecked[u].s.y, Unchecked[u].s.x, Unchecked[u].s.y, abs((Unchecked[u].s.x + 1) - gx) + abs(Unchecked[u].s.y - gy), Unchecked[u].steps + 1);
        }
      }
      /*tile below*/
      if((Unchecked[u].s.y + 1) < level.h)/*first, make sure we're on the map*/
      {
        if((IsInList(Unchecked,Unchecked[u].s.x ,Unchecked[u].s.y + 1,NULL) == 0)&&(IsInList(Checked,Unchecked[u].s.x,Unchecked[u].s.y + 1,NULL) == 0))/*make sure we don't repeat a search*/
        {
          if(TileValid(Unchecked[u].s.x,Unchecked[u].s.y + 1,team))
            NewtoList(Unchecked,Unchecked[u].s.x,Unchecked[u].s.y + 1, Unchecked[u].s.x, Unchecked[u].s.y, abs(Unchecked[u].s.x - gx) + abs((Unchecked[u].s.y + 1) - gy), Unchecked[u].steps + 1);
        }
      }
      /*tile above*/
      if((Unchecked[u].s.y - 1) >= 0)/*first, make sure we're on the map*/
      {
        if((IsInList(Unchecked,Unchecked[u].s.x ,Unchecked[u].s.y - 1,NULL) == 0)&&(IsInList(Checked,Unchecked[u].s.x,Unchecked[u].s.y - 1,NULL) == 0))/*make sure we don't repeat a search*/
        {
          if(TileValid(Unchecked[u].s.x,Unchecked[u].s.y - 1,team))
            NewtoList(Unchecked,Unchecked[u].s.x,Unchecked[u].s.y - 1, Unchecked[u].s.x, Unchecked[u].s.y, abs(Unchecked[u].s.x - gx) + abs((Unchecked[u].s.y - 1) - gy), Unchecked[u].steps + 1);
        }
      }
      memset(&Unchecked[u],0,sizeof(PNode));
  }
  while(1);
  IsInList(Checked,Unchecked[u].s.x,Unchecked[u].s.y,&u);
  p = Checked[u].steps;
  if(p >= pathlen)
  {
    fprintf(stderr,"path too long for the entity to remember!\n");
    return -1;
  }
  if(path != NULL)
  {
    for(i = (p - 1);i >= 0;i--)
    {
      path[i].x = Checked[u].s.x;
      path[i].y = Checked[u].s.y;
      IsInList(Checked,Checked[u].p.x,Checked[u].p.y,&u);
    }
  }
  return p;
}


/*eol@eof*/

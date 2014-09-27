#include <math.h>
#include "region.h"
#include "entity.h"
#include "levels.h"

Dungeon dungeon;

void PrintDungeon()
{
  int i,j,index;
  for(j = 0;j < dungeon.d.h;j++)
  {
    for(i = 0;i < dungeon.d.w;i++)
    {
      index = i + (j * dungeon.d.w);
      fprintf(stdout,"%i",dungeon.tiles[index]);
    }
    fprintf(stdout,"\n");
  }
}

/*

  This is the glue between the level and random dungeon generator

*/

void CreateRandomLevel(Level *level,SDL_Rect dim,Uint32 deftile,char *up, char *down,int numonsters,char *monsterlist[])
{
  int i;
  char s[80];
  Object *o;
  if(level == NULL)return;
  CreateDungeon(deftile,dim.w,dim.h);
  AddMonsters(random() * 20,numonsters, monsterlist);
  if(level->tilelist != NULL)free(level->tilelist);
  level->tilelist = (Uint32 *)malloc(sizeof(Uint32)*dungeon.numtiles);
  if(level->tilelist == NULL)
  {
    fprintf(stderr,"unable to allocat memory for generated level dungeon!\n");
    return;
  }
  for(i = 0;i < dungeon.numtiles;i++)
  {
    level->tilelist[i] = dungeon.tiles[i];
  }
  level->w = dungeon.d.w;
  level->h = dungeon.d.h;
  level->numtiles = dungeon.numtiles;
  /*PrintDungeon();*/
  o = GetObjectByName("stairsup");
  if(o != NULL)
  {
    sprintf(s,"level %s",up);
    AddDataKeyToObject(o,s);
    AddDataKeyToObject(o,"offset 1 0");
    AddDataKeyToObject(o,"target stairsdown 0");
  }
  o = GetObjectByName("stairsdown");
  if(o != NULL)
  {
    sprintf(s,"level %s",down);
    AddDataKeyToObject(o,s);
    AddDataKeyToObject(o,"offset 1 0");
    AddDataKeyToObject(o,"target stairsup 0");
  }
}

int GetDistanceBetweenRegions(D_Region *r1,D_Region *r2)
{
  SDL_Point p1,p2;
  if((r1 == NULL)||(r2 == NULL))return -1;
  p1.x = r1->r.x + r1->r.w/2;
  p1.y = r1->r.y + r1->r.h/2;
  p2.x = r2->r.x + r2->r.w/2;
  p2.y = r2->r.y + r2->r.h/2;
  return (int)RelativeSize(p1.x - p2.x, p1.y - p2.y);
}

int IsPointOpen(SDL_Point p)
{
  SDL_Point r;
  r.x = p.x - dungeon.d.x;
  r.y = p.y - dungeon.d.y;
  if(dungeon.tiles[r.x + (r.y*dungeon.d.w)] != 0)return 0;
  if(GetObjectByXY(r.x, r.y) != NULL)return 0;
  return 1;
}

int GetRandomEmptyTile(SDL_Point *point)
{
  int attempts = 100;
  SDL_Point p;
  SDL_Point stepv;
  if(point == NULL)return 0;
  while(attempts > 0)
  {
    attempts--;
    p.x = random()*dungeon.d.w + dungeon.d.x;
    p.y = random()*dungeon.d.h + dungeon.d.y;
    if(p.x > dungeon.d.w/2)stepv.x = -1;
    else if(p.x < dungeon.d.w/2)stepv.x = 1;
    else stepv.x = 0;
    if(p.y > dungeon.d.h/2)stepv.y = -1;
    else if(p.y < dungeon.d.h/2)stepv.y = 1;
    else stepv.y = 0;
    while(SDLPointInRect(p,dungeon.d))
    {
      if(IsPointOpen(p))
      {
        point->x = p.x - dungeon.d.x;
        point->y = p.y - dungeon.d.y;
        return 1;
      }
      p.x += stepv.x;
      p.y += stepv.y;
    }
  }
  return 0;
}

void AddMonsters(int count,int typecount, char *mlist[])
{
  Object *obj;
  SDL_Point p;
  char s[80];
  int i;
  int o;
  if(mlist == 0)return;
  for(i = 0;i < count;i++)
  {
    if(GetRandomEmptyTile(&p))
    {
      o = GetObjTypeIndexByName("monster");
      if(o != -1)
      {
        AddObjectToLevel(o,p.x,p.y);
        obj = GetObjectByXY(p.x, p.y);
        sprintf(s,"facing %i",(int)(random() * 360));
        AddDataKeyToObject(obj,s);
        sprintf(s,"monster %s",mlist[(int)(random() * typecount)]);
        AddDataKeyToObject(obj,s);
      }
    }
  }
}

void FindFarthestRooms(D_Region **r1,D_Region **r2)
{
  D_Region *c1,*c2;
  int bestdist = -1;
  if((r1 == NULL)||(r2 == NULL))return;
  c1 = dungeon.regions;
  while(c1 != NULL)
  {
    c2 = c1->next;
    while(c2 != NULL)
    {
      if((c1->rtype != R_HALL)&&(c2->rtype != R_HALL))
      {
        if(GetDistanceBetweenRegions(c1,c2) > bestdist)
        {
          *r1 = c1;
          *r2 = c2;
          bestdist = GetDistanceBetweenRegions(c1,c2);
        }
      }
      c2 = c2->next;
    }
    c1 = c1->next;
  }
}

int FitD_Region(D_Region *r)
{
  int w,h;
  int count = 0;
  D_Region *c;
  int dir = 0;
  c = dungeon.regions;
  while(c != NULL)
  {
    if(c != r)
    {
      count++;
      while(RectOverlap(Rect(r->r.x - 1,r->r.y - 1,r->r.w + 2,r->r.h + 2),c->r))
      {
        /*shrink our rect until it fits, or it gets too small.*/
        dir = GetDirRectToRect(r->r,c->r);
        switch(dir)
        {
          case D_N:
            r->r.h--;
            r->r.y++;
            break;
          case D_S:
            r->r.h--;
            break;
          case D_E:
            r->r.w--;
            break;
          case D_W:
            r->r.w--;
            r->r.x++;
          default:
            return 0;
        }
        if((r->r.w < RMINX) || (r->r.h < RMINY))
          return 0;
      }
    }
    c=c->next;
  }
  GetDungeonSize(&w,&h);
  if((w > dungeon.maxd.w)||(h > dungeon.maxd.h))return 0;
  return 1;
}

void AddExitToD_Region(D_Region *r, SDL_Point enter, D_Region *link,int doortype)
{
  int i;
  if((r == NULL)||(link == NULL))return;
  for(i = 0;i < 4;i++)
  {
    if(r->exitsused[i] == 0)
    {
      r->exits[i].x = enter.x;
      r->exitcount++;
      r->exits[i].y = enter.y;
      r->exitsused[i] = 1;
      r->exitreg[i] = link;
      r->exitdir[i] = GetDirRectToPoint(r->r,enter);
      if(doortype != -1)r->exitdoor[i] = doortype;
      return;
    }
  }
}

D_Region *AddD_Region(int x, int y, int w, int h,int depth,SDL_Point enter,D_Region *p)
{
  D_Region *r;
  int exits = 0;
  int i;
  int dir = 0;
  SDL_Point xp;
  SDL_Rect nextr;
  r = NewD_Region();
  if(r == NULL)return NULL;
  r->parent = p;
  r->r.x = x;
  r->r.y = y;
  r->r.w = w;
  r->r.h = h;
  r->depth = depth;
  r->color = LightGrey;
  if(!FitD_Region(r))
  {
    FreeD_Region(r);
    return NULL;
  }
  /*Fit D_Region may have altered the dimensions*/
  x = r->r.x;
  y = r->r.y;
  w = r->r.w;
  h = r->r.h;
  exits = 2 + rand_int(3 + depth);
  if(exits > 4)exits = 4;
  dir = rand_int(4);
  i = 0;
  if(p != NULL)
  {
    AddExitToD_Region(r, enter, p,-1);
    dir = r->exitdir[0];
    i = 1;
  }
  if(depth <= 0)
  {
    r->rtype = R_ROOM;
    r->color = LightBlue;
    return r;
  }
  if(rand_int(4)==0)r->rtype = R_ROOM;
  else r->rtype = R_HALL;
  for(;i < exits;i++)
  {
    dir = (dir + 1)%4;
    xp = GetD_RegionExitByDir(r->r,dir);
    nextr = GetD_RegionDimByDir(r->r,dir);
    AddExitToD_Region(r,xp,AddD_Region(nextr.x,nextr.y,nextr.w,nextr.h,depth-1,xp,r),-1);
  }
  if(r->exitcount <= 1)r->rtype = R_ROOM;
  if(r == dungeon.regions)
  {
    r->rtype = R_ROOM;
    r->color = LightRed;
  }
  return r;
}

void LinkAdjacentD_Regions()
{
  SDL_Point p;
  D_Region *c1,*c2;
  c2 = dungeon.regions;
  while(c2 != NULL)
  {
    c1 = c2->next;
    while(c1 != NULL)
    {
      /*if neither can have an additional exit, forget it!*/
      if((c1->exitcount < 4)&&(c2->exitcount < 4))
      {
        if(!D_RegionsLinked(c1, c2))
        {
          if(RelationDistance(c1, c2) >= 3)
          {
            if(GetCommonPoint(c1, c2, &p)==1)
            {
              AddExitToD_Region(c1, p, c2,D_Secret);
              AddExitToD_Region(c2, p, c1,D_Secret);
            }
          }
        }
      }
      c1 = c1->next;
    }
    c2 = c2->next;
  }
}

void SetDungeonDoors()
{
  D_Region *c;
  int i;
  c = dungeon.regions;
  while(c != NULL)
  {
    for(i = 0;i < 4;i++)
    {
      if((c->exitsused[i])&&(c->exitreg[i] != NULL))
      {
        if((c->rtype == R_HALL)&&(c->exitreg[i]->rtype == R_HALL))
        {
          c->exitdoor[i] = D_NoDoor;
          continue;
        }
        c->exitdoor[i] = D_Door + abs(rand_int(5));
      }
    }
    c = c->next;
  }
}

void DigHall(D_Region *r, int c)
{
  int i,j,e;
  int index;
  int step;
  int xdir = 0,ydir = 0;
  int xcenter,ycenter;
  int steplen = 0;
  int x,y;
  x = -dungeon.d.x;
  y = -dungeon.d.y;
  xcenter = r->r.x + (r->r.w/2);
  ycenter = r->r.y + (r->r.h/2);
  for(e = 0;e < 4;e++)
  {
    if(r->exitsused[e])
    {
      switch(r->exitdir[e])
      {
        case D_N:
          steplen = abs(r->exits[e].y - ycenter);
          xdir = 0;
          ydir = 1;
          break;
        case D_S:
          steplen = abs(r->exits[e].y - ycenter);
          xdir = 0;
          ydir = -1;
          break;
        case D_E:
          steplen = abs(r->exits[e].x - xcenter);
          xdir = -1;
          ydir = 0;
          break;
        case D_W:
          steplen = abs(r->exits[e].x - xcenter);
          xdir = 1;
          ydir = 0;
          break;
        default:
          xdir = 0;
          ydir = 0;
          steplen = 0;
      }
      i = r->exits[e].x + xdir;
      j = r->exits[e].y + ydir;
      for(step = 0;step < steplen;step++)
      {
        index = i + x + ((y + j) * dungeon.d.w);
        if((index < dungeon.numtiles)&&(index > 0))
        {
          dungeon.tiles[index] = 0;
          dungeon.tilecolor[index] = c;
        }
        i += xdir;
        j += ydir;
      }
    }
  }
}

void DigRoom(D_Region *r,int c)
{
  int i,j;
  int index;
  int count = 0;
  int x,y;
  x = r->r.x - dungeon.d.x;
  y = r->r.y - dungeon.d.y;
  for(j = 0;j < r->r.h;j++)
  {
    for(i = 0;i < r->r.w;i++)
    {
      index = i + x + ((y + j) * dungeon.d.w);
      if((index < dungeon.numtiles)&&(index > 0))
      {
        dungeon.tiles[index] = 0;
        dungeon.tilecolor[index] = c;
        count ++;
      }
    }
  }
}

void AddExits()
{
  int i,x,y;
  FindFarthestRooms(&dungeon.up,&dungeon.down);
  if(dungeon.up != NULL)
  {
    x = (dungeon.up->r.x + dungeon.up->r.w/2) - dungeon.d.x;
    y = (dungeon.up->r.y + dungeon.up->r.h/2) - dungeon.d.y;
    i = GetObjTypeIndexByName("stairsup");
    AddObjectToLevel(i,x,y);
  }
  if(dungeon.down != NULL)
  {
    x = (dungeon.down->r.x + dungeon.down->r.w/2) - dungeon.d.x;
    y = (dungeon.down->r.y + dungeon.down->r.h/2) - dungeon.d.y;
    i = GetObjTypeIndexByName("stairsdown");
    AddObjectToLevel(i,x,y);
  }
}

void AddDoor(int index,int x, int y, int door,int dir)
{
  int i;
  dungeon.tiles[index] = 0;
  /*add door object...*/
  if(door == D_NoDoor)return;
  switch(dir)
  {
    case D_E:
    case D_W:
      i = GetObjTypeIndexByName("v_door");
      break;
    default:
      i = GetObjTypeIndexByName("h_door");
  }
  AddObjectToLevel(i,x,y);
}

void DigDoors(D_Region *r)
{
  int i;
  int index;
  for(i = 0; i< 4;i++)
  {
    if(r->exitsused[i])
    {
      index = (r->exits[i].x - dungeon.d.x) + ((r->exits[i].y - dungeon.d.y) * dungeon.d.w);
      if((index >= 0)&&(index < dungeon.numtiles))
      {
        AddDoor(index,r->exits[i].x - dungeon.d.x,r->exits[i].y - dungeon.d.y,r->exitdoor[i], r->exitdir[i]);
      }
    }
  }
}

void DigRooms()
{
  D_Region *r;
  int count = 0;
  r = dungeon.regions;
  while(r != NULL)
  {
    if(r->rtype == R_HALL)DigHall(r,r->color);
    else DigRoom(r,r->color);
    r = r->next;
    count++;
  }
  r = dungeon.regions;
  while(r != NULL)
  {
    DigDoors(r);
    r = r->next;
  }
}

void SetDungeonDetail(Uint32 deftile)
{
  int i;
  int maxx,maxy;
  D_Region *r;
  r = dungeon.regions;
  if(r == NULL)return;
  maxx = maxy = -999999999;
  while(r != NULL)
  {
    dungeon.d.x = MIN(dungeon.d.x,r->r.x);
    dungeon.d.y = MIN(dungeon.d.y,r->r.y);
    maxx = MAX(maxx,r->r.x + r->r.w);
    maxy = MAX(maxy,r->r.y + r->r.h);
    r = r->next;
  }
  dungeon.d.x--;
  dungeon.d.y--;
  dungeon.d.w = maxx - dungeon.d.x + 1;
  dungeon.d.h = maxy - dungeon.d.y + 1;
  dungeon.numtiles = dungeon.d.h*dungeon.d.w;
  dungeon.tiles = (Uint32 *)malloc(sizeof(Uint32)*dungeon.d.h*dungeon.d.w);
  if(dungeon.tiles == NULL)
  {
    fprintf(stderr,"Unable to allocate dungeon tiles!\n");
    return;
  }
  dungeon.tilecolor = (int *)malloc(sizeof(int)*dungeon.d.h*dungeon.d.w);
  for(i = 0;i < dungeon.numtiles;i++)
  {
    dungeon.tiles[i] = deftile;
  }
  LinkAdjacentD_Regions();
  SetDungeonDoors();
  DigRooms();
  AddExits();
}

void GetDungeonSize(int *w, int *h)
{
  int minx,miny;
  int maxx,maxy;
  D_Region *r;
  if((w == NULL)||(h == NULL))return;
  r = dungeon.regions;
  if(r == NULL)return;
  maxx = maxy = -999999999;
  minx = miny = 999999999;
  while(r != NULL)
  {
    minx = MIN(minx,r->r.x);
    miny = MIN(miny,r->r.y);
    maxx = MAX(maxx,r->r.x + r->r.w);
    maxy = MAX(maxy,r->r.y + r->r.h);
    r = r->next;
  }
  minx--;
  miny--;
  *w = maxx - minx + 1;
  *h = maxy - miny + 1;
}

void CreateDungeon(Uint32 deftile,int maxw, int maxh)
{
  ClearDungeon();
  dungeon.maxd.w = maxw;
  dungeon.maxd.h = maxh;
  dungeon.entrance = (int)(random() * 10) % 2;
  AddD_Region(0,0,3 + abs(rand_int(4)),3 + abs(rand_int(3)), 8,MkPoint(0,0),NULL);
  SetDungeonDetail(deftile);
}

void ClearDungeon()
{
  if(dungeon.tiles != NULL)free(dungeon.tiles);
  memset(&dungeon,0,sizeof(dungeon));
}

void FreeD_Region(D_Region *r)
{
  D_Region *c;
  if(r == NULL)return;
  if(r == dungeon.regions)
  {
    dungeon.regions = dungeon.regions->next;
    free(r);
    return;
  }
  c = dungeon.regions;
  while(c != NULL)
  {
    if(c->next == r)
    {
      c->next = r->next;
      dungeon.numregions--;
      free(r);
      return;
    }
    c = c->next;
  }
}

D_Region *NewD_Region()
{
  D_Region *r;
  D_Region *c;
  r = (D_Region *)malloc(sizeof(D_Region));
  if(r == NULL)
  {
    fprintf(stderr,"Unable to add new region\n");
    return NULL;
  }
  memset(r,0,sizeof(D_Region));
  dungeon.numregions++;
  if(dungeon.regions == NULL)
  {
    dungeon.regions = r;
    return r;
  }
  c = dungeon.regions;
  while(c->next != NULL)c = c->next;
  c->next = r;
  return r;
}

int RectOverlap(SDL_Rect a,SDL_Rect b)
{
  if((a.x + a.w) <= b.x)return 0;
  if((a.y + a.h) <= b.y)return 0;
  if((b.x + b.w) <= a.x)return 0;
  if((b.y + b.h) <= a.y)return 0;
  return 1;
}

SDL_Rect GetD_RegionDimByDir(SDL_Rect r1,int dir)
{
  SDL_Rect r2 = {0,0,3,3};
  r2.w = RMINX + abs(rand_int(RMAXX - RMINX));
  r2.h = RMINY + abs(rand_int(RMAXY - RMINY));
  switch(dir)
  {
    case D_N:
      r2.x = (r1.x + r1.w/2) - r2.w/2;
      r2.y = (r1.y - 1) - r2.h;
      break;
    case D_S:
      r2.x = (r1.x + r1.w/2) - r2.w/2;
      r2.y = (r1.y + r1.h + 1);
      break;
    case D_E:
      r2.y = (r1.y + r1.h/2) - r2.h/2;
      r2.x = (r1.x + r1.w + 1);
      break;
    case D_W:
      r2.y = (r1.y + r1.h/2) - r2.h/2;
      r2.x = (r1.x - 1) - r2.w;
      break;
  }
  return r2;
}

SDL_Point GetD_RegionExitByDir(SDL_Rect r,int dir)
{
  SDL_Point p = {0,0};
  switch(dir)
  {
    case D_N:
      p.y = r.y - 1;
      p.x = r.x + r.w/2;
      break;
    case D_S:
      p.y = r.y + r.h;
      p.x = r.x + r.w/2;
      break;
    case D_E:
      p.x = r.x + r.w;
      p.y = r.y + r.h/2;
      break;
    case D_W:
      p.x = r.x - 1;
      p.y = r.y + r.h/2;
      break;
  }
  return p;
}

char *GetDirName(int i)
{
  switch(i)
  {
    case D_N:
      return "North";
    case D_S:
      return "South";
    case D_E:
      return "East";
    case D_W:
      return "West";
  }
  return NULL;
}

int GetDirRectToRect(SDL_Rect r1, SDL_Rect r2)
{
  SDL_Point p1,p2;
  p1.x = r1.x + r1.w/2;
  p1.y = r1.y + r1.h/2;
  p2.x = r2.x + r2.w/2;
  p2.y = r2.y + r2.h/2;
  if(p1.x < p2.x)return D_E;
  if(p1.x > p2.x)return D_W;
  if(p1.y < p2.y)return D_S;
  if(p1.y > p2.y)return D_N;
  else return D_NONE;
}

int GetDirRectToPoint(SDL_Rect r,SDL_Point p)
{
  if(p.x < r.x)return D_W;
  if(p.y < r.y)return D_N;
  if(p.x >= r.x + r.w) return D_E;
  if(p.y >= r.y + r.h) return D_S;
  else return D_NONE;
}

SDL_Point MkPoint(int x,int y)
{
  SDL_Point p;
  p.x = x;
  p.y = y;
  return p;
}

SDL_Rect Rect(int x, int y, int w, int h)
{
  SDL_Rect r;
  r.x = x;
  r.y = y;
  r.w = w;
  r.h = h;
  return r;
}

int D_RegionsLinked(D_Region *r1, D_Region *r2)
{
  int i,j;
  for(i = 0;i < 4;i++)
  {
    if(r1->exitsused[i] != 0)
    {
      for(j = 0;j < 4;j++)
      {
        if(r2->exitsused[j] != 0)
        {
          if(r1->exitreg[i] == r2->exitreg[j])return 1;
        }
      }
    }
  }
  return 0;
}

int GetCommonPoint(D_Region *r1, D_Region *r2, SDL_Point *p)
{
  int dir;
  int l,b;
  SDL_Rect h;   /*overlap area*/
  if((r1 == NULL)||(r2 == NULL))return -1;
  if(!RectOverlap(Rect(r1->r.x - 1,r1->r.y - 1,r1->r.w + 2,r1->r.h + 2), Rect(r2->r.x - 1,r2->r.y - 1,r2->r.w + 2,r2->r.h + 2)))return 0;
  /*at this point,we know they are not overlapping and are only off by 1 tile*/
  dir = GetDirRectToRect(r1->r,r2->r);
  switch(dir)
  {
    case D_N:
      h.x = MAX(r1->r.x,r2->r.x);
      l = MIN((r1->r.x + r1->r.w),(r2->r.x + r2->r.w));
      h.w = l - h.x - 1;
      h.h = 0;
      h.y = r1->r.y - 1;
      break;
    case D_S:
      h.x = MAX(r1->r.x,r2->r.x);
      l = MIN((r1->r.x + r1->r.w),(r2->r.x + r2->r.w));
      h.w = l - h.x - 1;
      h.h = 0;
      h.y = r2->r.y - 1;  
      break;
    case D_E:
      h.y = MAX(r1->r.y,r2->r.y);
      b = MIN((r1->r.y + r1->r.h),(r2->r.y + r2->r.h));
      h.h = b - h.y - 1;
      h.w = 0;
      h.x = r2->r.x - 1;
      break;
    case D_W:
      h.y = MAX(r1->r.y,r2->r.y);
      b = MIN((r1->r.y + r1->r.h),(r2->r.y + r2->r.h));
      h.h = b - h.y - 1;
      h.w = 0;
      h.x = r1->r.x - 1;
      break;
    default:
      return 0;
  }
  if((h.w < 0)||(h.h < 0))return 0;
  if(p != NULL)
  {
    p->x = h.x + (int)floor(h.w*0.5);
    p->y = h.y + (int)floor(h.h*0.5);
  }
  return 1;
}

int RelationDistance(D_Region *r1, D_Region *r2)
{
  /*figur out distance in hierarchy from each other*/
  int step1 = 0,step2 = 0;
  D_Region *c1,*c2;
  if((r1 == NULL)||(r2 == NULL))return -1;
  c1 = r1;
  c2 = r2;
  while(c2 != NULL)
  {
    step1 = 0;
    while(c1 != NULL)
    {
      if(c1 == c2)
      {
        return (step1 + step2);
      }
      step1++;
      c1 = c1->parent;
    }
    step2++;
    c2 = c2->parent;
  }
  return -1;
}

int rand_int(int i)
{
  float a;
  a = random() * i * 10000;
  return ((int)a % i);
}

/*eol@eof*/

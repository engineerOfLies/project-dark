#include "gtrace.h"
#include "c_collide.h"
#include "levelmesh.h"

extern Level level;
extern LevelMesh lmesh;

int CanWalkTo(Coord s, Coord G,float radius,Entity *self)
{
  Trace t;
  Coord v;
  VectorSubtract(G,s,v);
  if(LevelTrace(s,v, radius, 0, 0, NULL))return 0;
  if(EntTrace(s,v, radius, F_SOLID, &t, self))
  {
    if(self->target != NULL)
    {
      if(t.other != self->target)return 0;
    }
    else return 0;
  }
  if(BoundTrace(s,v, radius, 0, NULL))return 0;
  return 1;
}

float AngleBetweenVectors2D(Coord v1,Coord v2)
{
  float theta;
  Normalize (&v1);
  Normalize(&v2);
  theta  = acos(v1.x * v2.x + v1.y * v2.y);
  theta *= RadToDeg;
  return theta;
}

int LookingTowardsPoint(Coord p, Coord v, Coord t,float range)
{
  Coord v2;
  float theta;
  Normalize (&v);
  v2.z = 0;
  v2.x = t.x - p.x;
  v2.y = t.y - p.y;
  Normalize(&v2);
  theta  = acos(v.x * v2.x + v.y * v2.y);
  theta *= RadToDeg;
  if(theta < range)return 1;
  return 0;
}

int MovingTowardsPoint(Coord p, Coord v, Coord t)
{
  return LookingTowardsPoint(p,v,t,90);
}

int ClipmaskCheck(int mask,Entity *e)
{
  if(e == NULL)return 0;
  if((mask & F_NODEAD)&&(e->state == MS_Deadbody))return 0;
  if((mask & F_SOLID)&&(!e->solid))return 0;
  if((mask & F_SIGHT)&&(!e->sightblock))return 0;
  if((mask & F_TDAMAGE)&&(!e->takesdamage))return 0;
  return 1;
}

/*traces vs entities*/
int EntTrace(Coord s,Coord v, float radius, int clipmask, Trace *trace, Entity *self)
{
  Entity *ent = NULL;
  Entity *hit = NULL;
  Coord n;
  Point2D out;
  ent = GetNextEntity(NULL,self);
  while(ent != NULL)
  {
    if(ent != self->owner)
    {
      if(ClipmaskCheck(clipmask,ent))
      {
        if(MovingTowardsPoint(s,v,ent->p))
        {
          if(PointHitCircle(M_Point(s.x, s.y), M_Point(v.x,v.y), M_Point(ent->p.x,ent->p.y), radius + ent->radius, &out))
          {
            v.x = out.p[0] - s.x;
            v.y = out.p[1] - s.y;
            n.x = out.p[0] - ent->p.x;
            n.y = out.p[0] - ent->p.y;
            Normalize(&n);
  /*          v.x += (n.x * FUDGE);
            v.y += (n.y * FUDGE);*/
            hit = ent;
          }
        }
      }
    }
    ent = GetNextEntity(ent,self);
  }
  if(hit != NULL)
  {
    if(trace != NULL)
    {
      trace->normal.x = n.x;
      trace->normal.y = n.y;
      trace->POC.x = out.p[0];
      trace->POC.y = out.p[1];
      trace->FV.x = v.x;
      trace->FV.y = v.y;
      n.x *= EPSILON;
      n.y *= EPSILON;
      trace->FV.x += n.x;
      trace->FV.y += n.y;
      trace->POC.x += n.x;
      trace->POC.y += n.y;
      trace->hit = 1;
      trace->hittype = 1;
      trace->other = hit;
    }
    return 1;
  }
  return 0;
}

int MovingTowardsEdge(LEdge edge,Coord s,Coord v, float radius)
{/*
  Edge2D out;
  Coord n;
  float angle;*/
  /*If the Angle between the vector and either endpoint is acute, then yes, we are moving towards it.*/
  /*CORRECTION if the angle between the normal and the vector is obtuse, then we ar emoving towards!*/
  /*GetClipEdge(M_Edge(edge.p[0].x,edge.p[0].y,edge.p[1].x,edge.p[1].y), s, radius, &out,&n);
  angle = AngleBetweenVectors2D(n,v);
  fprintf(stdout,"angle between edge normal and vector: %f\n",angle);
  if(angle > 90)return 1;
 */ 
  if(MovingTowardsPoint(s,v,M_Coord(edge.p[0].x * TILEW,edge.p[0].y * TILEH,0)))return 1;
  if(MovingTowardsPoint(s,v,M_Coord(edge.p[1].x * TILEW,edge.p[1].y * TILEH,0)))return 1;
 
  return 0;
}

int EdgeInRange(LEdge edge,Coord s,Coord v, float radius)
{
  int minx,miny;
  int maxx,maxy;
  minx = MIN(s.x,s.x + v.x) - radius - FUDGE;
  miny = MIN(s.y,s.y + v.y) - radius - FUDGE;
  maxx = MAX(s.x,s.x + v.x) + radius + FUDGE;
  maxy = MAX(s.y,s.y + v.y) + radius + FUDGE;
  if(((edge.p[0].x * TILEW) < minx)&&((edge.p[1].x * TILEW) < minx))return 0;
  if(((edge.p[0].x * TILEW) > maxx)&&((edge.p[1].x * TILEW) > maxx))return 0;
  if(((edge.p[0].y * TILEH) < miny)&&((edge.p[1].y * TILEH) < miny))return 0;
  if(((edge.p[0].y * TILEH) > maxy)&&((edge.p[1].y * TILEH) > maxy))return 0;
  return 1;
}

int EdgeFromTile(LEdge *edge,int tile)
{
  /*function checks to seee if this edge belongs to the tile*/
  int x,y;
  x = tile % level.w;
  y = tile / level.h;
  if((tile < 0)||(tile >= level.numtiles))return 0;
  if(((edge->p[0].x == x)&&(edge->p[0].y == y)&&(edge->p[1].x == x + 1)&&(edge->p[1].y == y))||
    ((edge->p[0].x == x)&&(edge->p[0].y == y)&&(edge->p[1].x == x)&&(edge->p[1].y == y + 1))||
    ((edge->p[0].x == x + 1)&&(edge->p[0].y == y)&&(edge->p[1].x == x + 1)&&(edge->p[1].y == y + 1))||
    ((edge->p[0].x == x)&&(edge->p[0].y == y + 1)&&(edge->p[1].x == x + 1)&&(edge->p[1].y == y + 1)))return 1;
  if(((edge->p[1].x == x)&&(edge->p[1].y == y)&&(edge->p[0].x == x + 1)&&(edge->p[0].y == y))||
    ((edge->p[1].x == x)&&(edge->p[1].y == y)&&(edge->p[0].x == x)&&(edge->p[0].y == y + 1))||
    ((edge->p[1].x == x + 1)&&(edge->p[1].y == y)&&(edge->p[0].x == x + 1)&&(edge->p[0].y == y + 1))||
    ((edge->p[1].x == x)&&(edge->p[1].y == y + 1)&&(edge->p[0].x == x + 1)&&(edge->p[0].y == y + 1)))return 1;
  return 0;
}

int NewLevelTrace(Coord s,Coord v, float radius, int clipmask, int ignore, Trace *trace)
{
  cpSegmentQueryInfo info;
  Coord fn;
  int i;
  int count = GetLevelShapeCount();
  cpSegmentShape *shape;
  for(i = 0;i < count;i++)
  {
    shape = (cpSegmentShape *)GetLevelShapeByIndex(i);
    if(shape != NULL)
    {
      shape->r = radius;
    }
  }
  shape = (cpSegmentShape *)cpSpaceSegmentQueryFirst(GetLevelSpace(), cpv(s.x,s.y), cpv(s.x + v.x, s.y + v.y), CP_ALL_LAYERS, CP_NO_GROUP,&info);
  if(shape == NULL)
  {
    return 0;
  }
  if(trace != NULL)
  {
    trace->normal.x = info.n.x;
    trace->normal.y = info.n.y;
    info.n.x *= 0.0000001;
    info.n.y *= 0.0000001;
    fn.x = v.x;
    fn.y = v.y;
    Normalize(&fn);
    VectorScale(fn,FUDGE,fn);
    VectorScale(v,info.t,v);
    trace->POC.x = s.x + v.x - fn.x + info.n.x;
    trace->POC.y = s.y + v.y - fn.y + info.n.y;
    trace->FV.x = v.x - fn.y + info.n.x;
    trace->FV.y = v.y - fn.y + info.n.y;
    trace->hit = 1;
    trace->hittype = 0;
  }
  return 1;
}

int LevelTrace(Coord s,Coord v, float radius, int clipmask, int ignore, Trace *trace)
{
  LEdge *edge = NULL;   /*edge index*/
  Coord n;    /*normal of collision*/
  Coord bn;
  Coord fv;   /*fudge vector*/
  Point2D out;
  float bestdist = -1;
  float testdist;
  Point2D best;
  int hit = 0;
  VectorCopy(v,fv);
  Normalize (&fv);
  VectorScale(fv,FUDGE,fv);
  fv.x *= -1;
  fv.y *= -1;
  for(edge = lmesh.edge;edge != NULL;edge = edge->next)
  {
    if((edge->usecount == 1)&&(!EdgeFromTile(edge,ignore)))
    { /*only check outside edges.  Interior edges can be ignored*/
      if((EdgeInRange(*edge,s,v,radius + FUDGE))&&(MovingTowardsEdge(*edge,s,v, radius)))
      {
        /*ignore edges that are outside the realm of possible*/
        if(CircleHitEdge(M_Point(s.x,s.y),radius,M_Point(v.x,v.y),M_Edge(edge->p[0].x * TILEW, edge->p[0].y * TILEH, edge->p[1].x * TILEW, edge->p[1].y * TILEH), &out,&n))
        {
          hit = 1;
          if(bestdist == -1)
          {
            best.p[0] = out.p[0];
            best.p[1] = out.p[1];
            bn.x = n.x;
            bn.y = n.y;
            bestdist = ((s.x - out.p[0])*(s.x - out.p[0])) + ((s.y - out.p[1])*(s.y - out.p[1]));
          }
          else
          {
            testdist = ((s.x - out.p[0])*(s.x - out.p[0])) + ((s.y - out.p[1])*(s.y - out.p[1]));
            if(testdist < bestdist)
            {
              best.p[0] = out.p[0];
              best.p[1] = out.p[1];
              bn.x = n.x;
              bn.y = n.y;
              bestdist = testdist;
            }
          }
          v.x = best.p[0] - s.x;
          v.y = best.p[1] - s.y;
          v.x += bn.x;
          v.y += bn.y;
          if(trace != NULL)
          {
            bn.x *= FUDGE;
            bn.y *= FUDGE;
            trace->POC.x = best.p[0];
            trace->POC.y = best.p[1];
            trace->FV.x = best.p[0] - s.x;
            trace->FV.y = best.p[1] - s.y;
            trace->FV.x += bn.x;
            trace->FV.y += bn.y;
            trace->POC.x += bn.x;
            trace->POC.y += bn.y;
            trace->hit = 1;
            trace->hittype = 0;
          }
        }
      }
    }
  }
  return hit;
}

int BoundTrace(Coord s,Coord v, float radius, int clipmask, Trace *trace)
{
  int minx,miny;
  int maxx,maxy;
  int xhit = 0;
  int yhit = 0;
  float tx,ty;
  minx = MIN(s.x,s.x + v.x);
  miny = MIN(s.y,s.y + v.y);
  maxx = MAX(s.x,s.x + v.x);
  maxy = MAX(s.y,s.y + v.y);
  if(v.x < 0)
  {
    if(minx < radius)
    {
      xhit = 1;
      tx = radius;
    }
  }
  else if(v.x > 0)
  {
    if(maxx >= (level.w * TILEW ) - radius)
    {
      xhit = 1;
      tx = (level.w * TILEW ) - radius;
    }
  }
  if(!xhit)
  {
    tx = s.x + v.x;
  }
  if(v.y < 0)
  {
    if(miny < radius)
    {
      yhit = 1;
      ty = radius;
    }
  }
  else if(v.y > 0)
  {
    if(maxy >= (level.h * TILEH ) - radius)
    {
      yhit = 1;
      ty = (level.h * TILEH ) - radius;
    }
  }
  if(!yhit)
  {
    ty = s.y + v.y;
  }
  if(((xhit)||(yhit))&&(trace != NULL))
  {
    trace->POC.x = tx;
    trace->POC.y = ty;
    trace->FV.x = tx - s.x;
    trace->FV.y = ty - s.y;
    trace->hit = 1;
    trace->hittype = 2;
  }
  return xhit|yhit;
}

int GTrace(Coord s,Coord v, float radius, int clipmask, Trace *trace, Entity *self)
{
  int hit;
  int passhit;
  int endhit = 0;
  int tries = 0;
  Coord ov;
  Coord nn = {0,0,0};
  VectorCopy(v,ov);
  if(trace)memset(trace,0,sizeof(Trace));
  do
  {
    passhit = 0;
    hit = EntTrace(s,v,radius,clipmask, trace,self);
    if(hit)
    {
      if(trace != NULL)
      {
        v.x = trace->FV.x;
        v.y = trace->FV.y;
        nn.x += trace->normal.x;
        nn.y += trace->normal.y;
      }
    }
    endhit |= hit;
    passhit |= hit;
    hit = LevelTrace(s,v,radius,clipmask,-1, trace);
    if(hit)
    {
      hit = 2;
      if(trace != NULL)
      {
        v.x = trace->FV.x;
        v.y = trace->FV.y;
        nn.x += trace->normal.x;
        nn.y += trace->normal.y;
      }
    }
    endhit |= hit;
    passhit |= hit;
    hit = BoundTrace(s,v, radius,clipmask,trace);
    if(hit)
    {
      if(trace != NULL)
      {
        v.x = trace->FV.x;
        v.y = trace->FV.y;
      }
    }
    tries++;
    endhit |= hit;
    passhit |= hit;
  }while((passhit)&&(tries < TRACESTRENGTH));
  if(tries == TRACESTRENGTH)
  {
    if(trace != NULL)
    {
      v.x = trace->FV.x = 0;
      v.y = trace->FV.y = 0;
      trace->POC.x = s.x;
      trace->POC.y = s.y;
    }
  }
  if(trace)
  {
    if(endhit)
    {
      trace->FV.x += nn.x * EPSILON;
      trace->FV.y += nn.y * EPSILON;
      trace->POC.x += nn.x * EPSILON;
      trace->POC.y += nn.y * EPSILON;
    }
  }
  return endhit;
}

int BackupGTrace(Coord s,Coord v, float radius, int clipmask, Trace *trace, Entity *self)
{
  int hit;
  int endhit = 0;
  int tries = 0;
  do
  {
    hit = EntTrace(s,v,radius,clipmask, trace,self);
    if(hit)
    {
      if(trace != NULL)
      {
        v.x = trace->FV.x;
        v.y = trace->FV.y;
      }
    }
    endhit |= hit;
    tries++;
  }while((hit)&&(tries < TRACESTRENGTH));
  tries = 0;
  do
  {
    hit = LevelTrace(s,v,radius,clipmask,-1, trace);
    if(hit)
    {
      hit = 2;
      if(trace != NULL)
      {
        v.x = trace->FV.x;
        v.y = trace->FV.y;
      }
    }
    tries++;
    endhit |= hit;
  }while((hit)&&(tries < TRACESTRENGTH));
  tries = 0;
  do
  {
    hit = BoundTrace(s,v, radius,clipmask,trace);
    if(hit)
    {
      if(trace != NULL)
      {
        v.x = trace->FV.x;
        v.y = trace->FV.y;
      }
    }
    tries++;
    endhit |= hit;
  }while((hit)&&(tries < TRACESTRENGTH));
  return endhit;
}

/*calls normal GTrace, after calculating the vector from s to g.*/
int GTracePtP(Coord s,Coord g, float radius, int clipmask, Trace *trace, Entity *self)
{
  Coord v = {0,0,0};
  v.x = g.x - s.x;
  v.y = g.y - s.y;
  return GTrace(s, v, radius, clipmask, trace, self);
}


/*eol@eof*/

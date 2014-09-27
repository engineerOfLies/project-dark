#include "c_collide.h"

zfmModel *levelmask = NULL;
zfmModel *levelbounds = NULL;
extern BBox bounds;

void ReflectVector(Coord v, Coord n,Coord *out)
{
  float f;
  
  f = DotProduct(v,n);
  out->x = v.x - (2 * n.x * f);
  out->y = v.y - (2 * n.y * f);
  out->z = v.z - (2 * n.z * f);
}

int MoveEntEntityClipMask(Entity *ent,zfmModel *mask,Point2D *p,Point2D *nv)
{
  float length,distance;
  Edge2D E,B_edge;
  Point2D P, B,v;
  float minx, miny, maxx, maxy;
  face3i *face = NULL;
  int edge = 0;
  int best = -1;
  if(p == NULL)return 0;
  if(mask == NULL)
  {
    p->p[0] = ent->p.x + ent->v.x;
    p->p[1] = ent->p.y + ent->v.y;
    return 0;
  }
  if(ent->v.x > 0)
  {
    minx = ent->p.x - ent->box.x;
    maxx = ent->p.x - ent->box.x + ent->box.w + ent->v.x;
  }
  else if(ent->v.x < 0)
  {
    minx = ent->p.x - ent->box.x + ent->v.x;
    maxx = ent->p.x - ent->box.x + ent->box.w;
  }
  else
  {
    minx = ent->p.x - ent->box.x;
    maxx = ent->p.x - ent->box.x + ent->box.w;
  }
  if(ent->v.y > 0)
  {
    miny = ent->p.y - ent->box.y;
    maxy = ent->p.y - ent->box.y + ent->box.h + ent->v.y;
  }
  else if(ent->v.y < 0)
  {
    miny = ent->p.y - ent->box.y + ent->v.y;
    maxy = ent->p.y - ent->box.y + ent->box.h;
  }
  else
  {
    miny = ent->p.y - ent->box.y;
    maxy = ent->p.y - ent->box.y + ent->box.h;
  }
  do
  {
    
    face = GetNextOutSideEdge(face, &edge, mask, 0, 0, minx, maxx, miny, maxy);
    if(face != NULL)
    { /*check to see if this edge is worth checking*/
      switch(edge)
      {
        case 1:
          E.p1[0] = levelmask->verticesAtRest[face->vert1[0]].vert[0];
          E.p1[1] = levelmask->verticesAtRest[face->vert1[0]].vert[1];
          E.p2[0] = levelmask->verticesAtRest[face->vert2[0]].vert[0];
          E.p2[1] = levelmask->verticesAtRest[face->vert2[0]].vert[1];
          break;
        case 2:
          E.p1[0] = levelmask->verticesAtRest[face->vert3[0]].vert[0];
          E.p1[1] = levelmask->verticesAtRest[face->vert3[0]].vert[1];
          E.p2[0] = levelmask->verticesAtRest[face->vert2[0]].vert[0];
          E.p2[1] = levelmask->verticesAtRest[face->vert2[0]].vert[1];
          break;
        case 3:
          E.p1[0] = levelmask->verticesAtRest[face->vert1[0]].vert[0];
          E.p1[1] = levelmask->verticesAtRest[face->vert1[0]].vert[1];
          E.p2[0] = levelmask->verticesAtRest[face->vert3[0]].vert[0];
          E.p2[1] = levelmask->verticesAtRest[face->vert3[0]].vert[1];
          break;
      }
      if(CircleHitEdge(M_Point(ent->p.x,ent->p.y),ent->box.x,M_Point(ent->v.x,ent->v.y),E, &P,NULL))
      {
        if(best == -1)
        {
          memcpy(&B_edge,&E,sizeof(Edge2D));
          B.p[0] = P.p[0];
          B.p[1] = P.p[1];
          best = 1;
        }
        else
        {
          if(RelativeSize(P.p[0] - ent->p.x, P.p[1] - ent->p.y) < RelativeSize(B.p[0] - ent->p.x, B.p[1] - ent->p.y))
          {
            B.p[0] = P.p[0];
            B.p[1] = P.p[1];
            memcpy(&B_edge,&E,sizeof(Edge2D));
          }
        }
      }
    }
  }while(face != NULL);
  if(best == 1)
  {
    length =  PointLength(ent->v.x, ent->v.y);  /*the distance we want to move*/
    distance =  PointLength(B.p[0] - ent->p.x, B.p[1] - ent->p.y);/*the shortest distance we can safely move*/
    if(distance < length)
    {
      GetEdgeNormal(B_edge,*p,nv);
      p->p[0] = B.p[0];
      p->p[1] = B.p[1];
      v = M_Point(ent->v.x,ent->v.y);
      Normalize2D(&v.p[0],&v.p[1]);
      v.p[0] *= -0.55;
      v.p[1] *= -0.55;
      p->p[0] += v.p[0];
      p->p[1] += v.p[1];
      return 1;
    }
  }
  p->p[0] = ent->p.x + ent->v.x;
  p->p[1] = ent->p.y + ent->v.y;
  return 0;
}


int MoveEntityClippingEnts(Entity *ent,Entity **other)
{
  Entity *test = NULL;
  Point2D contact;
  if(other != NULL)*other = NULL;
  while((test = GetNextEntity(test,ent)) != NULL)
  {
    if((test->solid)&&(test->team != ent->team))
    {
      if(PointHitCircle(M_Point(ent->p.x,ent->p.y), M_Point(ent->v.x,ent->v.y), M_Point(test->p.x,test->p.y), ent->box.x + test->box.x, &contact))
      {
        ent->v.x = contact.p[0] - ent->p.x;
        ent->v.y = contact.p[1] - ent->p.y;
        if(other != NULL)*other = test;
        return H_ENTITY;
      }
    }
  }
  return 0;
}

int UpdateEntityPosition(Entity *ent,Trace *rtrace)
{
  Trace mtrace;
  Trace *trace;
  int hit = 0;
  if(ent == NULL)return 0;
  if(rtrace != NULL)
  {
    trace = rtrace;
  }
  else trace = &mtrace;
  hit = GTrace(ent->p,ent->v, ent->radius, 1, trace,ent);
  if(hit)
  {
    ent->p.x = trace->POC.x;
    ent->p.y = trace->POC.y;
    ent->v.x = trace->FV.x;
    ent->v.y = trace->FV.y;
    if(trace->hittype == HT_Entity)
    {
      if(trace->other != NULL)
      {
        if(trace->other->touch != NULL)
        {
          trace->other->touch(trace->other,ent);
        }
        if(ent->touch != NULL)
        {
          ent->touch(ent,trace->other);
        }
      }
    }
  }
  else
  {
    ent->p.x += ent->v.x;
    ent->p.y += ent->v.y;
  }
  return hit;
}

/*
  Given the point withthe radius, moving the direction v, at what point do we hit the edge
*/
int CircleHitEdge(Point2D p,float rad,Point2D v,Edge2D e, Point2D *out, Coord *n)
{
  Edge2D e1;   /*the boudning edges for the edge in question*/
  Point2D Contact;
  Coord norm;
  Coord fv;
  int  hit[] = {0,0,0,0};
  norm.z = 0;
  fv.x = v.p[0] * -1;
  fv.y = v.p[1] * -1;
  Normalize(&fv);
  fv.x *= FUDGE;
  fv.y *= FUDGE;
  GetClipEdge(e, M_Coord(p.p[0],p.p[1],0), rad, &e1,&norm);
  /*GetEdgeBounding(e, rad, &e1, &e2);*/
  /*I realized that since all of the masks are connected, I don't need to check both end points*/
  hit[0] = PointHitCircle(p, v, M_Point(e.p1[0],e.p1[1]), rad, &Contact);
  if(hit[0])
  {
    v.p[0] = Contact.p[0] - p.p[0];
    v.p[1] = Contact.p[1] - p.p[1];
    v.p[0] += fv.x;
    v.p[1] += fv.y;
    if(n != NULL)
    {
      n->x = p.p[0] - e.p1[0];
      n->y = p.p[1] - e.p1[1];
      Normalize(n);
      #if 0
      v.p[0] += (n->x/* * FUDGE*/);
      v.p[1] += (n->y/* * FUDGE*/);
      #endif
    }
  }
  hit[1] =  PointHitCircle(p, v, M_Point(e.p2[0],e.p2[1]), rad, &Contact);
  if(hit[1])
  {
    v.p[0] = Contact.p[0] - p.p[0];
    v.p[1] = Contact.p[1] - p.p[1];
    v.p[0] += fv.x;
    v.p[1] += fv.y;
    if(n != NULL)
    {
      n->x = p.p[0] - e.p2[0];
      n->y = p.p[1] - e.p2[1];
      Normalize(n);
    }
  }
  hit[2] = RayCrossEdge(p.p[0],p.p[1],v.p[0], v.p[1],e1.p1[0], e1.p1[1], e1.p2[0],e1.p2[1], &Contact.p[0], &Contact.p[1]);
  if(hit[2])
  {
    v.p[0] = Contact.p[0] - p.p[0];
    v.p[1] = Contact.p[1] - p.p[1];
    v.p[0] += fv.x;
    v.p[1] += fv.y;
    if(n != NULL)
    {
      n->x = norm.x;
      n->y = norm.y;
      #if 0
      v.p[0] += (n->x/* * FUDGE*/);
      v.p[1] += (n->y/* * FUDGE*/);
      #endif
    }
  }
  if(hit[0]|hit[1]|hit[2]|hit[3])
  {
    if(out != NULL)
    {
      out->p[0] = Contact.p[0];
      out->p[1] = Contact.p[1];
      out->p[0] += fv.x;
      out->p[1] += fv.y;
      if(n != NULL)
      {
        /*
        out->p[0] += (n->x * FUDGE);
        out->p[1] += (n->y * FUDGE);*/
      }
    }
    return 1;
  }
  return 0;
}

/*alternate version of PointHitCircle*/
int PointHitCircle(Point2D p, Point2D v, Point2D o, float rad, Point2D *out)
{
  float disc;
  float a,b,c;
  float t1,t2;
  float inner;
  int t1hit = 0,t2hit = 0;
  Point2D t1out,t2out;
  Point2D f;
  if(out == NULL)return -1;
  f.p[0] = p.p[0] - o.p[0];
  f.p[1] = p.p[1] - o.p[1];
  a = dot2D(v,v );
  b = 2 * dot2D(f,v);
  c = dot2D(f,f) - rad*rad ;
  disc = b*b-4*a*c;
  if(disc <= 0)return 0;
  if(a == 0)return 0;
  disc = sqrt(disc);
  t1 = (-b + disc)/(2*a);
  t2 = (-b - disc)/(2*a);
  if((t1 > 1) && (t2 > 1))
  {
    /*both fell shy*/
    return 0;
  }
  if((t1 < 0)||(t2 < 0))
  {
    /*This should never happen, we are IN the circle*/
    /*but for some reason does and should be accounted for.*/
    f.p[0] *= -1;
    f.p[1] *= -1;
    inner = sqrt((f.p[0] * f.p[0])+(f.p[1] * f.p[1]));
    Normalize2D(&f.p[0],&f.p[1]);
    f.p[0] *= (rad - inner)/* + FUDGE*/;
    f.p[1] *= (rad - inner)/* + FUDGE*/;
    out->p[0] = p.p[0] + f.p[0];
    out->p[1] = p.p[1] + f.p[1];
    return 1;
  }
  if( t1 >= 0 && t1 <= 1 )
  {
    /*hit on one part of the circle*/
    t1out.p[0] = p.p[0] + (v.p[0] * t1);
    t1out.p[1] = p.p[1] + (v.p[1] * t1);
    t1hit = 1;
  }
  if( t2 >= 0 && t2 <= 1 )
  {
    /*hit on one part of the circle*/
    t2out.p[0] = p.p[0] + (v.p[0] * t2);
    t2out.p[1] = p.p[1] + (v.p[1] * t2);
    t2hit = 1;
  }
  if(t2hit && t1hit)
  {
    if(t1 < t2)
    {
      out->p[0] = t1out.p[0];
      out->p[1] = t1out.p[1];
      return 1;
    }
    else
    {
      out->p[0] = t2out.p[0];
      out->p[1] = t2out.p[1];
      return 1;
    }
  }
  if(t2hit)
  {
    out->p[0] = t2out.p[0];
    out->p[1] = t2out.p[1];
    return 1;
  }
  else
  {
    out->p[0] = t1out.p[0];
    out->p[1] = t1out.p[1];
    return 1;
  }
}

/*find the point where p will interesect c as it moves along v*/
int PointHitCircle1(Point2D p, Point2D v, Point2D c, float rad, Point2D *out)
{
  Point2D s;
  float b,a,z;
  float L1,L2;  /*length along the vector where the points hit*/
  float Lpart1,Lpart2;    /*components of the math to make L1 and L2 don't need to do the math twice*/
  float disc;
  Normalize2D(&v.p[0],&v.p[1]);
  s.p[0] = p.p[0] - c.p[0];
  s.p[1] = p.p[1] - c.p[1];
  b = 2 * dot2D(s,v);
  a = dot2D(v,v);
  z = dot2D(s,s) - (rad*rad);
  disc = (b * b) - (4 * a * z);
  if(disc < 0)
    return 0;
  if(a == 0)return 0;
  Lpart1 = -(dot2D(s, v)/a);
  Lpart2 = sqrt(((rad * rad)-(dot2D(s,s))/a) + ((dot2D(s,v)/a)*(dot2D(s,v)/a)));
  L1 = Lpart1 + Lpart2;
  L2 = Lpart1 - Lpart2;
  if((L1 < 0)&&(L2 > 0))
  { /*then we are past this point so go with it.*/
    out->p[0] = p.p[0] + (v.p[0]*L1);
    out->p[1] = p.p[1] + (v.p[1]*L1);
    return 1;
  }
  if((L2 < 0)&&(L1 > 0))
  {
    out->p[0] = p.p[0] + (v.p[0]*L2);
    out->p[1] = p.p[1] + (v.p[1]*L2);
    return 1;
  }
  if(fabs(L1) < fabs(L2))
  {
    out->p[0] = p.p[0] + (v.p[0]*L1);
    out->p[1] = p.p[1] + (v.p[1]*L1);
  }
  else
  {
    out->p[0] = p.p[0] + (v.p[0]*L2);
    out->p[1] = p.p[1] + (v.p[1]*L2);
  }
  return 1;
}

/*This Version attempts to get THE edge  that is closest to the point in question*/
void GetClipEdge(Edge2D in, Coord p, float rad, Edge2D *out,Coord *norm)
{
  Coord n;
  Coord t1,t2;      /*test points*/
  Coord mp = {0,0,0};
  float d1,d2;
  if((out == NULL)&&(norm == NULL))return;/*nothing to do then, why waste cycles*/
  n.x = in.p1[1] - in.p2[1];
  n.y = in.p2[0] - in.p1[0];
  n.z = 0;
  Normalize(&n);
  mp.x = (in.p1[0] + in.p2[0])/2;
  mp.y = (in.p1[1] + in.p2[1])/2;
  t1.x = mp.x + n.x;
  t1.y = mp.y + n.y;
  t2.x = mp.x - n.x;
  t2.y = mp.y - n.y;
  d1 = RelativeSize(t1.x - p.x, t1.y - p.y);
  d2 = RelativeSize(t2.x - p.x, t2.y - p.y);
  if(d1 > d2)
  {
    n.x *= -1;
    n.y *= -1;
  }
  if(norm != NULL)
  {
    norm->x = n.x;
    norm->y = n.y;
  }
  if(out != NULL)
  {
    out->p1[0] = in.p1[0] + (n.x * rad);
    out->p1[1] = in.p1[1] + (n.y * rad);
    out->p2[0] = in.p2[0] + (n.x * rad);
    out->p2[1] = in.p2[1] + (n.y * rad);
  }
}

/*given an edge and a radius, it will return the two other edges bounding it*/
void GetEdgeBoundingOld(Edge2D in, float rad, Edge2D *out1, Edge2D *out2)
{
  Point2D p;    /*the vector perpendicular to the edge.*/
  if((out1 == NULL)||(out2 == NULL))return;
  p.p[0] = in.p1[1] - in.p2[1];
  p.p[1] = in.p2[0] - in.p1[0];
  Normalize2D(&p.p[0], &p.p[1]);
  p.p[0] *= rad;
  p.p[1] *= rad;
  out1->p1[0] = in.p1[0] + p.p[0];
  out1->p1[1] = in.p1[1] + p.p[1];
  out1->p2[0] = in.p2[0] + p.p[0];
  out1->p2[1] = in.p2[1] + p.p[1];
  out2->p1[0] = in.p1[0] - p.p[0];
  out2->p1[1] = in.p1[1] - p.p[1];
  out2->p2[0] = in.p2[0] - p.p[0];
  out2->p2[1] = in.p2[1] - p.p[1];
}

void GetEdgeBounding(Edge2D in, float rad, Edge2D *out1, Edge2D *out2)
{
  Point2D p;    /*the vector perpendicular to the edge.*/
  if((out1 == NULL)||(out2 == NULL))return;
  p.p[0] = in.p1[1] - in.p2[1];
  p.p[1] = in.p2[0] - in.p1[0];
  Normalize2D(&p.p[0], &p.p[1]);
  p.p[0] *= rad;
  p.p[1] *= rad;
  out1->p1[0] = in.p1[0] + p.p[0];
  out1->p1[1] = in.p1[1] + p.p[1];
  out1->p2[0] = in.p2[0] + p.p[0];
  out1->p2[1] = in.p2[1] + p.p[1];
  out2->p1[0] = in.p1[0] - p.p[0];
  out2->p1[1] = in.p1[1] - p.p[1];
  out2->p2[0] = in.p2[0] - p.p[0];
  out2->p2[1] = in.p2[1] - p.p[1];
}


/*goes through each face and returns the first time both f1 and f2 are found on the same triangle that is NOT ignore*/
int IsEdgeShared(face3i *ignore,int f1,int f2,zfmModel *model)
{
  int i;
  for(i = 0; i < model->numfaces;i++)
  {
    if(&model->faces[i] == ignore)continue;
    if(((model->faces[i].vert1[0] == f1)||(model->faces[i].vert2[0] == f1)||(model->faces[i].vert3[0] == f1))&&((model->faces[i].vert1[0] == f2)||(model->faces[i].vert2[0] == f2)||(model->faces[i].vert3[0] == f2)))
      return 1;
  }
  return 0;
}


/*if face is NULL, starts with the first face given, returns NULL when all done*/
face3i *GetNextOutSideEdge(face3i *face,int *edge,zfmModel *model,float x,float y,float minx,float maxx,float miny,float maxy)
{
  if(face == NULL)face = &model->faces[0];
  else face++;
  for(;face < &model->faces[model->numfaces - 1];face++)
  {
    if((((model->verticesAtRest[face->vert1[0]].vert[0] + x < minx) && ( model->verticesAtRest[face->vert2[0]].vert[0]  + x < minx))||((model->verticesAtRest[face->vert1[0]].vert[0] + x > maxx)&&(model->verticesAtRest[face->vert2[0]].vert[0] + x > maxx))||((model->verticesAtRest[face->vert1[0]].vert[1] + y > maxy)&&(model->verticesAtRest[face->vert2[0]].vert[1] + y > maxy))||((model->verticesAtRest[face->vert1[0]].vert[1] + y < miny)&&(model->verticesAtRest[face->vert2[0]].vert[1] + y < miny)))==0)
    {/*don't bother doing any complicated math if there is no hope of hitting this edge*/
    if(IsEdgeShared(face,face->vert1[0],face->vert2[0],model) == 0)
    {
      if(edge != NULL)*edge = 1;
      return face;
    }
    }
    if(!(((model->verticesAtRest[face->vert3[0]].vert[0] + x < minx)&&(model->verticesAtRest[face->vert2[0]].vert[0] + x < minx))||((model->verticesAtRest[face->vert3[0]].vert[0] + x > maxx)&&(model->verticesAtRest[face->vert2[0]].vert[0] + x > maxx))||((model->verticesAtRest[face->vert3[0]].vert[1] + y > maxy)&&(model->verticesAtRest[face->vert2[0]].vert[1]  + y > maxy))||((model->verticesAtRest[face->vert3[0]].vert[1] + y < miny)&&(model->verticesAtRest[face->vert2[0]].vert[1] + y < miny))))
    {
      if(IsEdgeShared(face,face->vert3[0],face->vert2[0],model) == 0)
      {
        if(edge != NULL)*edge = 2;
        return face;
      }
    }
    if(!(((model->verticesAtRest[face->vert3[0]].vert[0] + x < minx)&&(model->verticesAtRest[face->vert1[0]].vert[0] + x < minx))||((model->verticesAtRest[face->vert3[0]].vert[0] + x > maxx)&&(model->verticesAtRest[face->vert1[0]].vert[0] + x > maxx))||((model->verticesAtRest[face->vert3[0]].vert[1] + y > maxy)&&(model->verticesAtRest[face->vert1[0]].vert[1] + y > maxy))||((model->verticesAtRest[face->vert3[0]].vert[1] + y < miny)&&(model->verticesAtRest[face->vert1[0]].vert[1] + y < miny))))
    {
      if(IsEdgeShared(face,face->vert1[0],face->vert3[0],model) == 0)
      {
        if(edge != NULL)*edge = 3;
        return face;
      }
    }
  }
  return NULL;
}

void GetEdgeNormal(Edge2D e,Point2D p,Point2D *out)
{
  Point2D N1,N2,C;
  Point2D nv1,nv2;
  C.p[0] = (e.p1[0] + e.p2[0])* 0.5;
  C.p[1] = (e.p1[1] + e.p2[1])* 0.5;/*center point of the line segment*/
  nv1.p[0] = e.p1[1] - e.p2[1];
  nv1.p[1] = e.p2[0] - e.p1[0];/*first possible normal*/
  Normalize2D(&nv1.p[0],&nv1.p[1]);
  nv2.p[0] = e.p2[1] - e.p1[1];
  nv2.p[1] = e.p1[0] - e.p2[0];/*second possible normal*/
  Normalize2D(&nv2.p[0],&nv2.p[1]);
  N1.p[0] = C.p[0] + nv1.p[0];
  N1.p[1] = C.p[1] + nv1.p[1];/*point one unit away in the normal direction from the center point*/
  N2.p[0] = C.p[0] + nv2.p[0];
  N2.p[1] = C.p[1] + nv2.p[1];/*point one unit away in the normal direction from the center point*/
  if(RelativeSize(N1.p[0] - p.p[0],N1.p[1] - p.p[1]) < RelativeSize(N2.p[0] - p.p[0],N2.p[1] - p.p[1]))
  {
    out->p[0] = nv1.p[0];
    out->p[1] = nv1.p[1];
  }
  else
  {
    out->p[0] = nv2.p[0];
    out->p[1] = nv2.p[1];
  }
  Normalize2D(&out->p[0],&out->p[1]);
}

/*utility*/
void Normalize2D(float *vx, float *vy)
{
  double hyp;
  hyp = sqrt((*vx * *vx) + (*vy * *vy));
  hyp = 1 / hyp;
  *vx = (*vx * hyp);
  *vy = (*vy * hyp);
}

Point2D M_Point(float x1, float y1)
{
  Point2D p;
  p.p[0] = x1;
  p.p[1] = y1;
  return p;
}

Edge2D M_Edge(float x1, float y1, float x2, float y2)
{
  Edge2D e;
  e.p1[0] = x1;
  e.p1[1] = y1;
  e.p2[0] = x2;
  e.p2[1] = y2;
  return e;
}


float PointLength(float x, float y)
{
  return sqrt((x * x) + (y * y));
}


float dot2D (Point2D a, Point2D b)
{
  return a.p[0] * b.p[0] + a.p[1] * b.p[1];
}

int  PointInRect(Coord p,float x, float y,float w, float h)
{
  if((p.x >= x)&&(p.x < x + w)&&(p.y >= y)&&(p.y < y + h))
    return 1;
  return 0;
}


/*eol@eof*/

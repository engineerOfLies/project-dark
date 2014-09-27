#include "entity.h"
#include "hud.h"

extern Uint32 NOW;
extern GLint viewport[4];
extern GLdouble modelview[16];
extern GLdouble projection[16];
extern SDL_Rect Camera;
extern glCamera_T glCamera;
extern Mouse_T Mouse;
Entity EntityList[MAXENTITIES];
int numents;

void InitEntList()
{
  int i;
  numents = 0;
  memset(EntityList,0,sizeof(Entity) * MAXENTITIES);
  for(i = 0 ;i < MAXENTITIES  ;i++)
  {
    EntityList[i].used = 0;
    EntityList[i].think = NULL;
    if(EntityList[i].texture!=NULL)FreeSprite(EntityList[i].texture);
    EntityList[i].texture=NULL;
    EntityList[i].update = NULL;
    EntityList[i].self=NULL;
    EntityList[i].owner = NULL;
  }
}

void ClearEntities()
{
  int i;
  for(i = 0;i < MAXENTITIES;i++)
  {
    if(EntityList[i].used)
      FreeEntity(&EntityList[i]);
  }
}

void ClearEntitiesExcept(Entity *skip)
{
  int i;
  for(i = 0;i < MAXENTITIES;i++)
  {
    if((EntityList[i].used)&&(skip != &EntityList[i]))
    {
      FreeEntity(&EntityList[i]);
    }
  }
}

void FreeEntity(Entity *ent)
{
  numents--;
  ent->used = 0;
  if(ent->texture!=NULL)FreeSprite(ent->texture);
  if(ent->shape != NULL)
  {
    cpShapeFree(ent->shape);
  }
  memset(ent,0,sizeof(Entity));
}

Entity *NewEntity()
{
  int i;
  if((numents +1) >= MAXENTITIES)
  {
    return NULL;
  }
  for(i = 0; i < MAXENTITIES; i++)
  {
    if(EntityList[i].used == 0)
    {
      memset(&EntityList[i],0,sizeof(Entity));
      EntityList[i].used = 1;
      EntityList[i].shown = 1;
      EntityList[i].alpha = 1;
      EntityList[i].solid = 1;
      EntityList[i].s.x = EntityList[i].s.y = EntityList[i].s.z = 1;
      numents++;
      return &EntityList[i];
    }
  }
  return NULL;
}

void thinkEntities(Entity *ent)
{
  if(ent->used != 0)
  {
    if(ent->think != NULL)
    {
      if(ent->NextThink <=  NOW )
      {	
        ent->think(ent->self);
        ent->NextThink= NOW + ent->ThinkRate;
      }
    }
  }
}

void updateEntities(Entity *ent)
{
  if(ent->used != 0)
  {
    if(ent->update != NULL)
    {
      if(ent->NextUpdate <=  NOW )
      {
        ent->update(ent->self);
        ent->NextUpdate= NOW + ent->UpdateRate;
      }
    }
  }
}

Entity *GetEntityByObjIndex(int id)
{
  int i;
  for(i = 0;i < MAXENTITIES;i++)
  {
    if((EntityList[i].used)&&(EntityList[i].objindex == id))
    {
      return &EntityList[i];
    }
  }
  return NULL;
}


Entity *GetEntityByID(int id)
{
  int i;
  for(i = 0;i < MAXENTITIES;i++)
  {
    if((EntityList[i].used)&&(EntityList[i].id == id))
    {
      return &EntityList[i];
    }
  }
  return NULL;
}


Entity *EntityIndex(int index)
{

  if(index > MAXENTITIES)
  {
    return NULL;
  }
  return &EntityList[index];
}

void drawEntities(Entity *ent,int drawbox)
{
  int i;
  if(ent == NULL)return;
  if(ent->used != 0)
  {
    if(ent->shown)
    {
      if(ent->drawtrail)DrawEntTrail(ent);
      if(ent->zfmmodel != NULL)
      {
        if(drawbox)DrawBBox(ent, 1, 1, 1);
        if(drawbox)DrawCircle3D(ent->p.x - Camera.x,ent->p.y - Camera.y,ent->p.z,ent->box.x, M_Coord(1,1,1), 1);
        zfmDrawModel(ent->zfmmodel,ent->zfmmodel->skeleton,ent->texture,ent->shaderProg,ent->p.x - Camera.x,ent->p.y - Camera.y,ent->p.z,ent->r.x,ent->r.y,ent->r.z,ent->s.x,ent->s.y,ent->s.z,GL_TRIANGLES,ent->frame,ent->alpha);
      }
      else if(ent->useslayers)
      {
        for(i = 0;i < 8;i++)
        {
          if(ent->layers[i] != NULL)
          {
            DrawSpriteStretchRot( ent->layers[i], ent->p.x - Camera.x, ent->p.y - Camera.y, (int)ent->layerframes[i], ent->s.x, ent->s.y, ent->r.z, ent->alpha );
          }
        }
      }
      else if(ent->texture != NULL)
      {
        DrawSpriteStretchRot( ent->texture, ent->p.x - Camera.x, ent->p.y - Camera.y, ent->frame,ent->s.x,ent->s.y, ent->r.z, ent->alpha );
      }
    }
  }
}

void updateAllEntities()
{
  int i;

  for (i = 0 ;i < MAXENTITIES  ;i++)
  {
    if (EntityList[i].used != 0)
    {
      if(EntityList[i].update != NULL)
      {
        EntityList[i].update(&EntityList[i]);
      }
    }
  }
  for (i = 0 ;i < MAXENTITIES  ;i++)
  {
    if (EntityList[i].used != 0)
    {
      if(EntityList[i].think != NULL)
      {
        EntityList[i].think(&EntityList[i]);
      }
    }
  }
/*touch will happen in the movement update*/
}

void drawAllEntities(int drawbox,int floor)
{
  int i;
  for (i = 0 ;i < MAXENTITIES  ;i++)
  {

    if ((EntityList[i].used != 0)&&(floor == EntityList[i].FloorDraw))
    {

      drawEntities(&EntityList[i],drawbox);
    }
  }
}

Entity *GetNextEntity(Entity *last,Entity *ignore)
{
  Entity *e;
  if(last == NULL)e = &EntityList[0];
  else 
  {
    e = last;
    e++;
  }
  while(e < &EntityList[MAXENTITIES])
  {
    if((e->used)&&(e != ignore))
      return e;
    e++;
  }
  return NULL;
}

void EntLookAtOther(Entity *self,Entity *other)
{
  Coord v;
  if((self == NULL)||(other == NULL))return;
  v.z = 0;
  v.x = other->p.x - self->p.x;
  v.y = other->p.y - self->p.y;
  VectorNormalize(&v);
  self->r.z = DegreeFromVect(-v.x, v.y);
}

int EntityClippingOther(Entity *self, Entity *other)
{
  int i;
  if(self == NULL)return 0;
  for(i = 0;i < MAXENTITIES;i++)
  {
    if(EntityList[i].used)
    {
      if((&EntityList[i] == self)||(&EntityList[i] == self->owner))continue;
      if(EntityList[i].team == self->team)continue;
      if(RelativeSize(self->p.x- EntityList[i].p.x, self->p.y- EntityList[i].p.y) <= ((self->box.x + EntityList[i].box.x)*(self->box.x + EntityList[i].box.x)))
      {
        other = &EntityList[i];
        return 4;
      }
    }
  }
  return 0;
}

void CoolDowns(Entity *self)
{
  int i;
  int total = 0;
  if(self == NULL)return;
  if(self->Acd > 0)self->Acd--;
  if(self->Lcd > 0)self->Lcd--;
  if(self->Rcd > 0)self->Rcd--;
  if(self->Gcd > 0)self->Gcd--;
  for(i = 0;i < D_NumTypes;i++)
  {
    if(self->resbonus[i] > 0)
    {
      self->resbonus[i]-=0.1;
      if(self->resbonus[i] < 0)self->resbonus[i] = 0;
    }
    total += self->resbonus[i];
  }
  if(total <= 0)
  {
    if(self->layers[7] != NULL)
    {
      FreeSprite(self->layers[7]);
      self->layers[7] = NULL;
    }
  }
}

void PushEntity(Entity *ent,Coord v)
{
  if(ent == NULL)return;
  ent->v.x += v.x;
  ent->v.y += v.y;
  ent->v.z += v.z;
}

int EntCollide(Entity *self, Entity *other)
{
  BBox b1,b2;
  if((self == NULL)||(other == NULL))return 0;
  memcpy(&b1,&self->box,sizeof(BBox));
  memcpy(&b2,&other->box,sizeof(BBox));
  b1.x = self->p.x - self->box.x;
  b1.y = self->p.y - self->box.y;
  b1.z = self->p.z - self->box.z;
  b2.x = other->p.x - other->box.x;
  b2.y = other->p.y - other->box.y;
  b2.z = other->p.z - other->box.z;
  return BoxCollide(b1,b2);
}

void DrawModelBBox(zfmModel *model, float r, float g, float b)
{
  glPushMatrix();
 
  glTranslatef(model->center[0]-(model->dimen[0]*0.5f),model->center[1]-(model->dimen[1]*0.5f),model->center[2]-(model->dimen[2]*0.5f));
  glColor4f(r,g,b,1);
  glBegin( GL_LINES);

  glVertex3f(model->dimen[0],0,0);
  glVertex3f(model->dimen[0],model->dimen[1],0);
  
  glVertex3f(0,0,0);
  glVertex3f(0,model->dimen[1],0);
  
  glVertex3f(0,0,model->dimen[2]);
  glVertex3f(0,model->dimen[1],model->dimen[2]);
  
  glVertex3f(model->dimen[0],0,model->dimen[2]);
  glVertex3f(model->dimen[0],model->dimen[1],model->dimen[2]);
  
  glVertex3f(model->dimen[0],model->dimen[1],0);
  glVertex3f(0,model->dimen[1],0);
  glVertex3f(0,0,0);
  glVertex3f(model->dimen[0],0,0);

  glVertex3f(model->dimen[0],model->dimen[1],model->dimen[2]);
  glVertex3f(0,model->dimen[1],model->dimen[2]);
  glVertex3f(0,0,model->dimen[2]);
  glVertex3f(model->dimen[0],0,model->dimen[2]);

  glVertex3f(model->dimen[0],model->dimen[1],0);
  glVertex3f(0,model->dimen[1],0);
  glVertex3f(0,model->dimen[1],model->dimen[2]);
  glVertex3f(model->dimen[0],model->dimen[1],model->dimen[2]);
  
  glVertex3f(model->dimen[0],0,0);
  glVertex3f(0,0,0);
  glVertex3f(0,0,model->dimen[2]);
  glVertex3f(model->dimen[0],0,model->dimen[2]);
  
  glVertex3f(model->dimen[0],model->dimen[1],0);
  glVertex3f(model->dimen[0],model->dimen[1],model->dimen[2]);
  glVertex3f(model->dimen[0],0,model->dimen[2]);
  glVertex3f(model->dimen[0],0,0);
  
  glVertex3f(0,model->dimen[1],0);
  glVertex3f(0,model->dimen[1],model->dimen[2]);
  glVertex3f(0,0,model->dimen[2]);
  glVertex3f(0,0,0);
  glEnd();
  
  glColor4f(1,1,1,1);
  glPopMatrix();
}

void DrawBBox(Entity *self, float r, float g, float b)
{
  glPushMatrix();
  glLineWidth(2);
  glTranslatef(self->p.x-self->box.x,self->p.y-self->box.y,self->p.z-self->box.z);
  glColor4f(r,g,b,1);
  glBegin( GL_LINES);

  glVertex3f(self->box.w,0,0);
  glVertex3f(self->box.w,self->box.h,0);
  
  glVertex3f(0,0,0);
  glVertex3f(0,self->box.h,0);
  
  glVertex3f(0,0,self->box.d);
  glVertex3f(0,self->box.h,self->box.d);
  
  glVertex3f(self->box.w,0,self->box.d);
  glVertex3f(self->box.w,self->box.h,self->box.d);
  
  glVertex3f(self->box.w,self->box.h,0);
  glVertex3f(0,self->box.h,0);
  glVertex3f(0,0,0);
  glVertex3f(self->box.w,0,0);

  glVertex3f(self->box.w,self->box.h,self->box.d);
  glVertex3f(0,self->box.h,self->box.d);
  glVertex3f(0,0,self->box.d);
  glVertex3f(self->box.w,0,self->box.d);

  glVertex3f(self->box.w,self->box.h,0);
  glVertex3f(0,self->box.h,0);
  glVertex3f(0,self->box.h,self->box.d);
  glVertex3f(self->box.w,self->box.h,self->box.d);
  
  glVertex3f(self->box.w,0,0);
  glVertex3f(0,0,0);
  glVertex3f(0,0,self->box.d);
  glVertex3f(self->box.w,0,self->box.d);
  
  glVertex3f(self->box.w,self->box.h,0);
  glVertex3f(self->box.w,self->box.h,self->box.d);
  glVertex3f(self->box.w,0,self->box.d);
  glVertex3f(self->box.w,0,0);
  
  glVertex3f(0,self->box.h,0);
  glVertex3f(0,self->box.h,self->box.d);
  glVertex3f(0,0,self->box.d);
  glVertex3f(0,0,0);
  glEnd();
  
  glColor4f(1,1,1,1);
  glPopMatrix();
}

void DrawBBox2(BBox box,float r, float g, float b)
{
  glPushMatrix();
 
  glTranslatef(box.x-(box.w*0.5f),box.y-(box.h*0.5f),box.z-(box.d*0.5f));

  glColor4f(r,g,b,1);
  glBegin( GL_LINES);

  glVertex3f(box.w  ,0        ,0);
  glVertex3f(box.w  ,box.h    ,0);
  
  glVertex3f(0      ,0        ,0);
  glVertex3f(0      ,box.h    ,0);
  
  glVertex3f(0      ,0        ,box.d);
  glVertex3f(0      ,box.h    ,box.d);
  
  glVertex3f(box.w  ,0        ,box.d);
  glVertex3f(box.w  ,box.h    ,box.d);
  
  glVertex3f(box.w  ,box.h    ,0);
  glVertex3f(0      ,box.h    ,0);
  
  glVertex3f(0      ,0        ,0);
  glVertex3f(box.w  ,0        ,0);

  glVertex3f(box.w  ,box.h    ,box.d);
  glVertex3f(0      ,box.h    ,box.d);
  
  glVertex3f(0      ,0        ,box.d);
  glVertex3f(box.w  ,0        ,box.d);

  glVertex3f(box.w  ,box.h    ,0);
  glVertex3f(0      ,box.h    ,0);
  
  glVertex3f(0      ,box.h    ,box.d);
  glVertex3f(box.w  ,box.h    ,box.d);
  
  glVertex3f(box.w  ,0        ,0);
  glVertex3f(0      ,0        ,0);
  
  glVertex3f(0      ,0        ,box.d);
  glVertex3f(box.w  ,0        ,box.d);
  
  glVertex3f(box.w  ,box.h    ,0);
  glVertex3f(box.w  ,box.h    ,box.d);
  
  glVertex3f(box.w  ,0        ,box.d);
  glVertex3f(box.w  ,0        ,0);
  
  glVertex3f(0      ,box.h    ,0);
  glVertex3f(0      ,box.h    ,box.d);
  
  glVertex3f(0      ,0        ,box.d);
  glVertex3f(0      ,0        ,0);
  glEnd();
  
  glColor4f(1,1,1,1);
  glPopMatrix();
}

void DrawBBoxFilled(BBox box,float r, float g, float b)
{
  glPushMatrix();
 
  glTranslatef(box.x-(box.w*0.5f),box.y-(box.h*0.5f),box.z-(box.d*0.5f));

  glColor4f(r,g,b,1);
  glBegin( GL_QUADS);

  glVertex3f(box.w  ,0        ,0);
  glVertex3f(box.w  ,box.h    ,0);
  
  glVertex3f(0      ,0        ,0);
  glVertex3f(0      ,box.h    ,0);
  
  glVertex3f(0      ,0        ,box.d);
  glVertex3f(0      ,box.h    ,box.d);
  
  glVertex3f(box.w  ,0        ,box.d);
  glVertex3f(box.w  ,box.h    ,box.d);
  
  glVertex3f(box.w  ,box.h    ,0);
  glVertex3f(0      ,box.h    ,0);
  
  glVertex3f(0      ,0        ,0);
  glVertex3f(box.w  ,0        ,0);

  glVertex3f(box.w  ,box.h    ,box.d);
  glVertex3f(0      ,box.h    ,box.d);
  
  glVertex3f(0      ,0        ,box.d);
  glVertex3f(box.w  ,0        ,box.d);

  glVertex3f(box.w  ,box.h    ,0);
  glVertex3f(0      ,box.h    ,0);
  
  glVertex3f(0      ,box.h    ,box.d);
  glVertex3f(box.w  ,box.h    ,box.d);
  
  glVertex3f(box.w  ,0        ,0);
  glVertex3f(0      ,0        ,0);
  
  glVertex3f(0      ,0        ,box.d);
  glVertex3f(box.w  ,0        ,box.d);
  
  glVertex3f(box.w  ,box.h    ,0);
  glVertex3f(box.w  ,box.h    ,box.d);
  
  glVertex3f(box.w  ,0        ,box.d);
  glVertex3f(box.w  ,0        ,0);
  
  glVertex3f(0      ,box.h    ,0);
  glVertex3f(0      ,box.h    ,box.d);
  
  glVertex3f(0      ,0        ,box.d);
  glVertex3f(0      ,0        ,0);
  glEnd();
  
  glColor4f(1,1,1,1);
  glPopMatrix();
}


Coord objMouseRayTrace(obj_read *model)
{
  Coord TempS;    /*temp position*/
  Coord cam;
  Coord T1,T2,T3;
  Point Mouse;    /*used locally, not the real mouse*/
  int i;
  Coord downvect;
  Coord rotation;
  Coord up,right,forward, angles;
  Coord tempvect;
  GLdouble dirx1,diry1,dirz1,dirx2,diry2,dirz2;
  
  if(model == NULL)
  {
    return TempS;
  }
  
  SDL_GetMouseState(&Mouse.x,&Mouse.y);
  
  cam.x=(-1)*glCamera.position.x;
  cam.y=(-1)*glCamera.position.y;
  cam.z=(-1)*glCamera.position.z;
	

  rotation.x=glCamera.rotation.x;
  rotation.y=glCamera.rotation.y;
  rotation.z=glCamera.rotation.z;

	
  angles=glCamera.rotation;

  AngleVectors(&angles,&forward,&right,&up);
  forward.x*=-1;
  forward.y*=-1;
  forward.z*=-1;
  right.x*=-1;
  right.y*=-1;
  right.z*=-1;


  RotatePointAroundVector(&tempvect,right,forward,((Mouse.y-384)*(0.05859375)*(-1)));
	
  RotatePointAroundVector(&downvect,up,tempvect,((Mouse.x-512)*(0.05859375)));
 
  RotateVectorX(&downvect,glCamera.rotation.x+90);
  RotateVectorY(&downvect,glCamera.rotation.y);
  RotateVectorZ(&downvect,glCamera.rotation.z);
  
  downvect.x*=-1;

  gluUnProject(Mouse.x, Mouse.y, 0, modelview, projection, viewport, &dirx1, &diry1, &dirz1);
  gluUnProject(Mouse.x, Mouse.y, 1, modelview, projection, viewport, &dirx2, &diry2, &dirz2);
	
  downvect.x=(dirx2-dirx1);
  downvect.y=(diry2-diry1);
  downvect.z=(dirz2-dirz1);


  RotateVectorX(&downvect,glCamera.rotation.x);
  RotateVectorY(&downvect,glCamera.rotation.y);
  RotateVectorZ(&downvect,glCamera.rotation.z);

  downvect.x*=-1;
  downvect.z*=-1;
  

  for (i = 1; i < model->numfaces; i++)
  {
    
    T1.x = model->vertices[model->faces[i].vert1[0]].vert[0];
    T1.y = model->vertices[model->faces[i].vert1[0]].vert[1];
    T1.z = model->vertices[model->faces[i].vert1[0]].vert[2];
    
    T2.x = model->vertices[model->faces[i].vert2[0]].vert[0];
    T2.y = model->vertices[model->faces[i].vert2[0]].vert[1];
    T2.z = model->vertices[model->faces[i].vert2[0]].vert[2];
    
    T3.x = model->vertices[model->faces[i].vert3[0]].vert[0];
    T3.y = model->vertices[model->faces[i].vert3[0]].vert[1];
    T3.z = model->vertices[model->faces[i].vert3[0]].vert[2];

    T1.x+=200;
    T1.y+=200;
    T2.x+=200;
    T2.y+=200;
    T3.x+=200;
    T3.y+=200;
    if(RayInTriangle(cam, downvect, T1, T2, T3, &TempS) == -1)
    {
      return TempS;
    }
  }

  return TempS;
}


Coord zfmRayTrace(zfmModel *model, float x, float y, float z, float x1, float y1, float z1)
{
  Coord TempS;    /*temp position*/
  Coord cam;
  Coord T1,T2,T3;
  int i;
  Coord downvect;
  
  if(model == NULL)
  {
    return TempS;
  }



  cam.x = x;
  cam.y = y;
  cam.z = z;
  
  downvect.x = 0;
  downvect.y = 0;
  downvect.z = -1;

  

  for (i = 1; i < model->numfaces; i++)
  {
    T1.x = model->verticesAtRest[model->faces[i].vert1[0]].vert[0];
    T1.y = model->verticesAtRest[model->faces[i].vert1[0]].vert[1];
    T1.z = model->verticesAtRest[model->faces[i].vert1[0]].vert[2];
    
    T2.x = model->verticesAtRest[model->faces[i].vert2[0]].vert[0];
    T2.y = model->verticesAtRest[model->faces[i].vert2[0]].vert[1];
    T2.z = model->verticesAtRest[model->faces[i].vert2[0]].vert[2];
    
    T3.x = model->verticesAtRest[model->faces[i].vert3[0]].vert[0];
    T3.y = model->verticesAtRest[model->faces[i].vert3[0]].vert[1];
    T3.z = model->verticesAtRest[model->faces[i].vert3[0]].vert[2];
    
    T1.x +=x1;
    T1.y +=y1;
    T1.z +=z1;

    T2.x +=x1;
    T2.y +=y1;
    T2.z +=z1;
    
    T3.x +=x1;
    T3.y +=y1;
    T3.z +=z1;
    if(RayInTriangle(cam, downvect, T1, T2, T3, &TempS) == -1)
    {
      return TempS;
    }
  }
  TempS.z = -1;
  return TempS;
}

float VectorAngle2D(float x,float y)
{
  float angle = atan2(y, x) + M_PI;
  float fraction = angle * 0.5 / M_PI;
  if (fraction >= 1.0)
  {
    fraction -= 1.0;
  }
  return (fraction * 360);
}

void RotateVectorX(Coord *vect, float angle)
{
  Coord temp;
	
  angle=angle*DEGTORAD;

  temp.x=vect->x;
  temp.y=(vect->y*cos(angle))-(vect->z*sin(angle));
  temp.z=(vect->y*sin(angle))+(vect->z*cos(angle));

  vect->x=temp.x;
  vect->y=temp.y;
  vect->z=temp.z;
}
void RotateVectorY(Coord *vect, float angle)
{
  Coord temp;

  angle=angle*DEGTORAD;

  temp.y=vect->y;
  temp.x=(vect->x*cos(angle))+(vect->z*sin(angle));
  temp.z=(vect->x*sin(angle)*(-1))+(vect->z*cos(angle));
		
  vect->x=temp.x;
  vect->y=temp.y;
  vect->z=temp.z;
}
void RotateVectorZ(Coord *vect, float angle)
{
  Coord temp;

  angle=angle*DEGTORAD;

  temp.z=vect->z;
  temp.x=(vect->x*cos(angle))-(vect->y*sin(angle));
  temp.y=(vect->x*sin(angle))+(vect->y*cos(angle));

  vect->x=temp.x;
  vect->y=temp.y;
  vect->z=temp.z;
}

int RayInTriangle(Coord start, Coord dir, Coord t1, Coord t2, Coord t3, Coord *contact)
{
  Coord normal;
  int n;
  n = RayInPlane(start, dir, t1, t2, t3, contact, &normal); 

  if(n == 0)return 0;
  if(PointInTriangle(*contact, t1, t2, t3,normal) == 1)return (1 * n);
  return 0;
}

int RayInPlane(Coord start, Coord dir, Coord t1, Coord t2, Coord t3, Coord *contact, Coord *normal)
{
  float A,B,C,D;
  float t;
  float denom;
  A = (t1.y * (t2.z - t3.z)) + (t2.y * (t3.z - t1.z)) + (t3.y * (t1.z - t2.z));
  B = (t1.z * (t2.x - t3.x)) + (t2.z * (t3.x - t1.x)) + (t3.z * (t1.x - t2.x));
  C = (t1.x * (t2.y - t3.y)) + (t2.x * (t3.y - t1.y)) + (t3.x * (t1.y - t2.y));
  D = -((t1.x *(t2.y * t3.z - t3.y * t2.z)) + (t2.x *(t3.y * t1.z - t1.y * t3.z)) + (t3.x *(t1.y * t2.z - t2.y * t1.z)));
  denom = ((A * dir.x) + (B * dir.y) + (C * dir.z));
  if(denom == 0)return 0;
  t = - (((A * start.x) + (B * start.y) + (C * start.z) + D) / denom);
  if((t > 0)&&(t <= 1))
  {
    contact->x = start.x + (dir.x * t);
    contact->y = start.y + (dir.y * t);
    contact->z = start.z + (dir.z * t);
    normal->x = A;
    normal->y = B;
    normal->z = C;
    return 1;
  }
  contact->x = start.x + (dir.x * t);
  contact->y = start.y + (dir.y * t);
  contact->z = start.z + (dir.z * t);  
  normal->x = A;
  normal->y = B;
  normal->z = C;
  return -1;
}

int PointInTriangle(Coord point, Coord t1, Coord t2, Coord t3, Coord normal)
{
  int MaxAxis;
  float MaxVal;
  int count = 0;
  Coord temp;
  Coord dir = {10000,0,0};
  MaxAxis = 2;
  MaxVal = fabs(normal.x);
  if(fabs(normal.y) > MaxVal)
  {
    MaxAxis = 1;
    MaxVal = fabs(normal.y);
  }
  if(fabs(normal.z) > MaxVal)
  {
    MaxAxis = 2;
    MaxVal = fabs(normal.z);
  }
  switch(MaxAxis)
  {
    case 2:
      if(RayCrossEdgeZContact(point,dir,t1.x, t1.y,t2.x, t2.y, NULL) != 0)count++;
      if(RayCrossEdgeZContact(point,dir,t3.x, t3.y,t2.x, t2.y, NULL) != 0)count++;
      if(RayCrossEdgeZContact(point,dir,t1.x, t1.y,t3.x, t3.y, NULL) != 0)count++;
      break;
    case 1:
      temp.y = point.z;
      temp.x = point.x;
      if(RayCrossEdgeZContact(temp,dir,t1.x, t1.z,t2.x, t2.z, NULL) != 0)count++;
      if(RayCrossEdgeZContact(temp,dir,t3.x, t3.z,t2.x, t2.z, NULL) != 0)count++;
      if(RayCrossEdgeZContact(temp,dir,t1.x, t1.z,t3.x, t3.z, NULL) != 0)count++;
      break;
    case 0:
      temp.y = point.y;
      temp.x = point.z;
      if(RayCrossEdgeZContact(temp,dir,t1.z, t1.y,t2.z, t2.y, NULL) != 0)count++;
      if(RayCrossEdgeZContact(temp,dir,t3.z, t3.y,t2.z, t2.y, NULL) != 0)count++;
      if(RayCrossEdgeZContact(temp,dir,t1.z, t1.y,t3.z, t3.y, NULL) != 0)count++;
      break;
  }
  return (count % 2);
}

int RayCrossEdgeZContact(Coord point,Coord v,float x1, float y1,float x2, float y2, Coord *contact)
{
  float minx, maxx,miny,maxy;
  float testx, testy,testx2,testy2;
  float endx, endy;
  float Ua,Ub,Uden;
  
  endx = point.x + v.x;
  endy = point.y + v.y;
  if(point.x > endx)
  {
    minx = endx;
    maxx = point.x;
  }
  else
  {
    maxx = endx;
    minx = point.x;
  }
  if(point.y > endy)
  {
    miny = endy;
    maxy = point.y;
  }
  else
  {
    maxy = endy;
    miny = point.y;
  }
  
  Uden = ((y2 - y1)*(endx - point.x)) - ((x2 - x1)*(endy - point.y));
  if(Uden == 0)return 0;/*parallel, can't hit*/
 
  Ua = (((x2 - x1)*(point.y - y1))-((y2 - y1)*(point.x - x1))) / Uden;
  Ub = (((endx - point.x)*(point.y - y1))-((endy - point.y)*(point.x - x1))) / Uden;
  
  testx = point.x + (Ua * (endx - point.x));
  testy = point.y + (Ua * (endy - point.y));
  testx2= x1 + (Ub * (x2-x1));
  testy2= y1 + (Ub * (y2-y1));
  
  if(contact != NULL)
  {
    contact->x = testx;
    contact->y = testy;
  }
  
  if((Ua >= 0) && (Ua <= 1) && (Ub >= 0) && ( Ub <= 1))
    return 1;
  else
    return 0;

}

int RayCrossEdge(float sx,float sy,float vx, float vy,float x1, float y1,float x2, float y2, float *tx, float *ty)
{
  int ret;
  Coord p,v;
  p.x = sx;
  p.y = sy;
  v.x = vx;
  v.y = vy;
  ret = RayCrossEdgeZContact(p, v, x1, y1, x2, y2, &v);
  if(tx)*tx = v.x;
  if(ty)*ty = v.y;
  return ret;
}


void RotatePointAroundVector( Coord *dst, Coord dir, Coord point, float degrees )
{
  float	m[3][3];
  float	im[3][3];
  float	zrot[3][3];
  float	tmpmat[3][3];
  float	rot[3][3];
  Coord vr, vup, vf;

  vf.x = dir.x;
  vf.y = dir.y;
  vf.z = dir.z;

  PerpendicularVector( &vr, dir );
  CrossProduct( vr, vf, &vup );
  m[0][0] = vr.x;
  m[1][0] = vr.y;
  m[2][0] = vr.z;

  m[0][1] = vup.x;
  m[1][1] = vup.y;
  m[2][1] = vup.z;

  m[0][2] = vf.x;
  m[1][2] = vf.y;
  m[2][2] = vf.z;

  memcpy( im, m, sizeof( im ) );

  im[0][1] = m[1][0];
  im[0][2] = m[2][0];
  im[1][0] = m[0][1];
  im[1][2] = m[2][1];
  im[2][0] = m[0][2];
  im[2][1] = m[1][2];

  memset( zrot, 0, sizeof( zrot ) );
  zrot[0][0] = zrot[1][1] = zrot[2][2] = 1.0F;

  zrot[0][0] = cos( ( degrees*DEGTORAD ) );
  zrot[0][1] = sin( ( degrees*DEGTORAD ) );
  zrot[1][0] = -sin( ( degrees*DEGTORAD ) );
  zrot[1][1] = cos( ( degrees*DEGTORAD ) );

  R_ConcatRotations( m, zrot, tmpmat );
  R_ConcatRotations( tmpmat, im, rot );

  dst->x = rot[0][0] * point.x + rot[0][1] * point.y + rot[0][2] * point.z;
  dst->y = rot[1][0] * point.x + rot[1][1] * point.y + rot[1][2] * point.z;
  dst->z = rot[2][0] * point.x + rot[2][1] * point.y + rot[2][2] * point.z;
}

void AngleVector2D(Coord *v,float angle)
{
  Coord uv = {1,0,0}; /*unit vector*/
  if(v == NULL)return;
  RotateVectorZ (&uv,-90 - angle);
  v->x = uv.x;
  v->y = uv.y;
  v->z = 0;
}

void AngleVectors (Coord *angles, Coord *forward, Coord *right, Coord *up)
{
  float		angle;
  static float		sr, sp, sy, cr, cp, cy;


  angle = angles->x * (DEGTORAD);
  sy = sin(angle);
  cy = cos(angle);
  angle = angles->y * (DEGTORAD);
  sp = sin(angle);
  cp = cos(angle);
  angle = angles->z * (DEGTORAD);
  sr = sin(angle);
  cr = cos(angle);

  if(forward != NULL)
  {
    forward->x = cp*cy;
    forward->y = cp*sy;
    forward->z = -sp;
  }
  if(right != NULL)
  {
    right->x = (-1*sr*sp*cy+-1*cr*-sy);
    right->y = (-1*sr*sp*sy+-1*cr*cy);
    right->z = -1*sr*cp;
  }
  if(up != NULL)
  {
    up->x = (cr*sp*cy+-sr*-sy);
    up->y = (cr*sp*sy+-sr*cy);
    up->z = cr*cp;
  }
}

/*
================
R_ConcatRotations
================
*/
void R_ConcatRotations (float in1[3][3], float in2[3][3], float out[3][3])
{
  out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
      in1[0][2] * in2[2][0];
  out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
      in1[0][2] * in2[2][1];
  out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
      in1[0][2] * in2[2][2];
  out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
      in1[1][2] * in2[2][0];
  out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
      in1[1][2] * in2[2][1];
  out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
      in1[1][2] * in2[2][2];
  out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
      in1[2][2] * in2[2][0];
  out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
      in1[2][2] * in2[2][1];
  out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
      in1[2][2] * in2[2][2];
}

void CrossProduct (Coord v1, Coord v2, Coord *cross)
{
  cross->x = v1.y*v2.z - v1.z*v2.y;
  cross->y = v1.z*v2.x - v1.x*v2.z;
  cross->z = v1.x*v2.y - v1.y*v2.x;
}


void PerpendicularVector( Coord *dst, Coord src )
{
  int	pos;
  float minelem = 1.0F;
  Coord tempvec;

	/*
  ** find the smallest magnitude axially aligned vector
        */
  pos=0;

  if ( fabs( src.x ) < minelem )
  {
    pos=0;
    minelem = fabs( src.x );
  }
  if ( fabs( src.y ) < minelem )
  {
    pos=1;
    minelem = fabs( src.y );
  }
  if ( fabs( src.y ) < minelem )
  {
    pos=2;
    minelem = fabs( src.z );
  }
	
  tempvec.x=0;
  tempvec.y=0;
  tempvec.z=0;

  switch(pos)
  {
    case 0:
      tempvec.x=1;
      break;
    case 1:
      tempvec.y=1;
      break;
    case 2:
      tempvec.z=1;
      break;
  }

	/*
  ** project the point onto the plane defined by src
        */
  ProjectPointOnPlane( dst, tempvec, src );

	/*
  ** normalize the result
        */
  VectorNormalize( dst );
}

float VectorNormalize (Coord *v)
{
  float	length;

  length = (v->x*v->x) + (v->y*v->y) + (v->z*v->z);
  length = sqrt (length);		/* FIXME*/

  if (length!= 0)
  {
    v->x /= length;
    v->y /= length;
    v->z /= length;
  }
		
  return length;

}

float RelativeSize(float x, float y)
{
  return ((x * x) + (y * y));
}

void ProjectPointOnPlane( Coord *dst, Coord p, Coord normal )
{
  float d;
  Coord n;
  float inv_denom;

  inv_denom = 1.0F / DotProduct( normal, normal );

  d = DotProduct( normal, p ) * inv_denom;

  n.x = normal.x * inv_denom;
  n.y = normal.y * inv_denom;
  n.z = normal.z * inv_denom;

  dst->x = p.z - d * n.x;
  dst->y = p.y - d * n.y;
  dst->z = p.x - d * n.z;
}


Entity *GetEntUnderMouse()
{
  int i;
  for(i = 0;i < MAXENTITIES;i++)
  {
    if (EntityList[i].used != 0)
    {
      if(MouseRayTraceInBox(EntityList[i].box, EntityList[i].p.x, EntityList[i].p.y, EntityList[i].p.z))
      {

        return &EntityList[i];
      }
    }
  }
  return NULL;
}

int MouseRayTraceAABB(Entity *self)
{
  Coord TempS;    
  Coord cam;
  Coord T1,T2,T3;
  Coord Offset;
  float h,w,d;
  Coord downvect;
  Coord rotation;
  Coord up,right,forward, angles;
  Coord tempvect;
  GLdouble dirx1,diry1,dirz1,dirx2,diry2,dirz2;
  
  cam.x=(-1)*glCamera.position.x;
  cam.y=(-1)*glCamera.position.y;
  cam.z=(-1)*glCamera.position.z;
	

  rotation.x=glCamera.rotation.x;
  rotation.y=glCamera.rotation.y;
  rotation.z=glCamera.rotation.z;

	
  angles=glCamera.rotation;

  AngleVectors(&angles,&forward,&right,&up);
  forward.x*=-1;
  forward.y*=-1;
  forward.z*=-1;
  right.x*=-1;
  right.y*=-1;
  right.z*=-1;


  RotatePointAroundVector(&tempvect,right,forward,((Mouse.y- Camera.w/2)*(0.05859375)*(-1)));
	
  RotatePointAroundVector(&downvect,up,tempvect,((Mouse.x-Camera.h/2)*(0.05859375)));
 
  RotateVectorX(&downvect,glCamera.rotation.x+90);
  RotateVectorY(&downvect,glCamera.rotation.y);
  RotateVectorZ(&downvect,glCamera.rotation.z);
  
  downvect.x*=-1;

  gluUnProject(Mouse.x, Mouse.y, 0, modelview, projection, viewport, &dirx1, &diry1, &dirz1);
  gluUnProject(Mouse.x, Mouse.y, 1, modelview, projection, viewport, &dirx2, &diry2, &dirz2);
	/*EDITED RANGE*/
  downvect.x=(dirx2-dirx1);
  downvect.y=(diry2-diry1);
  downvect.z=(dirz2-dirz1);


  RotateVectorX(&downvect,glCamera.rotation.x);
  RotateVectorY(&downvect,glCamera.rotation.y);
  RotateVectorZ(&downvect,glCamera.rotation.z);

  downvect.x*=-1;
  downvect.z*=-1;
  
  Offset.x=self->p.x + self->box.x;
  Offset.y=self->p.y + self->box.y;
  Offset.z=self->p.z + self->box.z;
  
  h=self->box.h +Offset.y;
  w=self->box.w +Offset.x;
  d=-self->box.d +Offset.z;

  T1.x=w;
  T1.y=h;
  T1.z=Offset.z;
  
  T2.x=Offset.x;
  T2.y=Offset.y;
  T2.z=Offset.z;
  
  T3.x=Offset.x;
  T3.y=h;
  T3.z=Offset.z;
  
  if(RayInTriangle(cam, downvect, T1, T2, T3, &TempS) == -1)
  {
    if(TempS.x < w )
    {
      return 1;
    }
  }

  T1.x=w;
  T1.y=h;
  T1.z=Offset.z;
  
  T2.x=w;
  T2.y=Offset.y;
  T2.z=Offset.z;
  
  T3.x=Offset.x;
  T3.y=Offset.y;
  T3.z=Offset.z;
  
  if(RayInTriangle(cam, downvect, T1, T2, T3, &TempS) == -1)
  {
    if(TempS.x < w )
    {
      return 1;
    }
  }

  T1.x=Offset.x;
  T1.y=Offset.y;
  T1.z=d;
  
  T2.x=Offset.x;
  T2.y=Offset.y;
  T2.z=Offset.z;
  
  T3.x=w;
  T3.y=Offset.y;
  T3.z=d;
  
  if(RayInTriangle(cam, downvect, T1, T2, T3, &TempS) == -1)
  {
    if(TempS.x < w )
    {
      return 1;
    }
  }

  T1.x=w;
  T1.y=Offset.y;
  T1.z=d;
  
  T2.x=w;
  T2.y=Offset.y;
  T2.z=Offset.z;
  
  T3.x=Offset.x;
  T3.y=Offset.y;
  T3.z=Offset.z;
  
  if(RayInTriangle(cam, downvect, T1, T2, T3, &TempS) == -1)
  {
    if(TempS.x < w )
    {
      return 1;
    }
  }
  return 0;
}

int MouseRayTraceInBox(BBox box, float x, float y, float z)
{
  Coord TempS;    
  Coord cam;
  Coord T1,T2,T3;
  Coord Offset;
  float h,w,d;
  Coord downvect;
  Coord rotation;
  Coord up,right,forward, angles;
  Coord tempvect;
  GLdouble dirx1,diry1,dirz1,dirx2,diry2,dirz2;
  
  
  cam.x=(-1)*glCamera.position.x;
  cam.y=(-1)*glCamera.position.y;
  cam.z=(-1)*glCamera.position.z;
	

  rotation.x=glCamera.rotation.x;
  rotation.y=glCamera.rotation.y;
  rotation.z=glCamera.rotation.z;

	
  angles=glCamera.rotation;

  AngleVectors(&angles,&forward,&right,&up);
  forward.x*=-1;
  forward.y*=-1;
  forward.z*=-1;
  right.x*=-1;
  right.y*=-1;
  right.z*=-1;


  RotatePointAroundVector(&tempvect,right,forward,((Mouse.y-Camera.w/2)*(0.05859375)*(-1)));
	
  RotatePointAroundVector(&downvect,up,tempvect,((Mouse.x-Camera.h/2)*(0.05859375)));
 
  RotateVectorX(&downvect,glCamera.rotation.x+90);
  RotateVectorY(&downvect,glCamera.rotation.y);
  RotateVectorZ(&downvect,glCamera.rotation.z);
  
  downvect.x*=-1;

  gluUnProject(Mouse.x, Mouse.y, 0, modelview, projection, viewport, &dirx1, &diry1, &dirz1);
  gluUnProject(Mouse.x, Mouse.y, 1, modelview, projection, viewport, &dirx2, &diry2, &dirz2);
	
  downvect.x=(dirx2-dirx1);
  downvect.y=(diry2-diry1);
  downvect.z=(dirz2-dirz1);


  RotateVectorX(&downvect,glCamera.rotation.x);
  RotateVectorY(&downvect,glCamera.rotation.y);
  RotateVectorZ(&downvect,glCamera.rotation.z);

  downvect.x*=-1;
  downvect.z*=-1;
  
  Offset.x = x - box.x;
  Offset.y = y - box.y;
  Offset.z = z - box.z;
/*  Offset.x=x + (box.x-(box.w*0.5f));
  Offset.y=y + (box.y-(box.h*0.5f));
  Offset.z=z + (box.z-(box.d*0.5f));
*/
  
  h= box.h +Offset.y;
  w= box.w +Offset.x;
  d=-box.d +Offset.z;

  T1.x=w;
  T1.y=h;
  T1.z=Offset.z;
  
  T2.x=Offset.x;
  T2.y=Offset.y;
  T2.z=Offset.z;
  
  T3.x=Offset.x;
  T3.y=h;
  T3.z=Offset.z;
  
  if(RayInTriangle(cam, downvect, T1, T2, T3, &TempS) == -1)
  {
    if(TempS.x < w )
    {
      return 1;
    }
  }

  T1.x=w;
  T1.y=h;
  T1.z=Offset.z;
  
  T2.x=w;
  T2.y=Offset.y;
  T2.z=Offset.z;
  
  T3.x=Offset.x;
  T3.y=Offset.y;
  T3.z=Offset.z;
  
  if(RayInTriangle(cam, downvect, T1, T2, T3, &TempS) == -1)
  {
    if(TempS.x < w )
    {
      return 1;
    }
  }

  T1.x=Offset.x;
  T1.y=Offset.y;
  T1.z=d;
  
  T2.x=Offset.x;
  T2.y=Offset.y;
  T2.z=Offset.z;
  
  T3.x=w;
  T3.y=Offset.y;
  T3.z=d;
  
  if(RayInTriangle(cam, downvect, T1, T2, T3, &TempS) == -1)
  {
    if(TempS.x < w )
    {
      return 1;
    }
  }

  T1.x=w;
  T1.y=Offset.y;
  T1.z=d;
  
  T2.x=w;
  T2.y=Offset.y;
  T2.z=Offset.z;
  
  T3.x=Offset.x;
  T3.y=Offset.y;
  T3.z=Offset.z;
  
  if(RayInTriangle(cam, downvect, T1, T2, T3, &TempS) == -1)
  {
    if(TempS.x < w )
    {
      return 1;
    }
  }
  return 0;
}

void DrawTrailSegment(Coord s, Coord e,Coord C)
{
  glColor4f(C.x,C.y,C.z, 0.5);
  glLineWidth(6);
  glBegin(GL_LINES);
  glVertex3f(s.x,s.y,-0.3);
  glVertex3f(e.x,e.y,-0.3);
  glEnd();
  glColor4f((C.x + 1)/2,(C.y + 1)/2,(C.z + 1)/2,0.5);
  glLineWidth(2);
  glBegin(GL_LINES);
  glVertex3f(s.x,s.y,-0.2);
  glVertex3f(e.x,e.y,-0.2);
  glEnd();
  glColor4f(1,1,1,0.5);
  glLineWidth(1);
  glBegin(GL_LINES);
  glVertex3f(s.x,s.y,-0.1);
  glVertex3f(e.x,e.y,-0.1);
  glEnd();
  glColor4f(1,1,1,1);
  glLineWidth(2);
}

void DrawEntPath(Entity *ent)
{
  int i;
  for(i = 0;i < (ent->pathlen - 1);i++)
  {
    DrawTrailSegment(M_Coord((ent->path[i].x * TILEW) + TILEW/2,(ent->path[i].y * TILEW) + TILEW/2,0),M_Coord((ent->path[i + 1].x * TILEW) + TILEW/2,(ent->path[i + 1].y * TILEW) + TILEW/2,0),ent->color);
  }
}

void DrawEntTrail(Entity *ent)
{
  int i;
  int t = ent->trailhead - 1;
  int t2 = ent->trailhead - 2;
  if(t < 0)t += 16;
  if(t2 < 0)t2 += 16;
  for(i = 0;i < (ent->trailcount - 1);i++)
  {
    DrawTrailSegment(ent->trail[t],ent->trail[t2],ent->color);
    t = t2;
    t2--;
    if(t2 < 0)t2 = 15;
  }
}

/*eol @ eof*/

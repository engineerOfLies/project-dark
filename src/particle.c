#include <stdlib.h>
#include <string.h>
#include "particle.h"
#include "entity.h"

#define MAXPART    8192

extern SDL_Rect    Camera;
extern SDL_Surface *screen;
extern Uint32      NOW;
extern GLuint ShaderProg;
Particle           ParticleList[MAXPART];
int                NumParticles;

float OffSet(float die)   /*this will give a random float between die and -die*/
{
  return (((rand()>>8) % (int)(die * 20))* 0.1) - die;
}


void ResetAllParticles()
{
  memset(&ParticleList,0,sizeof(Particle) * MAXPART);
  NumParticles = 0;
}

Particle *SpawnParticle()    /*give a pointer to newly created particles.*/
{
  int i;
  if(NumParticles >= MAXPART)return NULL;     /*all out of them darn particles*/
  for(i = 0;i < MAXPART;i++)
  {
    if(ParticleList[i].used == 0)   /*lets find the first available slot*/
    {
      ParticleList[i].used = 1;
      ParticleList[i].scale = 1;
      NumParticles++;
      return &ParticleList[i];
    }
  }
  return NULL;/*catch all*/
}

void DrawAllParticles()   /*draw and update all of the particles that are active to the draw buffer*/
{
  Coord pos = {0,0,0};
  float fade;
  Particle *p;
  int i,done,killed;
  killed = 0;
  done = 0;
  for(i = 0;i < MAXPART;i++)
  {
    if(done >= NumParticles)break;
    if(ParticleList[i].used == 1)
    {
      done++;
      /*particle drawing*/
      p = &ParticleList[i];   /*this for shorthand.*/
      switch(p->type)
      {
        case PT_Point:
          pos = Point2DFrom3D(M_Coord(p->p.x - Camera.x, p->p.y - Camera.y,0));
          glPushMatrix();
          glPushAttrib(GL_LIGHTING_BIT);
          glPointSize(p->scale);
          glDisable(GL_DEPTH_TEST);
          glEnable(GL_POINT_SMOOTH);
          glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
          glEnable(GL_BLEND); 
          glColor4f(p->Color.x,p->Color.y,p->Color.z,p->alpha);
          glBegin(GL_POINTS);
          glVertex3f(pos.x,pos.y,pos.z);
          glEnd();
          glPopAttrib();
          glColor4f(1,1,1,1);
          glEnable(GL_DEPTH_TEST);
          glDisable(GL_BLEND);
          glPopMatrix();
          break;
        case PT_Trail:
          break;
        case PT_Sprite:
          break;
        case PT_Obj:
          break;
        case PT_Model:
          break;
      }
      ParticleList[i].lifespan--;
      if(ParticleList[i].lifespan <= 0)/*if we have expired or fallen out of sight,...*/
      {
        killed++;
        ParticleList[i].used = 0;
      }
      else
      {
        ParticleList[i].p.x += ParticleList[i].v.x;   /*apply velocity to all positions*/
        ParticleList[i].p.y += ParticleList[i].v.y;
        ParticleList[i].p.z += ParticleList[i].v.z;
        ParticleList[i].v.x += ParticleList[i].a.x;   /*apply accelleration to all velocities*/
        ParticleList[i].v.y += ParticleList[i].a.y;
        ParticleList[i].v.z += ParticleList[i].a.z;
        /*fading out*/
        if(ParticleList[i].lifespan < ParticleList[i].fadeout)
        {
          fade = 1.0/ParticleList[i].fadeout;
          ParticleList[i].alpha -= fade;
        }
      }
    }
  }
  NumParticles -= killed;
}

void ParticleExplosion(float x,float y, float z, Uint8 time, int count, float speed, float r, float g, float b, float scale, float alpha)
{
  int i;
  Particle *p;
  for(i = 0; i < count;i++)
  {
    p = SpawnParticle();
    if(p == NULL)
    {
      NewMessage("Ha HOO",IndexColor(LightYellow));
      
      return;
    }
    p->p.x = x;
    p->p.y = y;
    p->p.z = z;
    p->Color.x = r + (crandom()*0.1);
    p->Color.y = g + (crandom()*0.1);
    p->Color.z = b + (crandom()*0.1);
    p->alpha = alpha;
    p->type = PT_Point;
    p->v.x = crandom();
    p->v.y = crandom();
    p->v.z = crandom();
    Normalize(&p->v);
    p->v.x *= speed;
    p->v.y *= speed;
    p->v.z *= speed;
    p->lifespan = time;
    p->fadeout = time;
    p->scale = scale;
  }
}


void SpawnSpray(float x,float y,float z, float vx,float vy, float vz,float speed,Coord color,float alpha, float scale, int time,int count)
{
  int i;
  Particle *p;
  for(i = 0; i < count;i++)
  {
    p = SpawnParticle();
    if(p == NULL)return;
    p->p.x = x;
    p->p.y = y;
    p->p.z = z;
    p->Color.x = color.x + (crandom()*0.01);
    p->Color.y = color.y + (crandom()*0.01);
    p->Color.z = color.z + (crandom()*0.01);
    p->alpha = alpha;
    p->type = PT_Point;
    p->v.x = vx + (crandom() * 0.4);
    p->v.y = vy + (crandom() * 0.4);
    p->v.z = vz + (crandom() * 0.4);
    Normalize(&p->v);
    p->v.x *= speed + (crandom() * 0.4);
    p->v.y *= speed + (crandom() * 0.4);
    p->v.z *= speed + (crandom() * 0.4);
    if(crandom() > 0)
    {
      p->p.x += (p->v.x * 0.5);
      p->p.y += (p->v.y * 0.5);
      p->p.z += (p->v.z * 0.5);
    }
    p->lifespan = time;
    p->fadeout = time;
    p->scale = scale;
  }
  
}

void DropParticle(Coord s,Coord color,Uint32 time)
{
  Particle *p;
  p = SpawnParticle();
  if(p == NULL)return;
  p->type = PT_Point;
  p->p.x = s.x;
  p->p.y = s.y;
  p->p.z = s.z;
  p->Color.x = color.x;
  p->Color.y = color.y;
  p->Color.z = color.z;
  p->alpha = 1;
  p->lifespan = time;
  p->fadeout = time;
  p->scale = 5;
}

/*EO@EOF*/

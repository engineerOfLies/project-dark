#ifndef __PARTICLE__
#define __PARTICLE__

#include "graphics.h"
#include "glmodel.h"
#include "modelLoader.h"

enum ParticleTypes {PT_Point,PT_Trail,PT_Sprite, PT_Obj,PT_Model};

#define TRAILMAX 16

typedef struct Particle_T
{
  Sprite *sprite;         /*bilboarded sprite or texture for a model*/
  Model *model;           /*non animated obj*/
  zfmModel *zmodel;       /*armature supported Model w/ animation*/
  Coord p;                /*coordinates of where the particle is getting rendered*/
  Coord trail[TRAILMAX];  /*last few coordinates*/
  Coord v;                /*vector of particle*/
  Coord a;                /*how is the particle accellerating*/
  Coord Color;            /*opengl float values for color*/
  float alpha;            /*alpha value for the particle*/
  int type;               /*enum of particle types.  All have different draw types*/
  int lifespan;           /*how many updates before I die*/
  int fadeout;            /*how many frames to fade out from*/
  float fadestep;         /*how alpha is reduced*/
  float scale;            /*size of the particle to be drawn*/
  int used;
}Particle;

void ParticleExplosion(float x,float y, float z, Uint8 time, int count, float speed, float r, float g, float b, float scale, float alpha);
void ResetAllParticles();
Particle *SpawnParticle();    /*give a pointer to newly created particles.*/
void DrawAllParticles();
void SpawnFountain(Uint32 Color,int mx,int my,Uint8 time);
void SpawnSpray(float x,float y,float z, float vx,float vy, float vz,float speed,Coord color,float alpha, float scale, int time,int count);
void DropParticle(Coord s,Coord color,Uint32 time);
void ItsRaining(Uint32 Color,Uint8 time,int volume,float decent);
#endif

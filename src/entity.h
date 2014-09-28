#ifndef __ENTITY__
#define __ENTITY__

#include <chipmunk/chipmunk.h>
#include "audio.h"
#include "modelLoader.h"
#include "levels.h"


#define PATHMAX 64
#define MAXENTITIES   1024
#define FUDGE 0.2

enum Damage_T {D_Physical, D_Pierce, D_Slash, D_Crush, D_Magic, D_Fire, D_Ice, D_Elec, D_Shadow, D_Light};
enum Damage_MT {MT_Physical = 1, MT_Pierce = 2, MT_Slash = 4, MT_Crush = 8, MT_Magic = 16, MT_Fire = 32, MT_Ice = 64, MT_Elec = 128, MT_Shadow = 256, MT_Light = 512};
#define D_NumTypes 10

enum AttackType {AT_Stab,AT_Slash,AT_Bash, AT_Crush, AT_Shoot,AT_Throw,AT_Cast,AT_Parry,AT_Potion};
/*                                                                          this one does not think*/
enum MonsterStates {MS_Idle, MS_Stunned, MS_Alert, MS_Hostile, MS_Attacking, MS_Dead, MS_Deadbody, MS_Pathing};

typedef struct Entity_T
{
  int used;
  struct Entity_T *self;	/*pointer to itself*/
  struct Entity_T *owner;	/*pointer to the owner Entity*/
  struct Entity_T *target;/*pointer to the entity that we are targetting or stuck too*/
  
  cpShape *shape;           /*link to physics engine*/
  cpLayers cplayermask;      /*layes that this entity collides with*/
  
  int     dirty;          /*dirty entites have been modified and need to be updated between frames*/
  
  void (*think) (struct Entity_T *self);
  Uint32 NextThink;
  Uint16 ThinkRate;
  void (*update) (struct Entity_T *self);
  Uint32 NextUpdate;
  Uint16 UpdateRate;
  
  void (*activate) (struct Entity_T *self,struct Entity_T *other); /*this is from the player use*/
  void (*touch) (struct Entity_T *self,struct Entity_T *other);/*this is the touch from walking into*/
  void (*hit) (struct Entity_T *self,struct Entity_T *other);  /*this is the touch from damage*/
  void (*die) (struct Entity_T *self,struct Entity_T *other);
  
  int FloorDraw;          /*makes the item draw before the tiles*/
  char name[80];          /*name of the entity*/
  zfmModel *zfmmodel;
  int padding[40];
  Sprite *texture;          /*the texture for the Entity*/
  int  useslayers;          /*if this entity uses the layered approach*/
  Sprite *layers[8];        /*the layers to make the entity*/
  float  layerframes[8];    /**/
  Armature *skel;
  GLuint shaderProg;
  float alpha;
  int shown;
  int sightblock;         /*if set, it will show up in traces for vision.*/
  int id;                 /*searchable id number*/
  float frame;
  float fdir;
  int textframe;          /*for use when the texture is a sprite, but broken up amonst multiple images*/
  float frate;              /**/
  Coord color;            /*color of the entity*/

  Sound* sound[4];         /*4 sounds per ent*/
  char pad[10];
  Coord p;                /*position*/
  Coord p2;               /*offset position*/
  Point path[PATHMAX];         /*the path to be followed to the entity's target*/
  int   pathstep;         /*which part of the path we are following*/
  int   pathlen;          /*how many steps are in the path*/
  Coord trail[16];        /*the last 16 locations of the entity*/
  int   trailhead;        /**/
  int   trailcount;       /*starts 0, stays 16*/
  int   drawtrail;        /*if 1 draws the trails*/
  
  Coord v;                /*velocity*/
  Coord mv;               /*desired movement vector*/
  Coord a;                /*acceleration*/
  Coord r;                /*rotation*/
  Coord rvec;
  Coord s;                /*scale*/
  Coord Offset;
  Coord AimDir;           /*vector player is aiming in*/
  int switchdelay;        /*delay between checking to change the weapon again*/
  int particletrail;      /*if it leaves a trail of particles in its wake*/
  int state;
  int Acd,Rcd,Lcd,Gcd;    /*cooldown counters for different actions: ALL, Right arm, Left Arm and Other?*/
  int stun;               /*cooldown for stun, from taking damage, hitting a shield, getting guard broken, or finishing an attack*/
  int running;            /*toggle for player / monsters*/
  int dashing;            /*flag for a double input!*/
  int guard;              /*when active, entity is guarding*/
  int guardstart;         /*time index when guarding began*/
  float guardstrength;    /*% damage prevented*/
  int guardrange;         /*how wide of an  area is protected*/
  int attackrange;        /*for Combat reach*/
  int attacking;          /*right arm action time*/
  int attacktype;         /*stab, slash, bash, parry*/
  int monsterdamage;      /*damage a monster will deal*/
  int damagetype;         /*bit mask for damage types*/
  int resists[D_NumTypes];/*how much of each type is absorbed*/
  float resbonus[D_NumTypes];/*temp bonuses for each resistance*/
  int attackstart;        /*time when attack initiated*/
  float attackframe;        /*for timing animations*/
  float attackspeed;      /*speed factor*/
  int takesdamage;        /*filter for combat*/
  Coord targetpoint;     /*for use in attacking*/
  int life;
  BBox box;
  float radius;           /*for collision detection*/
  int solid;              /*clips other entities*/
  int sightdist;          /*how far away I can make targets*/
  float sightrange;       /*how wide an angle I can see.  Most are 90 off of center*/
  float health;   /*damage to be taken before death*/
  float speed;            /*max movement speed*/
  float accel;            /*movement acceleration*/
  int healthmax;
  float stamina;
  int staminamax;
  float mana;
  int manamax;
  int team;
  int alert;      /*flag for if an NPC is aware of the player.*/
  int hostile;    /*flag for if an NPC is hostile towards player*/
  char data[MAXOBJKEYS][80]; /*config data for the object*/
  /*Game Specific Code follows*/
  int objindex;   /*specific object's index for the given map  Needed for persistence*/
}Entity;

void PushEntity(Entity *ent, Coord v);
void InitEntList();
void ClearEntities();
void FreeEntity(Entity *ent);
Entity *NewEntity();
void CoolDowns(Entity *self);

Entity *GetEntityByID(int id);
Entity *GetEntityByObjIndex(int id);

void updateAllEntities();
void thinkEntities(Entity *ent);
void updateEntities(Entity *ent);
Entity *GetNextEntity(Entity *last,Entity *ignore);
Entity *EntityIndex(int index);
void drawAllEntities(int drawbox,int floor);
void DrawBBox(Entity *self, float r, float g, float b);
int EntCollide(Entity *self, Entity *other);
int EntityClippingOther(Entity *self, Entity *other);
void DrawEntTrail(Entity *ent);
void ClearEntitiesExcept(Entity *skip);

float VectorAngle2D(float x,float y);
void EntLookAtOther(Entity *self,Entity *other);
Coord objMouseRayTrace(obj_read *model);
void RotateVectorX(Coord *vect, float angle);
void RotateVectorY(Coord *vect, float angle);
void RotateVectorZ(Coord *vect, float angle);
void RotatePointAroundVector( Coord *dst, Coord dir, Coord point, float degrees );
void AngleVector2D(Coord *v,float angle);
void AngleVectors (Coord *angles, Coord *forward, Coord *right, Coord *up);
void R_ConcatRotations (float in1[3][3], float in2[3][3], float out[3][3]);
void CrossProduct (Coord v1, Coord v2, Coord *cross);
void PerpendicularVector( Coord *dst, Coord src );
float VectorNormalize (Coord *v);
void ProjectPointOnPlane( Coord *dst, Coord p, Coord normal );
float RelativeSize(float x, float y);

int RayInTriangle(Coord start, Coord dir, Coord t1, Coord t2, Coord t3, Coord *contact);
int RayInPlane(Coord start, Coord dir, Coord t1, Coord t2, Coord t3, Coord *contact, Coord *normal);
int PointInTriangle(Coord point, Coord t1, Coord t2, Coord t3, Coord normal);
int RayCrossEdge(float sx,float sy,float vx, float vy,float x1, float y1,float x2, float y2, float *tx, float *ty);
int RayCrossEdgeZContact(Coord point,Coord v,float x1, float y1,float x2, float y2, Coord *contact);
int MouseRayTraceAABB(Entity *self);
Entity *FindTarget(int targetType);
void DrawModelBBox(zfmModel *model, float r, float g, float b);
int MouseRayTraceInBox(BBox box, float x, float y, float z);
void DrawBBox2(BBox box,float r, float g, float b);
void DrawBBoxFilled(BBox box,float r, float g, float b);
Coord zfmRayTrace(zfmModel *model, float x, float y, float z, float x1, float y1, float z1);
Entity *GetEntUnderMouse();


#endif

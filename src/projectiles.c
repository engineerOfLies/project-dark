#include "projectiles.h"
#include "particle.h"
#include "c_collide.h"
#include "combat.h"
#include "player.h"

extern Entity *PlayerEnt;
extern PlayerSave PDat;

void ProjectileUpdate(Entity *self)
{
  Trace trace;
  self->life--;
  if((UpdateEntityPosition(self,&trace))||(self->life <= 0))
  {
    if((trace.hittype == 1) && (trace.other != NULL))
    {
      /*do damage*/
      if(self->owner == PlayerEnt)
      {
        switch(self->attacktype)
        {
          case PR_Arrow:
            TrainAttribute(A_Str, (int)(self->health *0.2) + 1);
            TrainAttribute(A_Dex, (int)(self->health *0.1) + 1);
            break;
          case PR_Knife:
            TrainAttribute(A_Foc, (int)(self->health *0.1) + 1);
            TrainAttribute(A_Dex, (int)(self->health *0.2) + 1);
            break;
          case PR_Spell:
            TrainAttribute(A_Foc, (int)(self->health *0.2) + 1);
            TrainAttribute(A_Dex, (int)(self->health *0.1) + 1);
            break;
          case PR_Potion:
            TrainAttribute(A_Foc, (int)(self->health *0.1) + 1);
            TrainAttribute(A_Dex, (int)(self->health *0.2) + 1);
            break;
        }
      }
      if(BlockCheck(self->r.z, trace.other))
      {
        NewMsg("Blocked!");
        AddDelays(self,2);
        if(trace.other == PlayerEnt)
        {
          TrainAttribute(A_End, (int)(self->health *0.1) + 1);
          TrainAttribute(A_Agi, (int)(self->health *0.1) + 1);
          trace.other->stamina-=(self->health *0.1);
        }
      }
      else Damage(self,self->owner, trace.other,self->health, 0, 1, 0);
    }
    if(self->attackrange > 0)
    {
      RadiusDamage(self, self->p, self->attackrange, self->health, 0, self->damagetype, 0, self);
      ParticleExplosion(self->p.x,self->p.y, self->p.z, self->attackrange/4, 10 * self->attackrange, self->attackrange / 2, self->color.x, self->color.y, self->color.z, 3, 0.5);
    }
    FreeEntity(self);
    return;
  }
}

Entity *SpawnGenericProjectile(Entity *owner, Coord p, Coord v,int damage)
{
  Entity *self;
  self = NewEntity();
  if(self == NULL)
  {
    fprintf(stderr,"unable to spawn a projectile!\n");
    return NULL;
  }
  self->owner =  owner;
  self->update = ProjectileUpdate;
  self->UpdateRate = 30;
  self->radius = 4;
  self->p.x = p.x;
  self->p.y = p.y;
  self->p.z = 0;
  self->v.x = v.x;
  self->v.y = v.y;
  self->v.z = 0;
  self->r.z = owner->r.z;
  self->s.x = self->s.y = self->s.z = 1;
  self->solid = 0;
  self->health = damage;
  self->accel = 3;
  return self;
}

void ShootArrow(Entity *owner)
{
  Item *item;
  Entity *self;
  float damage;
  float life;
  Coord v = {0,0,0};
  AngleVector2D(&v,owner->r.z);
  if(owner == PlayerEnt)
  {
    if(PDat.equips[E_Arrow] == -1)return;
    item = GetItemByName(PDat.inventory[PDat.equips[E_Arrow]].name);
    if(item == NULL)return;
    /*TODO: factor in upgrades*/
    /*TODO: factor in bow*/
    /*TODO: factor in balance jitter*/
    damage = item->damage + owner->monsterdamage;
    damage += PDat.attr[A_Dex].score * 0.1;
    damage += PDat.attr[A_Str].score * 0.1;
    VectorScale(v,(8 * item->speed),v);
    life = 10 + PDat.attr[A_Str].score/2;
  }
  else
  {
    item = GetItemByName("Wood Arrow");
    if(item == NULL)return;
    damage = owner->monsterdamage;
    VectorScale(v,6,v);
    life = owner->attackrange;
  }
  self = SpawnGenericProjectile(owner, owner->p, v,damage);
  if(self == NULL)return;
  strcpy(self->name,item->name);
  self->life = life;
  self->attacktype = PR_Arrow;
  self->damagetype = D_Physical | D_Pierce;
  self->texture = LoadSprite(item->sprite,32,32);
}

void ThrowKnife(Entity *owner)
{
  Item *item;
  float damage = 0;
  Entity *self;
  Coord v = {0,0,0};
  if(PDat.equips[E_Tool] == -1)return;
  item = GetItemByName(PDat.inventory[PDat.equips[E_Tool]].name);
  if(item == NULL)return;
  AngleVector2D(&v,owner->r.z);
  VectorScale(v,(7 * item->speed),v);
  damage = item->damage;
  damage += PDat.attr[A_Dex].score * 0.1;
  damage += PDat.attr[A_Str].score * 0.1;
  /*TODO: factor in upgrades*/
  /*TODO: factor in bow*/
  /*TODO: factor in balance jitter*/
  self = SpawnGenericProjectile(owner, owner->p, v,damage);
  if(self == NULL)return;
  strcpy(self->name,item->name);
  self->damagetype = D_Physical | D_Pierce;
  self->attacktype = PR_Knife;
  self->life = 25 + PDat.attr[A_Dex].score/2;
  self->texture = LoadSprite(item->sprite,32,32);
  if(owner == PlayerEnt)
  {
    if(PlayerHasSkill("Knife Throwing"))
    {
      damage += PDat.attr[A_Dex].score * 0.1;
      self->life *= 1.5;
    }
  }
}


void ShootForceBolt(Entity *owner)
{
  char *b;
  P_Skill *spell;
  float damage;
  Entity *self;
  Coord v = {0,0,0};
  spell = GetSpellByName(PDat.spells[PDat.chosenspell]);
  AngleVector2D(&v,owner->r.z);
  VectorScale(v,8.5,v);
  damage = GetSpellDamage(spell);
  self = SpawnGenericProjectile(owner, owner->p, v,damage);
  if(self == NULL)return;
  strcpy(self->name,spell->name);
  self->attacktype = PR_Spell;
  self->damagetype = D_Magic | D_Pierce;
  self->life = 20 + PDat.attr[A_Foc].score/2;
  self->texture = LoadSprite( spell->sprite,32,32);
  b = GetSpellSpecialData(spell,"radius");
  if(b != NULL)
  {
    self->attackrange = GetMagicDegree(b);
  }
  b = GetSpellSpecialData(spell,"color");
  if(b != NULL)
  {
    sscanf(b,"%f %f %f",&self->color.x,&self->color.y,&self->color.z);
  }
}

void ThrowPotion(Entity *owner,char *potion)
{
  char *k;
  Item *item;
  int r;
  float damage = 0;
  Entity *self;
  Coord v = {0,0,0};
  item = GetItemByName(potion);
  if(item == NULL)return;
  AngleVector2D(&v,owner->r.z);
  VectorScale(v,(7 * item->speed),v);
  damage = item->damage;
  damage += PDat.attr[A_Dex].score * 0.1;
  damage += PDat.attr[A_Foc].score * 0.1;
  /*TODO: factor in upgrades*/
  /*TODO: factor in bow*/
  /*TODO: factor in balance jitter*/
  self = SpawnGenericProjectile(owner, owner->p, v,damage);
  if(self == NULL)return;
  strcpy(self->name,item->name);
  self->attacktype = PR_Potion;
  self->life = 30 + PDat.attr[A_Dex].score/2;
  self->damagetype = D_Physical | D_Pierce;
  self->texture = LoadSprite(item->sprite,32,32);
  VectorScale(self->s,0.5,self->s);
  k = GetItemSpecialKey(item,"radius");
  if(k != NULL)
  {
    sscanf(k,"%i",&r);
    self->attackrange = r;
  }
  k = GetItemSpecialKey(item,"color");
  if(k != NULL)
  {
    sscanf(k,"%f %f %f",&self->color.x,&self->color.y,&self->color.z);
  }
}


int Ballistic(Entity *self)
{
  self->v.x *= WDAMP;
  self->v.y *= WDAMP;
  if(CoordLength(self->v) < self->accel)
  {
    self->v.x = 0;
    self->v.y = 0;
    return 1;
  }
  else
  {
    
  }
  return 0;
}


/*eol@eof*/

#include "combat.h"
#include "particle.h"
#include "monsters.h"
#include <chipmunk/chipmunk.h>

extern Uint32 NOW;
extern Entity *PlayerEnt;

void Damage(Entity *atk,Entity *owner, Entity *def,float damage, int dtype, int penetration, float kick)
{
  int i;
  if(def == NULL)return;
  if(def->takesdamage == 0)return;
  for(i = 0;i < D_NumTypes;i++)
  {
    if((def->resists[i] + def->resbonus[i]) > 0)
    {
      if(dtype & GetResistanceMask(i))
      {
        damage -= (def->resists[i] + def->resbonus[i]);
        if(damage < 1)damage = 1;
      }
    }
  }
  AddDelays(def,2);
  if(def->hit != NULL)
  {
    def->hit(def,atk);/*bad touch*/
  }
  def->health -= damage;
  NewMessage("Damage!",IndexColor(Red));
  if(def->health <= 0)
  {
    /*run death function, or remove entity*/
    NewMessage("Dead!",IndexColor(Red));
    def->takesdamage = 0;
    def->solid = 0;
    if(def->die != NULL)
    {
      def->die(def,owner);
    }
    else 
    {
      def->state = MS_Dead;
      FreeEntity(def);
    }
    return;
  }
}

int BackCheck(float AOA, Entity *def)
{
  int angle;
  cpVect v1,v2;
  v1 = cpv(cpfcos(AOA), cpfsin(AOA));
  v2 = cpv(cpfcos(def->r.z), cpfsin(def->r.z));
  angle = AngleBetweenVectors2D(M_Coord(v1.x,v1.y,0),M_Coord(v2.x,v2.y,0));
  if(angle < 10)return 1;
  return 0;
}
/*Angle of Attack*/
int BlockCheck(float AOA, Entity *def)
{
  float angle;
  /*shouldn't happen*/
  if(def == NULL)return -1;
  if(!def->guard)return 0;
  angle = def->r.z - AOA;
  angle = fabs(angle);
  if(angle >= 360)angle -= 360;
  angle -= 180;
  angle = fabs(angle);
  /*substitute for shield coverage*/
  if(angle < 40)
  {
    /*may have blocked*/
    if((NOW - def->guardstart) > 60)
    {
      /*ok blocked*/
      /*but did we parry?*/
      return 1;
    }
  }
  return 0;
}

void Hit(Entity *self,float AOA, Trace trace,float damage,int dtype)
{
  if(BlockCheck(AOA, trace.other))
  {
    NewMessage("BLOCKED!",IndexColor(LightYellow));
    AddDelays(self,4);
    if(trace.other == PlayerEnt)
    {
      TrainAttribute(A_End, (int)(damage *0.1) + 1);
      TrainAttribute(A_Agi, (int)(damage *0.1) + 1);
    }
    trace.other->stamina-=(damage *0.2);
    damage *= trace.other->guardstrength;
    if(trace.other->stamina <= 0)
    {
      trace.other->Acd += (damage *0.2);
      trace.other->guard = 0;
    }
    else trace.other->Acd += (damage *0.1);
  }
  else
  {
    NewMessage("HIT!",IndexColor(LightRed));
    if((PlayerHasSkill("Back Attack"))&&(BackCheck(AOA, trace.other)))
    {
      NewMsg("Back Attack!");
      damage *= 2;
    }
    NewMessage("HIT!",IndexColor(LightRed));
    if(self == PlayerEnt)
    {
      TrainAttribute(A_Str, (int)(damage *0.1) + 1);
      TrainAttribute(A_Dex, (int)(damage *0.1) + 1);
    }
  }
  Damage(self,NULL, trace.other,damage, dtype, 0, 1);
}

int Stab(Entity *self,Coord p,Coord t,float range,float thick, float step, float damage, int penetration,int dtype)
{
  float AOA;
  Trace trace;
  Coord v = {0,0,0};
  if(self == NULL)return -1;
  v.x = t.x - p.x;
  v.y = t.y - p.y;
  AOA = 90-VectorAngle2D(v.x,v.y);
  VectorScale(v,step,v);
  /*DropParticle(M_Coord(p.x + v.x,p.y + v.y,0),M_Coord(1,0,1),180);*/
  if(GTrace(p,v, thick, F_TDAMAGE | F_SOLID, &trace, self))
  {
    switch(trace.hittype)
    {
      case HT_Wall:
        NewMessage("This is going to dull the weapon!",IndexColor(LightGreen));
        break;
      case HT_Entity:
        Hit(self,AOA,trace,damage,dtype);
        break;
      case HT_Bounds:
        NewMessage("Attacking the Darkness",IndexColor(LightBlue));
        break;
    }
    return 1;
  }
  if(range == 1)
  {
    NewMessage("A Swing and a Miss!",IndexColor(LightRed));
  }
  return 0;
}

int Crush(Entity *self,Coord p,Coord t,float range,float angle1, float angle2, float thick, float step, float damage, int penetration,int dtype)
{
  Coord v = {0,0,0};
  AngleVector2D(&v,self->r.z - 90);
  VectorScale(v,self->radius * 0.9,v);
  p.x += v.x;
  p.y += v.y;
  return Stab(self,p,t,range,thick, step, damage, penetration,dtype);
}


int Slash(Entity *self,Coord p,Coord t,float range,float angle1, float angle2, float thick, float step, float damage, int penetration,int dtype)
{
  float AOA;
  Trace trace;
  Coord v = {0,0,0};
  Coord start = {0,0,0};
  Coord end = {0,0,0};
  if(self == NULL)return -1;
  v.x = t.x - p.x;
  v.y = t.y - p.y;
  RotateVectorZ(&v, angle1);
  start.x = p.x + v.x;
  start.y = p.y + v.y;
  RotateVectorZ(&v,((angle2 - angle1)*step));
  AOA = self->r.z + angle1 + ((angle2 - angle1)*step);
  end.x = p.x + v.x;
  end.y = p.y + v.y;
  v.x = end.x - start.x;
  v.y = end.y - start.y;
  if(Stab(self,p,end,range,thick, 1,damage, penetration, dtype))
  {
    return 1;
  }
  /*DropParticle(end,M_Coord(1,1,0),180);*/
  if(GTrace(start,v, thick, F_TDAMAGE | F_SOLID, &trace, self))
  {
    switch(trace.hittype)
    {
      case HT_Wall:
        NewMessage("This is going to dull the weapon!",IndexColor(LightGreen));
        break;
      case HT_Entity:
        Hit(self,AOA,trace,damage,dtype);
        break;
      case HT_Bounds:
        NewMessage("Attacking the Darkness",IndexColor(LightBlue));
        break;
    }
    return 1;
  }
  if(range == 1)
  {
    NewMessage("A Swing and a Miss!",IndexColor(LightRed));
  }
  return 0;
}

Uint32 GetResistanceMask(int type)
{
  switch(type)
  {
    case D_Physical:
      return MT_Physical;
    case D_Pierce:
      return MT_Pierce;
    case D_Slash:
      return MT_Slash;
    case D_Crush:
      return MT_Crush;
    case D_Magic:
      return MT_Magic;
    case D_Fire:
      return MT_Fire;
    case D_Ice:
      return MT_Ice;
    case D_Elec:
      return MT_Elec;
    case D_Shadow:
      return MT_Shadow;
    case D_Light:
      return MT_Light;
  }
  return 0;
}

char *GetResistanceType(int type)
{
  switch(type)
  {
    case D_Physical:
      return "Physical";
    case D_Pierce:
      return "Pierce";
    case D_Slash:
      return "Slash";
    case D_Crush:
      return "Crush";
    case D_Magic:
      return "Magic";
    case D_Fire:
      return "Fire";
    case D_Ice:
      return "Ice";
    case D_Elec:
      return "Lightning";
    case D_Shadow:
      return "Shadow";
    case D_Light:
      return "Holy";
  }
  return " ";
}

void Shoot();

void RadiusDamage(Entity *attacker, Coord p, float radius, float damage, int penetration, int dtype, float kick, Entity *ignore)
{
  float factor = 1;
  Entity *ent = NULL;
  if(attacker == NULL)return;
  ent = GetNextEntity(ent,attacker);
  if(radius == 0)radius = 1;
  while(ent != NULL)
  {
    if(ent->used)
    {
      if(ent->takesdamage)
      {
        /*check for walls between this entity and the attacker*/
        if(RelativeSize(attacker->p.x - ent->p.x,attacker->p.y - ent->p.y) <= (radius * radius))
        {
          NewMsg("Radius Damage!");
          factor = RelativeSize(attacker->p.x - ent->p.x,attacker->p.y - ent->p.y) / (radius * radius);
          Damage(attacker,attacker->owner, ent,damage * factor, dtype, penetration, kick);
        }
      }
    }
    ent = GetNextEntity(ent,attacker);
  }
}

void AddDelays(Entity *self,int amount)
{
  self->state = MS_Stunned;
  self->stun = amount;
  self->guard = 0;
  self->attacking = 0;
}


void ApplyStandardDelays(Entity *self,float speed,int left)
{
  if(self == NULL)return;
  if(left)  self->Lcd = 5 + (15 * speed);
  else self->Rcd = 5 + (15 * speed);
  self->Acd = 5 + (10 * speed);  /*same*/
  self->Gcd = 5 + (10 * speed);  /*same*/
}



/*eol@eof*/

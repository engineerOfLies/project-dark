#ifndef __COMBAT__
#define __COMBAT__

#include "gtrace.h"
#include "entity.h"
#include "levelobj.h"


/*slash and stab are called every frame from the begining of the attack through the end of the attack.  based on the stats of the weapon and the player, it will take longer or shorter to reach the full range.
Returns 1 when it makes a connection*/
int Stab(Entity *self,Coord p,Coord t,float range,float thick, float step, float damage, int penetration, int dtype);
int Crush(Entity *self,Coord p,Coord t,float range,float angle1, float angle2, float thick, float step, float damage, int penetration,int dtype);
/*angle1 and angle 2 are the angles off of center (where the player is facing)*/
int Slash(Entity *self,Coord p,Coord t,float range,float angle1, float angle2, float thick, float step, float damage, int penetration,int dtype);
void Shoot();
void Damage(Entity *atk,Entity *owner, Entity *def,float damage, int dtype, int penetration, float kick);
void RadiusDamage(Entity *attacker, Coord p, float radius, float damage, int penetration, int dtype, float kick, Entity *ignore);
int BackCheck(float AOA, Entity *def);
int BlockCheck(float AOA, Entity *def);
char *GetResistanceType(int type);
Uint32 GetResistanceMask(int type);
void ApplyStandardDelays(Entity *self,float speed,int left);
void AddDelays(Entity *self,int amount);

#endif

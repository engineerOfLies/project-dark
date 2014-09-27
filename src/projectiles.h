#ifndef __PROJECTILES__
#define __PROJECTILES__

#include "entity.h"
#include "levelobj.h"

enum PR_Type {PR_Arrow,PR_Knife,PR_Spell,PR_Potion};

void ShootForceBolt(Entity *owner);
void ShootArrow(Entity *owner);
int Ballistic(Entity *self);
void ThrowKnife(Entity *owner);
void ThrowPotion(Entity *owner,char *potion);

#endif

#ifndef __MONSTERS__
#define __MONSTERS__

#include "entity.h"
#include "gtrace.h"
#include "levels.h"
#include "player.h"
#include "combat.h"

#define MONSTERMAX 128

/*

    Monster combat Behavior's for near and far actions
    1 - perform action 1 (if in range, otherwise get in rage)
    2 - perform action 2
    O - Opportune Attack Look for player opening before attacking (if in range)
    F - Get Far - for rannged attackers
    N - Get Near - for melee attackers
    D - Defend - Hold Up Guard
    
    Sample:
    Near: AGGGAGGGAGGGGGA
    Far : N
*/


typedef struct
{
  char  name[80];
  char  sprite[80];
  int   sx,sy;                /*sprite frame dimensions*/
  float scale;                /*of the monster's sprite*/
  float radius;               /*collision radius*/
  int   gold;                 /*amount of gold that can be dropped*/
  int   goldchance;            /*this much addittional gold is possible, but random*/
  int   resists[D_NumTypes];
  int   align,tend;           /*chaos/order, Inner/Outer*/
  int   randombehavior;       /*if behaviors are chosen at random, otherwise it is gone in order*/
  char  nearbehavior[80];     /*sequence of actions that a monster would take while near*/
  char  farbehavior[80];      /*sequence of actions that a monster would take while near*/
  int   health,stamina,mana;  /*monster's maxes*/
  char  drops[4][80];         /*items that MAY be dropped when the monster dies.  Increased rarity for higher numbers*/
  float droprate;             /*monsters with a drop rate of 1 will ALWAYS drop the first item.*/
  int   grace;                /*the amount of grace that this monster is worth*/
  char  actions[2][80];       /*things a monster will do: stab, bash, cast*/
  int   range[2];             /*range of actions (if block, its the size of the blocking area (in arc
  degrees*/
  int   damage[2];            /*damages for actions*/
  int   dtype[2];             /*damage types for actions*/
  float movespeed;
  float attackspeed;          /*overall movement factor.  applied the normal speed of an action*/
  float guardstrength;        /*when guarding, how effective it is*/
  int   guardrange;           /*size of protected area*/
  int   sightdistance;        /*distance from monster where it can see*/
  int   sightrange;            /*degrees off of center.*/
  float darkmod;              /*sight modifier at night*/
  char  description[512];     /*description of the monster, to appear in beastiary*/
}Monster;

void SpawnMonsterGeneric(int x, int y,int r,Object *obj);
int GetInRange(Entity *self);   /*move to range of target, return 1 when in range*/
void LoadMonsterList();
Monster *GetMonsterByName(char name[80]);
void MonsterDie(Entity *self,Entity *other);
void UpdateMonster(Entity *self);
void MonsterThink(Entity *self);
void MonsterAlert(Entity *self, Entity *other);
void SetMonsterCombatStats(Entity *self,Monster *monster,int action);
void MonsterPath(Entity *self, Entity *target);


#endif

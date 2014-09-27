#ifndef __NPCS__
#define __NPCS__
#include "monsters.h"
#include "menus.h"

#define MAXNPCS 64

/*

NPC info struct

NPCS are an extention of Monsters.

*/


/*
  Quest Conditions Include:
  Offer:
  player has <alignment>
  player has <tendency>
  player has <skill>
  player has <spell>
  player has <item>
  <NPC> is alive
  <NPC> is dead
  <quest> is complete / incomplete
  Win:
  kill <monster> on <map>
  give <item> to <NPC>
  do <action> to <object>
  Lose:
  <NPC> dies (ie the original person who requested the task)
  <item> is lost (ie: map is left while item is on the ground)
*/

typedef struct
{
  int  used;              /*if this entry is used*/
  char name[80];          /*name of quest*/
  char desc[512];         /*text to display for the player when offered*/
  char reward[4][40];     /*what the playe gets when he wins*/
  char offercond[4][80];  /*conditions that must be met in order for the quest to be offered*/
  char failcond[4][80];   /*conditions that a player could fail a quest*/
  char wincond[4][80];    /*conditions that a player must meet in order to win the quest.*/
}Quest;

typedef struct
{
  int   used;
  char  name[80];         /*item name sold, skill or spell taught*/
  char  type[16];         /*item, skill or spell*/
  float markup;           /*percent change in price*/
  char  condition[4][80]; /*condition that must be met for the service to be offered.*/
}ShopItem;

typedef struct
{
  int  used;
  char topic[32];
  char dialog[256];
  char dialogcond[80];
}Dialog;

typedef struct
{
  char name[80];          /*NPC's unique name this will be the same as the monster info*/
  char monster[80];       /*the monster that serves as the basis for this NPC*/
  Quest quests[8];        /*quests offered by this NPC*/
  ShopItem shop[16];      /*if the NPC sells stuff*/
  Dialog dialog[8];       /*up to 8 different messages can be set up for an NPC to say*/
}NPC;

void LoadNPCList();
NPC *GetNPCByName(char name[80]);
void SpawnNPCGeneric(int x, int y,int r,Object *obj);
int GetDialogCount(NPC *npc);   /*determines how many points of conversation are currently available*/
int GetShopCount(NPC *npc);     /*same, but with shop items*/
int GetQuestCount(NPC *npc);    /*same, but with quests*/
int IsConditionMet(char cond[80]);
#endif

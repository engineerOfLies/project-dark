#ifndef __PLAYER__
#define __PLAYER__

#include "entity.h"
#include "levels.h"
#include "combat.h"
#include "items.h"

#define VERSION 0

#define STOCKMAX    1024
#define INVENMAX    256
#define SKILLMAX    16
#define SPELLMAX    32
#define MUTATIONS   10

enum ATTRIBUTES {A_Str, A_End, A_Dex, A_Agi, A_Foc, A_Spi};
enum STATUS {ST_Poison, ST_Sickness, ST_Bleed, ST_Burn, ST_Slow};

typedef struct
{
  char name[80];
  int  attributes[6];
  int  align, tend;
  char mutations[MUTATIONS][80];
  char  description[512];
}P_Race;

typedef struct
{
  char name[80];
  int  attributes[6];
  int  align, tend;
  char skills[5][80];     /*starting skills*/
  char spells[5][80];     /*starting spells*/
  char equip[EQUIPSLOTS][80]; /*starting equipment*/
  char items[5][80];              /*starting items*/
  int  itemcount[5];              /*how many of each item*/
  char  description[512];
}P_Train;

typedef struct
{
  char name[80];
  char icon[80];   /*path to icon*/
  char sprite[80];  /*if applicable, ie: projectile spells*/
  char type[16];   /*combat, rogue, mage, necro, holy, alch*/
  int  price;       /*for learning this skill  Special skills will be Quest Rewards*/
  int  range;       /*for Projectiles and radius effects*/
  int  cost[6];          /*use cost H/S/M/ body / herb / alch base*/
  char special[16][80];  /*other features of this item*/
  char  description[512];
}P_Skill;

typedef struct S_Prog
{
  char map[80];         /*the map where this progress item applies*/
  char name[80];        /*unique name of the progress item*/
  int  index;           /*index of item in the map to search by*/
  char value[80];       /*formatted information about the progress*/
  struct S_Prog *next;  /*linking this list up*/
}Progress;

typedef struct LevelHist_S
{
  char name[80];            /*the level's searchable name*/
  int  lsize;               /*number of tiles in this level*/
  int  w,h;                 /*size in tiles*/
  int  *ldata;              /*pointer to raw data*/
  Progress *progress;        /*progress saved on a per level basis*/
  struct LevelHist_S *next;
}LevelHist;

typedef struct
{
  int score;
  int train;
}Attribute;

typedef struct
{
  int inuse;
  char name[80];
  int  index;           /*its spot in the array*/
  int  instock;         /*if its in the player's inventory or stockpile*/
  int  count;           /*how many of this exact the player has*/
  int  upgrades[10];    /*no more than 10 upgrdes for any item*/
}P_Item;

typedef struct
{     /*persistent information about the player for loading nad saving*/
  int  Version;                   /*won't load older or newer game version saves*/
  char name[80];                  /*player name*/
  char race[80];                  /*player's race*/
  char train[80];                 /*player's starting training class*/
  int  gold;                      /*player's acquired gold*/
  int  corruption;                /*player's corruption level*/
  char mutations[10][80];         /*mutations acquired*/
  char map[80];                   /*where the player is*/
  int  day, minute;               /*Time index for the player*/
  int  x,y;                       /*player starting point*/
  int  H,HM,S,SM,M,MM;            /*player's health, etc*/
  int  speed;                     /*movement speed modifier*/
  int  carry;                     /*How much a player IS carrying*/
  int  carrymax;                  /*how much a player can carry*/
  int  alignment;                 /*Order, Chaos, Balance*/
  int  tendency;                  /*Inner, Outer, Neutral*/
  int  aligncount;                /*aligment of the top 100 deeds*/
  int  align[100];                /*average determines alignment*/
  int  tendcount;                 /*same for tendency*/
  int  tend[100];                 /*""*/
  Attribute attr[6];              /*6 primary attributes*/
  int  numskills;                 /*how many have been learned*/
  char skills[SKILLMAX][80];      /*skills learned by the player*/
  int  chosenspell;               /*which spell is selected to cast*/
  int  numspells;                 /*how many spells known*/
  char spells[SPELLMAX][80];      /*skills learned by the player*/
  int  numvisited;                /*number of levels visited*/
  int  inventorycount;            /*how much crap is in the player's inventory*/
  P_Item inventory[STOCKMAX];     /*Items in the player's stockpile and inventory*/
  int  equips[EQUIPSLOTS];        /*equipped items, index of item in inventory, -1 is nothing*/
  int  ailment[6];                /*how much of each status ailment is affecting the player*/
  int  ailres[6];                 /*resistances to ailments*/
  int  resist[D_NumTypes];        /*resistance to different types of damage*/
  int  grace;                     /*xp to spend on attributes*/
  int   sightdistance;            /*distance from monster where it can see*/
  int   sightrange;                /*degrees off of center.*/
  float darkmod;                  /*sight modifier at night*/
  LevelHist *levelhistory;        /*the seen mask for each level*/
}PlayerSave;

void ClearPlayer();
void GivePlayerAttributes(int attr[6]);
void TrainAttribute(int i, int train);
void UseItem(P_Item *pitem);
int GetGraceLevel();
int GetPlayerGrace();
void SpendPlayerGrace(int x);
int GetPlayerAttributeScore(int i);
void RaisePlayerAttribute(int i);
int CalcGraceCost(int at);
void GivePlayerGrace(int count);
void ClearLevelHistory();
LevelHist *NewLevelHistory(char name[80], int size,int x,int y);
LevelHist *GetLevelHistoryByName(char name[80]);
int GetLevelHistoryCount();
void SelectLevelHistoryByName(char name[80]);
void DrawLevelUnknown();
void UpdatePlayerCamera();
void SpawnPlayer(Coord p);
void NewPlayer(char race[80],char train[80]);
void SavePlayerGame();
int LoadPlayerGame(char *name);
void SetupPlayer(int x,int y);
void SetupPlayerSprite(Entity *self);
void UpdateSeenMask(int sightrange);
void GivePlayerSkill(char name[80]);
void GivePlayerSpell(char name[80]);
int  GivePlayerItem(char name[80]);
int PlayerHasSkill(char name[80]);
int SkillCheck(char skill[80],int bonus,int dif);
void TakePlayerItemByName(char name[80]);
void TakePlayerItem(P_Item *pitem);
P_Item *GetNextPlayerItemByPlace(P_Item *last,int stock);
int  IsItemEquipped(P_Item *pitem);
int GetEquippedItemSlot(P_Item *pitem);
int GetPlayerInventoryCount(int stockpile);
void EquipPlayerItem(int index,int where);
void LoadSkillList();
void LoadSpellList();
void LoadTrainingList();
void LoadRaceList();
int GetRaceCount();
P_Race *GetRaceByName(char name[80]);
P_Race *GetRaceByIndex(int id);
char *GetAttributeName(int i);
int GetAttributeIndex(char name[80]);
P_Train *GetTrainByIndex(int id);
int GetPlayerItemCount(char name[80]);
P_Item *GetPlayerItemByName(char name[80]);
P_Item *GetPlayerItemByN(int n, int stockpile);
int GetTrainingCount();
void CalcPlayerStats();
void CalcPlayerResistance();
void GivePlayerGold(int count);
void SpendArrow();
void SpendTool();
int  GetItemEquipmentSpot(int index);
void ToggleWeapon(int first,int second);
void CastingCheck(Item *item,int next);
void SelectNextSpellofType(char type[16]);
void SelectNextSpell();
void SelectNextTool();
char *GetSpellSpecialData(P_Skill *spell,char tag[80]);
float GetSpellDamage(P_Skill *spell);/*determines damage dealt or health returned*/
float GetMagicDegree(char degree[80]);
P_Skill *GetSpellByName(char name[80]);
P_Skill *GetSkillByName(char name[80]);
void SetPlayerProgress(char map[80],char name[80], int index,char key[80]);  /*sets it if exists, or makes it and sets it*/
int GetLevelProgressCount(LevelHist *lh);
Progress *GetProgressByQuestName(char quest[80]); /*this one is SLOW*/
Progress *GetProgressByIndexMap(char map[80],int index);
Progress *GetProgressByNameMap(char map[80],char name[80]);
Progress *GetProgressByNameIndexMap(char map[80],char name[80],int index);/*some id's can offer multiple progress items by name*/
void DrawPlayerInventory(int x,int y, int from, int total);/*Draws the player inventory text.  used by different menus.*/
void DrawPlayerResistance(int x,int y);
void DrawPlayerEquipment(int x,int y);
void DrawPlayerState(int x, int y);
void DrawPlayerSpells(int x, int y);
void DrawPlayerSkills(int x, int y);
int  SpellHasSpecial(P_Skill *spell,char tag[80]);
int  SkillHasSpecial(P_Skill *spell,char tag[80]);
int CanSeePoint(Coord s, Coord g, int targetrad, Entity *ignore);
#endif

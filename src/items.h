#ifndef __ITEMS__
#define __ITEMS__

#include "entity.h"

#define EQUIPSLOTS  12
#define MAXITEMS    512

enum EQUIPS {E_Head,E_Body,E_Neck,E_Legs, E_Ring1, E_Ring2, E_Primary1,E_Primary2, E_Secondary1, E_Secondary2, E_Arrow, E_Tool};

typedef struct
{
  char name[80];
  char icon[80];        /*where the icon of the item lives*/
  char sprite[80];      /*where the animated sprite of the item lives*/
  int  equips;          /*where it equips*/
  int  attributes[6];     /*min attributes*/
  int  cost;            /*buy price*/
  int  damage;          /*amount of damage dealt/absorbed*/
  int  balance;         /*bonus to hit / dodge*/
  int  upgradable;      /*if it will take  upgrades*/
  int  sellable;        /*if it will sell or not*/
  int  stackable;       /*if the item will stack in inventory*/
  int  range;           /*how far this will reach for a stab / projectile*/
  float speed;          /*modifier for action speeds*/
  float weight;         /*weight of 1 of this item*/
  char actions[3][80];  /*primary and seconday use of this item.  third slot is what happens if a req skill is not available*/
  char special[8][80];  /*other features of this item*/
  char description[256];
}Item;

char *GetEquipSlotName(int i);
void LoadItemList();
Item *GetItemByName(char name[80]);
int  ItemHasSpecial(Item *item,char tag[80]);
int  ItemHasAction(Item *item,char tag[80]);
char *GetItemSpecialKey(Item *item,char tag[80]);
Entity *SpawnItemDrop(int x, int y,char name[80],int count);

#endif

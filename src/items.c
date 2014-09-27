#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "items.h"
#include "player.h"

extern Entity *PlayerEnt;
int NumItems = 0;
Item ItemList[MAXITEMS];

Item *GetItemByName(char name[80])
{
  int i;
  for(i = 0;i < NumItems;i++)
  {
    if(strcmp(ItemList[i].name,name)==0)
    {
      return &ItemList[i];
    }
  }
  return NULL;
}

char *GetItemSpecialKey(Item *item,char tag[80])
{
  char *b;
  int i;
  char buf[16];
  if(item == NULL)return NULL;
  for(i = 0;i < 8;i++)
  {
    if(strlen(item->special[i]) > 1)
    {
      sscanf(item->special[i],"%s",buf);
      if(strcmp(buf,tag)==0)
      {
        b = strchr(item->special[i], ' ');
        if(b != NULL)
        {
          b++;
          return b;
        }
        else return NULL;/*found it, but it doesn't have any extra info*/
      }
    }
  }
  return NULL;
}

int  ItemHasSpecial(Item *item,char tag[80])
{
  int i;
  char buf[16];
  if(item == NULL)return -1;
  for(i = 0;i < 8;i++)
  {
    if(strlen(item->special[i]) > 1)
    {
      sscanf(item->special[i],"%s",buf);
      if(strcmp(buf,tag)==0)
      return 1;
    }
  }
  return 0;
}

int  ItemHasAction(Item *item,char tag[80])
{
  int i;
  if(item == NULL)return -1;
  for(i = 0;i < 3;i++)
  {
    if(strcmp(item->actions[i],tag)==0)
      return 1;
  }
  return 0;
}

void UseItem(P_Item *pitem)
{
  Item *item;
  if(pitem == NULL)return;
  item = GetItemByName(pitem->name);
  if(ItemHasSpecial(item,"heal"))
  {
    PlayerEnt->health += item->damage;
    if(PlayerEnt->health > PlayerEnt->healthmax)PlayerEnt->health = PlayerEnt->healthmax;
  }
  if(ItemHasSpecial(item,"mana"))
  {
    PlayerEnt->mana += item->damage;
    if(PlayerEnt->mana > PlayerEnt->manamax)PlayerEnt->mana = PlayerEnt->manamax;
  }
  TakePlayerItem(pitem);
}

void LoadItemList()
{
  int i = 0;
  int j = 0;
  int k = 0;
  FILE *file;
  char *c;
  char buf[512];
  file = fopen("system/itemlist.def","r");
  if(file == NULL)
  {
    fprintf(stderr,"Unable to open race file\n");
    return;
  }
  memset(&ItemList,0,sizeof(ItemList));
  i = 0;
  j = 0;
  k = 0;
  NumItems = 0;
  while(fscanf(file, "%s", buf) != EOF)
  {
    if(strcmp(buf,"#") ==0)
    {
      fgets(buf, sizeof(buf), file);
      continue;/*ignore the rest of the line.*/
    }
    if(strcmp(buf,"<end>") ==0)
    {
      i++;
      NumItems++;
      j = k = 0;
      if(i >= MAXITEMS)
      {
        fprintf(stderr,"maximum number of items loaded from file!\n");
        return;
      }
      continue;
    }
    if(strcmp(buf,"<item>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(ItemList[i].name, 80, file);
      c = strchr(ItemList[i].name, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<sprite>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(ItemList[i].sprite, 80, file);
      c = strchr(ItemList[i].sprite, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<icon>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(ItemList[i].icon, 80, file);
      c = strchr(ItemList[i].icon, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<desc>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(ItemList[i].description, 256, file);
      c = strchr(ItemList[i].description, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<requires>") ==0)
    {
      fscanf(file, "%i %i %i %i %i %i",&ItemList[i].attributes[0],&ItemList[i].attributes[1], &ItemList[i].attributes[2], &ItemList[i].attributes[3], &ItemList[i].attributes[4],&ItemList[i].attributes[5]);
      continue;
    }
    if(strcmp(buf,"<cost>") ==0)
    {
      fscanf(file, "%i",&ItemList[i].cost);
      continue;
    }
    if(strcmp(buf,"<damage>") ==0)
    {
      fscanf(file, "%i",&ItemList[i].damage);
      continue;
    }
    if(strcmp(buf,"<equips>") ==0)
    {
      fscanf(file, "%i",&ItemList[i].equips);
      continue;
    }
    if(strcmp(buf,"<balance>") ==0)
    {
      fscanf(file, "%i",&ItemList[i].balance);
      continue;
    }
    if(strcmp(buf,"<upgradable>") ==0)
    {
      fscanf(file, "%i",&ItemList[i].upgradable);
      continue;
    }
    if(strcmp(buf,"<sellable>") ==0)
    {
      fscanf(file, "%i",&ItemList[i].sellable);
      continue;
    }
    if(strcmp(buf,"<stackable>") ==0)
    {
      fscanf(file, "%i",&ItemList[i].stackable);
      continue;
    }
    if(strcmp(buf,"<range>") ==0)
    {
      fscanf(file, "%i",&ItemList[i].range);
      continue;
    }
    if(strcmp(buf,"<speed>") ==0)
    {
      fscanf(file, "%f",&ItemList[i].speed);
      continue;
    }
    if(strcmp(buf,"<weight>") ==0)
    {
      fscanf(file, "%f",&ItemList[i].weight);
      continue;
    }
    if(strcmp(buf,"<actions>") ==0)
    {
      if(j < 3)
      {
        fgetc(file);  /*clear the space before the word*/
        fgets(ItemList[i].actions[j], 80, file);
        c = strchr(ItemList[i].actions[j], '\n');
        /*replace trailing return with terminating character*/
        if(c != NULL) *c = '\0';
        j++;
      }
      else
      {
        fgets(buf, sizeof(buf), file);
      }
      continue;
    }
    if(strcmp(buf,"<special>") ==0)
    {
      if(k < 8)
      {
        fgetc(file);  /*clear the space before the word*/
        fgets(ItemList[i].special[k], 80, file);
        c = strchr(ItemList[i].special[k], '\n');
        /*replace trailing return with terminating character*/
        if(c != NULL) *c = '\0';
        k++;
      }
      else
      {
        fgets(buf, sizeof(buf), file);
      }
      continue;
    }
  }
}


char *GetEquipSlotName(int i)
{
  switch(i)
  {
    case E_Head:
      return "Head";
    case E_Body:
      return "Body";
    case E_Legs:
      return "Legs";
    case E_Neck:
      return "Neck";
    case E_Ring1:
      return "Ring";
    case E_Ring2:
      return "Ring";
    case E_Primary1:
      return "Primary Hand";
    case E_Primary2:
      return "Primary Alternate";
    case E_Secondary1:
      return "Secondary Hand";
    case E_Secondary2:
      return "Secondary Alternate";
    case E_Arrow:
      return "Quiver";
    case E_Tool:
      return "Tool";
  }
  return " ";
}

/*Item Pick up section*/

void PickUpItem(Entity *self,Entity *other)
{
  Item *item;
  int i;
  char text[80];
  if(self == NULL)return;
  if(other == NULL)return;
  if(self->owner != NULL)
  {
    if(self->owner->activate != NULL)
    {
      self->owner->activate(self->owner,self);
    }
  }
  else if(GetObjKey(self->data,"item",NULL))
  {
    SetPlayerProgress(GetLevelName(),"item", self->objindex,"taken");
  }
  if(strcmp(other->name,"player")==0)
  {
    if(strcmp(self->name,"gold")==0)
    {
      /*check for gold special case*/
      GivePlayerGold(self->health);
      sprintf(text,"Found %i Gold!",(int)self->health);
      NewMessage(text,IndexColor(Gold));
    }
    else
    {
      item = GetItemByName(self->name);
      if(item != NULL)
      {
        if(self->health > 1)
        {
          sprintf(text,"Found %i %ss!",(int)self->health,self->name);
        }else sprintf(text,"Found a %s!",self->name);
        NewMsg(text);
        for(i = 0;i < (int)self->health;i++)
        {
          GivePlayerItem(self->name);
        }
      }
    }
  }
  FreeEntity(self);
}

Entity *SpawnItemDrop(int x, int y,char name[80],int count)
{
  Entity *self;
  self = NewEntity();
  if(self == NULL)return NULL;
  strcpy(self->name,name);
  self->radius = TILEW/2;
  self->p.x = x;
  self->p.y = y;
  self->p.z = 0;
  self->r.z = crandom() * 180;
  self->s.x = self->s.y = self->s.z = 1;
  self->activate = PickUpItem;
  if(strcmp(self->name,"gold")==0)
  {
    self->texture = LoadSprite("images/models/item_pickup.png",32,32);
  }
  else self->texture = LoadSprite("images/models/item_pickup2.png",32,32);
  self->solid = 0;
  self->health = count;
  return self;
}


/*eol@eof*/

#include "player.h"
#include "c_collide.h"
#include "projectiles.h"
#include "particle.h"
#include "weapons.h"
#include "gtrace.h"
#include "levelobj.h"
#include "npcs.h"
#include "controls.h"

extern SDL_Rect Camera;
extern Level level;
extern Uint32 NOW;

/*action frames: {first frame, last frame}*/
int idleframes[2] = {72,81};
int deadframes[2] = {48,51};
int walkframes[2] = {53,58};
int sidestepframes[2] = {59,63};
int stabframes[2] = {15,20};
int slashframes[2] = {7,14};
int shootframes[3] = {37,43,41};/*the last frame is fire*/
int castframes[3] = {32,36,35};
int prayframes[3] = {71,64,64};
int twohandframes[2] = {83,87};
int crushframes[2] = {89,95};
int blockframes[2] = {1,2};
int throwframes[2] = {44,47};
int deflectframes[2] = {1,6};
int parryframes[2] = {21,25};

Entity *PlayerEnt;
PlayerSave PDat;
Sprite *blackout = NULL;
LevelHist *ThisLevel = NULL;
int NumRaces = 0;
P_Race RaceList[10];
int NumTrains = 0;
P_Train TrainList[20];
int NumSkills = 0;
P_Skill SkillList[32];
int NumSpells = 0;
P_Skill SpellList[64];

P_Item *Equipping = NULL;

void StatusWindow();


/*


    ---==== Player Config Windows Section ====---


*/

void DrawStatusWindow(HUDInfo *self)
{
  char txt[80];
  DrawImageWindow(self->box.x,self->box.y,self->box.w,self->box.h);
  DrawTextCenteredShadow(PDat.name,self->box.x + self->box.w/2,self->box.y + 2,IndexColor(White), F_Medium, 2);
  sprintf(txt,"%s %s",PDat.race,PDat.train);
  DrawTextCenteredShadow(txt,self->box.x + self->box.w/2,self->box.y + 4 + GetFontHeight(F_Large), IndexColor(White), F_Medium, 2);
  /*player attributes*/
  DrawPlayerState(self->box.x + (Camera.w * 0.02),self->box.y + (self->box.h * 0.1));
  /*player skills*/
  DrawPlayerSpells(self->box.x + (Camera.w * 0.40), self->box.y + (self->box.h * 0.15));
  /*player spells*/
  DrawPlayerSkills(self->box.x + (Camera.w * 0.55), self->box.y + (self->box.h * 0.15));
  /*Player Equipment*/
  DrawPlayerEquipment(self->box.x + (Camera.w * 0.20),self->box.y + (self->box.h * 0.1));
}

int UpdateStatusWindow(HUDInfo *self,int pressID)
{
  switch(pressID)
  {
    case 0:
      HideMouse();
      UnPause();
      PopWindow(self->Handle);
      return 1;
  }
  return 0;
}

void StatusWindow()
{
  HUDInfo *self;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stderr,"Big problem here, can't make status window.\n");
    return;
  }
  Pause();
  self->stealinput = 1;
  self->windowupdate = UpdateStatusWindow;
  self->windowdraw = DrawStatusWindow;
  self->box.w = Camera.w*0.75;
  self->box.h = Camera.h*0.71;
  self->box.x = Camera.w/2 - self->box.w/2;
  self->box.y = Camera.h * 0.075;
  self->state = 0;
  ShowMouse();
  PicButton(self,0,SDLK_RETURN,"Done",self->box.x + self->box.w - (GetButtonW() + 4),self->box.y + self->box.h - GetButtonH() - 4,GetButtonW(),GetButtonH());
}

void EquipMain()
{
  Item *item;
  if(Equipping == NULL)return;
  item = GetItemByName(Equipping->name);
  EquipPlayerItem(Equipping->index,item->equips);
  SetupPlayerSprite(PlayerEnt);
  CalcPlayerResistance();
}

void EquipBackup()
{
  Item *item;
  if(Equipping == NULL)return;
  item = GetItemByName(Equipping->name);
  EquipPlayerItem(Equipping->index,item->equips + 1);
  SetupPlayerSprite(PlayerEnt);
  CalcPlayerResistance();
}


void DrawItemActionMenu(HUDInfo *self)
{
  P_Item *pitem;
  pitem = self->ib;
  if(pitem == NULL)return;
  DrawImageWindow(self->box.x,self->box.y,self->box.w,self->box.h);
  DrawTextCenteredShadow(pitem->name,self->box.x + self->box.w/2,self->box.y + 2,IndexColor(White),F_Medium,2);
}

int UpdateItemActionMenu(HUDInfo *self,int pressID)
{
  Item *item;
  P_Item *pitem;
  pitem = self->ib;
  item = GetItemByName(pitem->name);
  switch(pressID)
  {
    case 0:
      PopWindow(self->Handle);
      return 1;
    case 1:
      /*start parsing item action*/
      UseItem(pitem);
      PopWindow(self->Handle);
      return 1;
    case 2:
      /*if Item Equips on a hand, check for main or backup*/
      if((item->equips == 6)||(item->equips == 8))
      {
        /*new dialog to see if player wants it as the main or backkup weapon*/
        Equipping = pitem;
        TwoOptions("Where do you equip it?","Main","Alternate",EquipMain,EquipBackup);
      }
      else if(item->equips != -1)
      {
        EquipPlayerItem(pitem->index,item->equips);
        SetupPlayerSprite(PlayerEnt);
        CalcPlayerResistance();
      }
      PopWindow(self->Handle);
      return 1;
    case 3:
      NewMsg(item->description);
      return 1;
    case 4:
      /*check for a stack, ask if you want to drop all?*/
      return 1;
  }
    return 0;
}

void ItemActionMenu(P_Item *pitem)
{
  int c;
  Item *item;
  HUDInfo *self;
  if(pitem == NULL)return;
  item = GetItemByName(pitem->name);
  if(item == NULL)return;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stderr,"Big problem here, can't make NPC Dialog window.\n");
    return;
  }
  self->ib = (void *)pitem;
  self->stealinput = 1;
  self->windowupdate = UpdateItemActionMenu;
  self->windowdraw = DrawItemActionMenu;
  self->box.w = Camera.w/4;
  self->box.x = Camera.w/2 - self->box.w/2;
  self->box.y = Camera.h * 0.2;
  c = 0;
  /*if the Item can be used*/
  if(ItemHasAction(item,"use"))
  {
    PicButton(self,1,SDLK_1,"Use",self->box.x + self->box.w*.5  - (GetButtonW()/2),self->box.y + GetFontHeight(F_Medium) + 8 + ((GetButtonH() + 4)*c),GetButtonW(),GetButtonH());
    c++;
  }
  /*if the Item can be equipped*/
  if(item->equips != -1)
  {
    PicButton(self,2,SDLK_2,"Equip",self->box.x + self->box.w*.5  - (GetButtonW()/2),self->box.y + GetFontHeight(F_Medium) + 8 + ((GetButtonH() + 4)*c),GetButtonW(),GetButtonH());
    c++;
  }
  /*if the Item can be examined*/
  if(strlen(item->description) > 5)
  {
    PicButton(self,3,SDLK_3,"Examine",self->box.x + self->box.w*.5  - (GetButtonW()/2),self->box.y + GetFontHeight(F_Medium) + 8 + ((GetButtonH() + 4)*c),GetButtonW(),GetButtonH());
    c++;
  }
  /*if the Item can be dropped*/
  if(!ItemHasSpecial(item,"questitem"))
  {
    PicButton(self,4,SDLK_4,"Drop",self->box.x + self->box.w*.5  - (GetButtonW()/2),self->box.y + GetFontHeight(F_Medium) + 8 + ((GetButtonH() + 4)*c),GetButtonW(),GetButtonH());
    c++;
  }
  PicButton(self,0,SDLK_RETURN,"Cancel",self->box.x + self->box.w*.5  - (GetButtonW()/2),self->box.y + GetFontHeight(F_Medium) + 8 + ((GetButtonH() + 4)*c),GetButtonW(),GetButtonH());
  c++;
  self->box.h = GetFontHeight(F_Medium) + 8 + (c * (GetButtonH() + 4));
}

void DrawInventoryMenu(HUDInfo *self)
{
  self->subwindow = GetPlayerInventoryCount(0);
  DrawImageWindow(self->box.x,self->box.y,self->box.w,self->box.h);
  DrawTextCenteredShadow("Inventory",self->box.x + self->box.w/2,self->box.y + 2,IndexColor(White),F_Medium,2);
  DrawPlayerEquipment(self->box.x + 10,self->box.y + GetFontHeight(F_Medium));
  DrawPlayerResistance(self->box.x + self->box.w*0.33,self->box.y + GetFontHeight(F_Medium));
  DrawPlayerInventory(self->box.x + self->box.w*0.66,self->box.y + GetFontHeight(F_Medium) + 16, self->state, 20);
}

int UpdateInventoryMenu(HUDInfo *self,int pressID)
{
  P_Item *pitem;
  switch(pressID)
  {
    case 0:
      HideMouse();
      UnPause();
      PopWindow(self->Handle);
      return 1;
    case 1:
      if(self->state > 0)self->state--;
      return 1;
    case 2:
      if(self->state < self->subwindow - 20)self->state++;
      return 1;
  }
  if((pressID > 2)&&(pressID < (3 + MIN(self->subwindow,19))))
  {
    pitem = GetPlayerItemByN(pressID - 2, 0);
    ItemActionMenu(pitem);
    return 1;
  }
  return 0;
}

void InventoryMenu()
{
  int i;
  HUDInfo *self;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stderr,"Big problem here, can't make NPC Dialog window.\n");
    return;
  }
  Pause();
  self->stealinput = 1;
  self->windowupdate = UpdateInventoryMenu;
  self->windowdraw = DrawInventoryMenu;
  self->box.w = Camera.w*0.75;
  self->box.h = Camera.h*0.71;
  self->box.x = Camera.w/2 - self->box.w/2;
  self->box.y = Camera.h * 0.075;
  self->state = 0;
  self->subwindow = GetPlayerInventoryCount(0);
  ShowMouse();
  PicButton(self,0,SDLK_RETURN,"Done",self->box.x + self->box.w*.75  - (GetButtonW()/2),self->box.y + self->box.h - GetButtonH() - 4,GetButtonW(),GetButtonH());
  ArrowButton(self,1,SDLK_MINUS," ",self->box.x + self->box.w*.75  - (GetButtonW()/2),self->box.y + GetFontHeight(F_Medium),GetButtonW(),16,SDLK_UP);
  ArrowButton(self,2,SDLK_PLUS," ",self->box.x + self->box.w*.75  - (GetButtonW()/2),self->box.y + GetFontHeight(F_Medium) + ((GetFontHeight(F_Small) + 2) * 20),GetButtonW(),16,SDLK_DOWN);
  for(i = 0; i < MIN(20,self->subwindow);i++)
  {
    TextButton(self,3 + i,SDLK_a + i," ",self->box.x + self->box.w*0.66,self->box.y + GetFontHeight(F_Medium) + 16 + ((i + 1) * (GetFontHeight(F_Small) + 4)),self->box.w/2 - 4,GetFontHeight(F_Small));
  }
}

void DrawPlayerEquipment(int x,int y)
{
  char txt[80];
  int i;
  for(i = 0;i < EQUIPSLOTS;i++)
  {
    sprintf(txt,"%s:",GetEquipSlotName(i));
    DrawTxt(txt,x + 1,y + 1 + (GetFontHeight(F_Small) * (i * 2)), IndexColor(DarkGrey), F_Small);
    DrawTxt(txt,x,y + (GetFontHeight(F_Small) * (i * 2)), IndexColor(Gold), F_Small);
    if(PDat.equips[i] >= 0)
    {
      if((i == E_Arrow)||(i == E_Tool))/*-1 is nothing*/
      {
        sprintf(txt,"(%i) %ss",PDat.inventory[PDat.equips[i]].count,PDat.inventory[PDat.equips[i]].name);
      }
      else sprintf(txt,"%s",PDat.inventory[PDat.equips[i]].name);
      DrawTxt(txt,x + (GetFontHeight(F_Small) * 2) + 1,y + 1 + (GetFontHeight(F_Small) * ((i * 2) + 1)), IndexColor(DarkGrey), F_Small);
      DrawTxt(txt,x + (GetFontHeight(F_Small) * 2),y + (GetFontHeight(F_Small) * ((i * 2) + 1)), IndexColor(Silver), F_Small);
    }
  }
}

void DrawPlayerResistance(int x,int y)
{
  char txt[80];
  int i;
  DrawTxtShadow("Resistance",x,y,IndexColor(Gold),F_Small,1);
  for(i = 0;i < D_NumTypes;i++)
  {
    sprintf(txt,"%s: %i",GetResistanceType(i),PDat.resist[i]);
    DrawTxt(txt,x + 1,y + 1 + GetFontHeight(F_Small) + ((GetFontHeight(F_Small) + 2) * i), IndexColor(DarkGrey), F_Small);
    DrawTxt(txt,x,y + GetFontHeight(F_Small) + ((GetFontHeight(F_Small) + 2) * i), IndexColor(Silver), F_Small);
  }
}

void DrawPlayerSkills(int x, int y)
{
  char txt[80];
  int i;
  DrawTxt("Learned Skills",x + 1,y + 1, IndexColor(DarkGrey), F_Small);
  DrawTxt("Learned Skills",x,y, IndexColor(Gold), F_Small);
  for(i = 0;i < SKILLMAX;i++)
  {
    if(strlen(PDat.skills[i]) <= 1)break;
    sprintf(txt,"%s",PDat.skills[i]);
    DrawTxt(txt,x + 1,y + ((GetFontHeight(F_Small) + 4) * (i + 1)) + 1, IndexColor(DarkGrey), F_Small);
    DrawTxt(txt,x,y + ((GetFontHeight(F_Small) + 4) * (i + 1)), IndexColor(Silver), F_Small);
  }
}

void DrawPlayerSpells(int x, int y)
{
  char txt[80];
  int i;
  DrawTxt("Known Spells",x + 1,y + 1, IndexColor(DarkGrey), F_Small);
  DrawTxt("Known Spells",x,y, IndexColor(Gold), F_Small);
  for(i = 0;i < SPELLMAX;i++)
  {
    if(strlen(PDat.spells[i]) <= 1)break;
    sprintf(txt,"%s",PDat.spells[i]);
    DrawTxt(txt,x + 1,y + ((GetFontHeight(F_Small) + 4) * (i + 1)) + 1, IndexColor(DarkGrey), F_Small);
    DrawTxt(txt,x,y + ((GetFontHeight(F_Small) + 4) * (i + 1)), IndexColor(Silver), F_Small);
  }
}

void DrawPlayerState(int x, int y)
{
  int i;
  char txt[80];
  int color;
  if(PlayerEnt != NULL)
  {
    PDat.H = PlayerEnt->health;
    PDat.S = PlayerEnt->stamina;
    PDat.M = PlayerEnt->mana;
  }
  sprintf(txt,"Health:  %i / %i",PDat.H,PDat.HM);
  DrawTxtShadow(txt,x,y, IndexColor(White), F_Small,1);
  sprintf(txt,"Stamina:  %i / %i",PDat.S,PDat.SM);
  DrawTxtShadow(txt,x,y + ((GetFontHeight(F_Small) + 4) * 1), IndexColor(White), F_Small,1);
  sprintf(txt,"Mana:  %i / %i",PDat.M,PDat.MM);
  DrawTxtShadow(txt,x,y + ((GetFontHeight(F_Small) + 4) * 2), IndexColor(White), F_Small,1);
  for(i = 0;i < 6;i++)
  {
    sprintf(txt,"%s:  %i",GetAttributeName(i),PDat.attr[i].score);
    if(PDat.attr[i].train > 0)color = LightGreen;
    else color = LightRed;
    DrawTxtShadow(txt,x,y + ((GetFontHeight(F_Small) + 4) * (3 + i)), IndexColor(color), F_Small,1);
  }
  sprintf(txt,"Speed:  %i",PDat.speed);
  DrawTxtShadow(txt,x,y + ((GetFontHeight(F_Small) + 4) * 9), IndexColor(White), F_Small,1);
  sprintf(txt,"Carry:  %i / %i",PDat.carry,PDat.carrymax);
  DrawTxtShadow(txt,x,y + ((GetFontHeight(F_Small) + 4) * 10), IndexColor(White), F_Small,1);
  sprintf(txt,"Gold:  %i",PDat.gold);
  DrawTxtShadow(txt,x,y + ((GetFontHeight(F_Small) + 4) * 11), IndexColor(Gold), F_Small,1);
  sprintf(txt,"Grace:  %i",PDat.grace);
  DrawTxtShadow(txt,x,y + ((GetFontHeight(F_Small) + 4) * 12), IndexColor(Silver), F_Small,1);
}

void DrawPlayerInventory(int x,int y, int from, int total)
{
  P_Item *pitem = NULL;
  char buf[80];
  char txt[80];
  int i;
  DrawTxtShadow("Inventory",x,y,IndexColor(Gold),F_Small,1);
  for(i = 0;i < from;i++)
  {
    pitem = GetNextPlayerItemByPlace(pitem,0);
    if(pitem == NULL)return;
  }
  for(i = 0;i < total;i++)
  {
    memset(txt,0,sizeof(txt));
    /*gonna make this based on icons someday*/
    pitem = GetNextPlayerItemByPlace(pitem,0);
    if(pitem == NULL)return;
    if(pitem->count > 1)
    {
      sprintf(buf,"(%i) %ss",pitem->count,pitem->name);
    }
    else strcpy(buf,pitem->name);
    if(IsItemEquipped(pitem))
    {
      sprintf(txt,"E - %s",buf);
      strcpy(buf,txt);
    }
    DrawTxtShadow(buf,x,y + ((i + 1) * (GetFontHeight(F_Small) + 4)),IndexColor(Silver),F_Small,1);
  }
}


/*

    --====  Player Entity Section ===--

*/

int  SkillHasSpecial(P_Skill *spell,char tag[80])
{
  /*same data structure!*/
  return SpellHasSpecial(spell,tag);
}

int  SpellHasSpecial(P_Skill *spell,char tag[80])
{
  int i;
  if(spell == NULL)return -1;
  for(i = 0;i < 16;i++)
  {
    if(strncmp(spell->special[i],tag,strlen(tag))==0)
      return 1;
  }
  return 0;
}

char *GetSpellSpecialData(P_Skill *spell,char tag[80])
{
  char *d;
  int i;
  if(spell == NULL)return NULL;
  for(i = 0;i < 16;i++)
  {
    if(strncmp(spell->special[i],tag,strlen(tag))==0)
    {
      d = strchr(spell->special[i],' ');
      if(d == NULL)return NULL;
      d++;
      return d;
    }
  }
  return NULL;
}

int GetResistanceIndex(char type[80])
{
  if(strcmp(type,"physical")==0)
  {
    return D_Physical;
  }
  if(strcmp(type,"respierce")==0)
  {
    return D_Pierce;
  }
  if(strcmp(type,"resslash")==0)
  {
    return D_Slash;
  }
  if(strcmp(type,"rescrush")==0)
  {
    return D_Crush;
  }
  if(strcmp(type,"resmagic")==0)
  {
    return D_Magic;
  }
  if(strcmp(type,"resfire")==0)
  {
    return D_Fire;
  }
  if(strcmp(type,"resice")==0)
  {
    return D_Ice;
  }
  if(strcmp(type,"reselec")==0)
  {
    return D_Elec;
  }
  if(strcmp(type,"resshadow")==0)
  {
    return D_Shadow;
  }
  if(strcmp(type,"reslight")==0)
  {
    return D_Light;
  }
  return -1;
}

void CastSpell(Entity *self)
{
  P_Skill *spell;
  int h;
  int i;
  int dt = 0;
  int b;
  char *k;
  Coord c = {1,1,1};
  char buf[80];
  char type[80];
  char degree[80];
  if(PDat.chosenspell == -1)
  {
    return;
  }
  spell = GetSpellByName(PDat.spells[PDat.chosenspell]);
  if(spell == NULL)return;
  if(strcmp(spell->type,"color")==0)
  {
    k = GetSpellSpecialData(spell,"color");
    if(k != NULL)
    {
      sscanf(k,"%f %f %f",&c.x,&c.y,&c.z);
    }
  }
  if(strcmp(spell->type,"alchemy")==0)
  {
    i = GivePlayerItem(spell->name);
    EquipPlayerItem(i,E_Tool);
    return;
  }
  if(SpellHasSpecial(spell,"projectile"))
  {
    ShootForceBolt(self);
    return;
  }
  if(SpellHasSpecial(spell,"radius"))
  {
    k = GetSpellSpecialData(spell,"radius");
    b = GetMagicDegree(k);
    dt = 0;
    for(i = 0;i < 16;i++)
    {
      if(strlen(spell->special[i])> 1)
      {
        sscanf(spell->special[i],"%s %s %s",buf,type,degree);
        if(strcmp(type,"magic")==0)
        {
          h += GetMagicDegree(degree);
          dt |= MT_Magic;
        }
        if(strcmp(type,"holy")==0)
        {
          h += GetMagicDegree(degree);
          dt |= MT_Light;
        }
        if(strcmp(type,"shadow")==0)
        {
          h += GetMagicDegree(degree);
          dt |= MT_Shadow;
        }
        if(strcmp(type,"fire")==0)
        {
          h += GetMagicDegree(degree);
          dt |= MT_Fire;
        }
        if(strcmp(type,"ice")==0)
        {
          h += GetMagicDegree(degree);
          dt |= MT_Ice;
        }
        if(strcmp(type,"elec")==0)
        {
          h += GetMagicDegree(degree);
          dt |= MT_Elec;
        }
        if(strcmp(type,"physical")==0)
        {
          h += GetMagicDegree(degree);
          dt |= MT_Physical;
        }
        if(strcmp(type,"pierce")==0)
        {
          h += GetMagicDegree(degree);
          dt |= MT_Pierce;
        }
        if(strcmp(type,"slash")==0)
        {
          h += GetMagicDegree(degree);
          dt |= MT_Slash;
        }
        if(strcmp(type,"crush")==0)
        {
          h += GetMagicDegree(degree);
          dt |= MT_Crush;
        }
      }
    }
    
    RadiusDamage(self, self->p, b, h, 0, dt, 0, self);
    ParticleExplosion(self->p.x,self->p.y, self->p.z, b/2, (10 * b), h * 0.75, c.x, c.y, c.z, 3, 0.5);
  }
  if(SpellHasSpecial(spell,"heal"))
  {
    h = GetSpellDamage(spell);
    self->health+=h;
    if(self->health > self->healthmax)self->health = self->healthmax;
  }
  if(SpellHasSpecial(spell,"protection"))
  {
    for(i = 0;i < 16;i++)
    {
      if(strlen(spell->special[i])> 1)
      {
        sscanf(spell->special[i],"%s",buf);
        if(strcmp(buf,"protection")==0)
        {
          sscanf(spell->special[i],"%s %s %s",buf,type,degree);
          h = GetMagicDegree(degree);
          h *= 1.5;
          b = GetResistanceIndex(type);
          if(b != -1)
          {
            self->resbonus[b] = h;
            if(self->layers[7] != NULL)FreeSprite(self->layers[7]);
            self->layers[7] = LoadSprite(spell->sprite,-1,-1);
            self->layerframes[7] = 0;
          }
        }
      }
    }
  }
}

void SetActionBySpellCast(Entity *self)
{
  int i;
  Coord v = {0,0,0};
  float costfactor = 1;
  P_Skill *spell;
  if(PDat.chosenspell == -1)
  {
    NewMessage("You can't cast magic with this!",IndexColor(White));
    return;
  }
  if(PlayerHasSkill("Magic Focus"))
  {
    costfactor *= 0.8;
  }
  spell = GetSpellByName(PDat.spells[PDat.chosenspell]);
  if(spell == NULL)return;
  if(spell->cost[4] > 0)
  {
    if(GetPlayerItemCount("Herbal Base") < spell->cost[4])
    {
      NewMsg("You need more Herbal Bases to make this!");
      return;
    }
  }
  if(spell->cost[5] > 0)
  {
    if(GetPlayerItemCount("Alchemy Base") < spell->cost[5])
    {
      NewMsg("You need more Alchemy Bases to make this!");
      return;
    }
  }
  for(i = 0;i < spell->cost[4];i++)
  {
    TakePlayerItemByName("Herbal Base");
  }
  for(i = 0;i < spell->cost[5];i++)
  {
    TakePlayerItemByName("Alchemy Base");
  }
  /*check for herb, dead body, and alchemy*/
  self->mana -= (spell->cost[2] * costfactor);
  self->stamina -= spell->cost[1];/*TODO add weapon weight / player strength to this factor*/
  self->health -= spell->cost[0];
  self->attacktype = AT_Cast;
  self->attacking = 1;
  self->attackstart = NOW;
  self->attackframe = 0;
  self->attackspeed = 1;
  self->attackrange = spell->range;
  if(self->target != NULL)
  {
    v.x =  self->target->p.x - self->p.x;
    v.y =  self->target->p.y - self->p.y;
    VectorNormalize(&v);
  }
  else
  {
    AngleVector2D(&v,self->r.z);
  }
  self->targetpoint.x = self->p.x + v.x;
  self->targetpoint.y = self->p.y + v.y;
  self->guard = 0;
}

void SetActionsByItemAction(Entity *self,Item *item,int alternate)
{
  Coord v;
  int left;
  if((self == NULL)||(item == NULL))return;
  if(alternate >= 3)
  {
    left = 0;
    alternate -= 3;
  }
  else
  {
    left = 1;
    alternate--;
  }
  self->monsterdamage = item->damage;/*TODO have stats affect damage*/
  if(strcmp(item->actions[alternate],"stab")==0)
  {
    if(self->stamina > 0)
    {
      self->stamina -= 15;/*TODO add weapon weight / player strength to this factor*/
      self->attacktype = AT_Stab;
      self->attacking = 1;
      self->attackstart = NOW;
      self->attackframe = 0;
      self->attackrange = self->radius*0.75 + item->range;
      self->attackspeed = 1.6 * item->speed;
      if(self->target != NULL)
      {
        v.x =  self->target->p.x - self->p.x;
        v.y =  self->target->p.y - self->p.y;
        VectorNormalize(&v);
      }
      else
      {
        AngleVector2D(&v,self->r.z);
      }
      VectorScale(v,self->attackrange,v);    /*using 32 as the spear length, it will be based on weapon stats*/
      self->targetpoint.x = self->p.x + v.x;
      self->targetpoint.y = self->p.y + v.y;
      self->guard = 0;
    }
    else NewMessage("Too Exhausted!",IndexColor(LightGreen));
    ApplyStandardDelays(self,item->speed,left);
  }
  else if(strcmp(item->actions[alternate],"slash")==0)
  {
    if(self->stamina > 0)
    {
      self->stamina -= 20;/*TODO add weapon weight / player strength to this factor*/
      self->attacktype = AT_Slash;
      self->attacking = 1;
      self->attackstart = NOW;
      self->attackframe = 0;
      self->attackspeed = 1.3 * item->speed;
      self->attackrange = self->radius*0.75 + item->range*0.9;
      if(self->target != NULL)
      {
        v.x =  self->target->p.x - self->p.x;
        v.y =  self->target->p.y - self->p.y;
        VectorNormalize(&v);
      }
      else
      {
        AngleVector2D(&v,self->r.z);
      }
      VectorScale(v,self->attackrange,v);    /*using 32 as the spear length, it will be based on weapon stats*/
      self->targetpoint.x = self->p.x + v.x;
      self->targetpoint.y = self->p.y + v.y;
      self->guard = 0;
    }
    else NewMessage("Too Exhausted!",IndexColor(LightGreen));
    ApplyStandardDelays(self,item->speed,left);
    
  }
  else if(strcmp(item->actions[alternate],"crush")==0)
  {
    if(self->stamina > 0)
    {
      self->stamina -= 25;/*TODO add weapon weight / player strength to this factor*/
      self->attacktype = AT_Crush;
      self->attacking = 1;
      self->attackstart = NOW;
      self->attackframe = 0;
      self->attackspeed = 1.0 * item->speed;
      self->attackrange = self->radius*0.75 + item->range;
      if(self->target != NULL)
      {
        v.x =  self->target->p.x - self->p.x;
        v.y =  self->target->p.y - self->p.y;
        VectorNormalize(&v);
      }
      else
      {
        AngleVector2D(&v,self->r.z);
      }
      VectorScale(v,self->attackrange,v);
      self->targetpoint.x = self->p.x + v.x;
      self->targetpoint.y = self->p.y + v.y;
      self->guard = 0;
    }
    else NewMessage("Too Exhausted!",IndexColor(LightGreen));
    ApplyStandardDelays(self,item->speed,left);
  }
  else if(strcmp(item->actions[alternate],"shoot")==0)
  {
    if(self->stamina > 0)
    {
      self->stamina -= 30;/*TODO add weapon weight / player strength to this factor*/
      self->attacktype = AT_Shoot;
      self->attacking = 1;
      self->attackstart = NOW;
      self->attackframe = 0;
      self->attackspeed = 1 * item->speed;
      self->attackrange = item->range;
      if(self->target != NULL)
      {
        v.x =  self->target->p.x - self->p.x;
        v.y =  self->target->p.y - self->p.y;
        VectorNormalize(&v);
      }
      else
      {
        AngleVector2D(&v,self->r.z);
      }
      self->targetpoint.x = self->p.x + v.x;
      self->targetpoint.y = self->p.y + v.y;
      self->guard = 0;
    }
    else NewMessage("Too Exhausted!",IndexColor(LightGreen));
    ApplyStandardDelays(self,item->speed,left);
  }
  else if(strcmp(item->actions[alternate],"Athrow")==0)
  {
    if(self->stamina > 0)
    {
      self->stamina -= 20;/*TODO add weapon weight / player strength to this factor*/
      self->attacktype = AT_Potion;
      self->attacking = 1;
      self->attackstart = NOW;
      self->attackframe = 0;
      self->attackspeed = 1 * item->speed;
      self->attackrange = item->range;
      if(self->target != NULL)
      {
        v.x =  self->target->p.x - self->p.x;
        v.y =  self->target->p.y - self->p.y;
        VectorNormalize(&v);
      }
      else
      {
        AngleVector2D(&v,self->r.z);
      }
      self->targetpoint.x = self->p.x + v.x;
      self->targetpoint.y = self->p.y + v.y;
      self->guard = 0;
    }
    else NewMessage("Too Exhausted!",IndexColor(LightGreen));
    ApplyStandardDelays(self,item->speed,left);
  }
  else if(strcmp(item->actions[alternate],"throw")==0)
  {
    if(self->stamina > 0)
    {
      self->stamina -= 20;/*TODO add weapon weight / player strength to this factor*/
      self->attacktype = AT_Throw;
      self->attacking = 1;
      self->attackstart = NOW;
      self->attackframe = 0;
      self->attackspeed = 1 * item->speed;
      self->attackrange = item->range;
      if(self->target != NULL)
      {
        v.x =  self->target->p.x - self->p.x;
        v.y =  self->target->p.y - self->p.y;
        VectorNormalize(&v);
      }
      else
      {
        AngleVector2D(&v,self->r.z);
      }
      self->targetpoint.x = self->p.x + v.x;
      self->targetpoint.y = self->p.y + v.y;
      self->guard = 0;
    }
    else NewMessage("Too Exhausted!",IndexColor(LightGreen));
    ApplyStandardDelays(self,item->speed,left);
  }
  else if(strcmp(item->actions[alternate],"use")==0)
  {
    if(self->stamina > 0)
    {
      self->stamina -= 5;/*TODO add weapon weight / player strength to this factor*/
      self->guard = 0;
      UseItem(GetPlayerItemByName(item->name));
    }
    else NewMessage("Too Exhausted!",IndexColor(LightGreen));
    ApplyStandardDelays(self,item->speed,left);
  }
  else if(strcmp(item->actions[alternate],"cast")==0)
  {
    if(self->mana > 0)
    {
      SetActionBySpellCast(self);
    }
    else NewMessage("Not Enough Mana!",IndexColor(LightBlue));
    ApplyStandardDelays(self,item->speed,left);
  }
  else if(strcmp(item->actions[alternate],"block")==0)
  {
    if(left)  self->Lcd = 1;
    else self->Rcd = 1;
    if(!self->guard)
    {
      self->guardstart = NOW;
    }
    self->guard = 2;
  }
}

void UpdatePlayerCamera()
{
  if(PlayerEnt == NULL)return;
  Camera.x = PlayerEnt->p.x - Camera.w/2;;
  Camera.y = PlayerEnt->p.y - Camera.h/2;;
}

void Dash(Entity *self)
{
  if(self->dashing)return;
  /*already dashing this frame!*/
  if(self->stamina > 20)
  {
    self->stamina -= 20;
    self->dashing = 1;
    AddDelays(self,10);
  }
}

void PlayerThink(Entity *self)
{
  SDLMod mod;
  Item *item = NULL;
  P_Item *pitem = NULL;
  Coord v = {0,0,0};
  Coord h = {0,0,0};
/*  Coord v2 = {0,0,0};*/
  int go = 0;
  /*float temp;*/
  Trace trace;
  Uint8 *keys;
  self->dashing = 0;
  self->running = 0;
  keys = SDL_GetKeyState(NULL);
  mod = SDL_GetModState();
  if(IsInputReleased("SaveGame"))
  {
    YesNo("Save Game?",SavePlayerGame,NULL);
    return;
  }
  if(IsPausing())return;
  if(self->target != NULL)
  {
    if((self->target->state == MS_Dead)||(self->target->state == MS_Dead))
      self->target = NULL;
  }
  if(self->state == MS_Stunned)
  {
    self->stun--;
    if(self->stun <= 0)
    {
      self->state = MS_Idle;
    }
  }
  /*dashing state is active until the player can move again*/
  self->dashing = 0;
  if(self->Acd <= 0)
  {
    /*this section is for actions*/
    go = 0;
    if(IsInputReleased("InventoryWindow"))
    {
      InventoryMenu();
      return;
    }
    else if(IsInputReleased("ControlsWindow"))
    {
      ControlsWindow();
      NewMsg("Got Here");
      return;
    }
    else if(IsInputReleased("StatusWindow"))
    {
      StatusWindow();
      return;
    }
    if(self->Lcd <= 0)    /*Left (or secondary) arm actions*/
    {
      if(IsInputDown("LAction1"))
      {
        go = 1;
        pitem = &PDat.inventory[PDat.equips[E_Secondary1]];
      }
      else if(IsInputDown("LAction2"))
      {
        go = 2;
        pitem = &PDat.inventory[PDat.equips[E_Secondary1]];
      }
      else if(IsInputDown("LSwap"))
      {
        ToggleWeapon(E_Secondary1,E_Primary1);
        SetupPlayerSprite(self);
        CalcPlayerResistance();
        self->Lcd = 4;
      }
      else if(IsInputReleased("NextSpell"))
      {
        SelectNextSpell();
        self->Lcd = 4;
      }
      else if(IsInputDown("NextTool"))
      {
        SelectNextTool();
        self->Lcd = 4;
      }
    }
    if(self->Rcd <= 0)
    {
      if(IsInputDown("RAction1"))
      {
        go = 3;
        pitem = &PDat.inventory[PDat.equips[E_Primary1]];
      }
      else if(IsInputDown("RAction2"))
      {
        go = 4;
        pitem = &PDat.inventory[PDat.equips[E_Primary1]];
      }
      else if(IsInputDown("RSwap"))
      {
        ToggleWeapon(E_Primary1,E_Secondary1);
        SetupPlayerSprite(self);
        self->Rcd = 4;
      }
      else if(IsInputReleased("Target"))
      {
        /*target nearest Foe*/
        if(self->target != NULL)
        {
          self->target = NULL;
        }
        else
        {
          self->target = GetClosestFoe(self,NULL);
        }
        self->Acd = 10;
      }
      else if(IsInputReleased("Interact"))
      {
        v.y = -1;
        RotateVectorZ(&v, -self->r.z);
        VectorScale(v,self->radius,v);
        h.x = self->p.x + v.x;
        h.y = self->p.y + v.y;
        VectorScale(v,1.5,v);
        memset(&trace,0,sizeof(Trace));
        if(GTrace(self->p,v, 1, F_NODEAD, &trace,self))
        {
          switch(trace.hittype)
          {
            case 0:
              NewMessage("Touched a Wall",IndexColor(LightYellow));
              break;
            case 1:
              if(trace.other != NULL)
              {
                if(trace.other->activate != NULL)
                {
                  trace.other->activate(trace.other,self);
                }
              }
              self->Acd = 10;
              break;
            case 2:
              NewMessage("Touched a Bounding Edge",IndexColor(LightYellow));
              break;
          }
        }
      }
      else if(IsInputPressed("UseTool"))
      {
        pitem = &PDat.inventory[PDat.equips[E_Tool]];
        if(pitem != NULL)
        {
          go = 1;
        }
      }
    }
    if((go)&&(pitem != NULL))
    {
      item = GetItemByName(pitem->name);
      /*TODO: Adjust stats based on weapon upgrades*/
      if(item != NULL)
      {
        /*any other stats that need to be set are set here*/
        SetActionsByItemAction(self,item,go);
      }
    }
  }
  if(self->Gcd <= 0)
  {
    self->mv.x = self->mv.y = 0;
    v.x = v.y = 0;
    if(IsInputDown("Run"))
    {
      if(self->stamina > 10)
      {
        self->stamina -= 2;
        self->running = 1;
      }
    }
    /*this section is for things that will only stop if the player is stunned or doing a major action*/
    if(IsInputDown("MoveLeft"))
    {
      if(IsInputDoubleTapped("MoveLeft"))
      {
        Dash(self);
      }
      /*
      if(self->target != NULL)
      {
        VectorToTarget(self,&v2);
        temp = -v2.x;
        v2.x = v2.y;
        v2.y = temp;
        VectorAdd(self->mv,v2,self->mv);
        if(self->layerframes[0] < sidestepframes[0])self->layerframes[0] = sidestepframes[0];
        else if(self->layerframes[0] >= sidestepframes[1])self->layerframes[0] = sidestepframes[0];
        else self->layerframes[0]+=0.5;
      }
      else*/ self->mv.x = -1;
    }
    if(IsInputDown("MoveRight"))
    {
      if(IsInputDoubleTapped("MoveRight"))
      {
        Dash(self);
      }
      /*
      if(self->target != NULL)
      {
        VectorToTarget(self,&v2);
        temp = v2.x;
        v2.x = -v2.y;
        v2.y = temp;
        VectorAdd(self->mv,v2,self->mv);
        if(self->layerframes[0] <= sidestepframes[0])self->layerframes[0] = sidestepframes[1];
        else if(self->layerframes[0] > sidestepframes[1])self->layerframes[0] = sidestepframes[1];
        else self->layerframes[0]-=0.5;
      }
      else*/ self->mv.x = 1;
    }
    if(IsInputDown("MoveDown"))
    {
      if(IsInputDoubleTapped("MoveDown"))
      {
        Dash(self);
      }
      /*
      if(self->target != NULL)
      {
        VectorToTarget(self,&v2);
        VectorNegate(v2,v2);
        VectorAdd(self->mv,v2,self->mv);
        if(self->layerframes[0] <= walkframes[0])self->layerframes[0] = walkframes[1];
        else if(self->layerframes[0] > walkframes[1])self->layerframes[0] = walkframes[1];
        else self->layerframes[0]-=0.5;
      }
      else */self->mv.y = 1;
    }
    if(IsInputDown("MoveUp"))
    {
      if(IsInputDoubleTapped("MoveUp"))
      {
        Dash(self);
      }
      /*
      if(self->target != NULL)
      {
        if(RelativeSize(self->p.x - self->target->p.x,self->p.y - self->target->p.y) > ((self->radius + self->target->radius) * (self->radius + self->target->radius)))
        {
          VectorToTarget(self,&v2);
          VectorAdd(self->mv,v2,self->mv);
        }
        if(self->layerframes[0] < walkframes[0])self->layerframes[0] = walkframes[0];
        else if(self->layerframes[0] >= walkframes[1])self->layerframes[0] = walkframes[0];
        else self->layerframes[0]+=0.5;
      }
      else*/ self->mv.y = -1;
    }
    VectorNormalize(&self->mv);
    if(self->target == NULL)
    {
      if((self->mv.x)||(self->mv.y))
      {
        self->r.z = DegreeFromVect(-self->mv.x, self->mv.y);
        if(self->layerframes[0] < walkframes[0])self->layerframes[0] = walkframes[0];
        else if(self->layerframes[0] >= walkframes[1])self->layerframes[0] = walkframes[0];
        else self->layerframes[0]+= 0.45;
      }
    }
    else
    {
      EntLookAtOther(self,self->target);
    }
  }
  Walk(self);
}

void UpdatePlayer(Entity *self)
{
  int   ox,oy;
  float restfactor = 1;
  CoolDowns(self);
  ox = self->p.x/TILEW;
  oy = self->p.y/TILEH;
  /* Stat Regen Section */
  if(!self->attacking)
  {
    if(self->guard)restfactor *= 0.8;
    if(RelativeSize(self->mv.x, self->mv.y) > 0)restfactor *= 0.8;
    if((self->Gcd)||(self->Acd))restfactor *= 0.8;
    /*TODO if stunne or status effected this will adjust*/
    if(self->stamina < self->staminamax)
    {
      self->stamina += (1 * restfactor);
    }
    if(restfactor == 1)
    {
      if(PlayerHasSkill("Magic Focus"))
      {
        restfactor *= 1.5;
      }
    }
    if(self->mana < self->manamax)
    {
      self->mana += (0.01 * restfactor);
    }
  }
  /*movement*/
  if(RelativeSize(self->v.x, self->v.y) > 0)
  {
    UpdateEntityPosition(self,NULL);
    if((ox != (int)self->p.x/TILEW)||(oy != (int)self->p.y/TILEH))
    {
      UpdateSeenMask(5);
    }
  }
  /*animation maintenance*/
  if(self->attacking)
  {
    /*this is going to become a lot more robust*/
    self->attackframe += self->attackspeed;
    if(self->attacktype == AT_Stab)
    {
      self->layerframes[1] = self->layerframes[2] = self->layerframes[3] = self->layerframes[4] = self->layerframes[5] = stabframes[0] + ((stabframes[1] - stabframes[0]) * self->attackframe/10.0);
      if(self->attackframe >= 4)
      {
        if((self->attackframe > 10)||(Stab(self,self->p,self->targetpoint,self->attackrange,2, self->attackframe/10.0, self->monsterdamage, 1, D_Physical|D_Pierce)))
        {
          self->attacking = 0;
        }
      }
    }
    if(self->attacktype == AT_Slash)
    {
      self->layerframes[1] = self->layerframes[2] = self->layerframes[3] = self->layerframes[4] = self->layerframes[5] = slashframes[0] + ((slashframes[1] - slashframes[0]) * self->attackframe/8.0);
      if(self->attackframe >= 4)
      {
        if((self->attackframe > 8)||(Slash(self,self->p,self->targetpoint,self->attackrange,35,-35,2, self->attackframe/8.0, self->monsterdamage, 1, D_Physical|D_Slash)))
        {
          self->attacking = 0;
        }
      }
    }
    if(self->attacktype == AT_Crush)
    {
      self->layerframes[1] = self->layerframes[2] = self->layerframes[3] = self->layerframes[4] = self->layerframes[5] = crushframes[0] + ((crushframes[1] - crushframes[0]) * self->attackframe/8.0);
      if(self->attackframe >= 4)
      {
        if((self->attackframe > 8)||(Crush(self,self->p,self->targetpoint,self->attackrange,35,-35,2, self->attackframe/8.0, self->monsterdamage, 1, D_Physical|D_Crush)))
        {
          self->attacking = 0;
        }
      }
    }
    if(self->attacktype == AT_Shoot)
    {
      self->layerframes[1] = self->layerframes[2] = self->layerframes[3] = self->layerframes[4] = self->layerframes[5] = shootframes[0] + ((shootframes[1] - shootframes[0]) * self->attackframe/8.0);
      if((int)self->attackframe == 2)
      {
        if(PDat.equips[E_Arrow] == -1)
        {
          self->attacking = 0;
          NewMessage("Out of Arrows!",IndexColor(White));
        }
      }
      if((int)self->attackframe == 6)
      {
        ShootArrow(self);
        SpendArrow();
        TrainAttribute(A_Str, 1);
        TrainAttribute(A_Dex, 1);
        TrainAttribute(A_Foc, 1);
      }
      if(self->attackframe > 8)
      {
        self->attacking = 0;
      }
    }
    if(self->attacktype == AT_Throw)
    {
      self->layers[1] = NULL;
      self->layerframes[1] = self->layerframes[2] = self->layerframes[3] = self->layerframes[4] = self->layerframes[5] = throwframes[0] + ((throwframes[1] - throwframes[0]) * self->attackframe/4.0);
      if((int)self->attackframe == 3)
      {
        if(PDat.equips[E_Tool] == -1)
        {
          self->attacking = 0;
          SetupPlayerSprite(self);
          NewMsg("All Out");
        }
      }
      if((int)self->attackframe == 6)
      {
        
        ThrowKnife(self);
        SpendTool();
        TrainAttribute(A_Dex, 1);
        TrainAttribute(A_Foc, 1);
      }
      if(self->attackframe > 8)
      {
        self->attacking = 0;
        SetupPlayerSprite(self);
      }
    }
    if(self->attacktype == AT_Potion)
    {
      self->layers[1] = NULL;
      self->layerframes[1] = self->layerframes[2] = self->layerframes[3] = self->layerframes[4] = self->layerframes[5] = throwframes[0] + ((throwframes[1] - throwframes[0]) * self->attackframe/4.0);
      if((int)self->attackframe == 3)
      {
        if(PDat.equips[E_Tool] == -1)
        {
          self->attacking = 0;
          SetupPlayerSprite(self);
          NewMsg("All Out");
        }
      }
      if((int)self->attackframe == 6)
      {
        ThrowPotion(self,PDat.inventory[PDat.equips[E_Tool]].name);
        SpendTool();
        TrainAttribute(A_Dex, 1);
        TrainAttribute(A_Foc, 1);
      }
      if(self->attackframe > 8)
      {
        self->attacking = 0;
        SetupPlayerSprite(self);
      }
    }
    if(self->attacktype == AT_Cast)
    {
      self->layerframes[1] = self->layerframes[2] = self->layerframes[3] = self->layerframes[4] = self->layerframes[5] = castframes[0] + ((castframes[1] - castframes[0]) * self->attackframe/8.0);
      if((int)self->attackframe == 6)
      {
        CastSpell(self);
        TrainAttribute(A_Foc, 1);
        TrainAttribute(A_Spi, 1);
      }
      if(self->attackframe > 8)
      {
        self->attacking = 0;
      }
    }
    self->frame = 2;
  }
  else if(self->guard)
  {
    /*using the shield, Set left arm sprite to guard*/
    self->guard--;
    self->frame = 1;
    self->layerframes[1] = self->layerframes[2] = self->layerframes[3] = self->layerframes[4] = self->layerframes[5] =  blockframes[0];
  }
  else
  {
    /*when idle*/
    if(self->layerframes[1] < idleframes[0])self->layerframes[1] = self->layerframes[2] = self->layerframes[3] = self->layerframes[4] = self->layerframes[5] = idleframes[0];
    else if (self->layerframes[1] > idleframes[1])self->layerframes[1] = self->layerframes[2] = self->layerframes[3] = self->layerframes[4] = self->layerframes[5] = idleframes[0];
    else
    {
      self->layerframes[1] = self->layerframes[2] = self->layerframes[3] = self->layerframes[4] = self->layerframes[5] = self->layerframes[1] + 0.2;
    }
  }
}

void SetupPlayerSprite(Entity *self)
{
  Item *item;
  int i;
  if(self == NULL)return;
  for(i=0;i < 7;i++)
  {
    /*clean up old references to the sprites*/
    if(self->layers[i] != NULL)FreeSprite(self->layers[i]);
    self->layers[i] = NULL;
  }
  /*first the pants*/
  if(PDat.equips[E_Legs] != -1)
  {
    item = GetItemByName(PDat.inventory[PDat.equips[E_Legs]].name);
    if(item != NULL)
    {
      self->layers[0] = LoadSprite(item->sprite,128,128);
    }
  }else self->layers[0] = LoadSprite("images/models/leather_boots.png",128,128);
  /*then the primary weapon*/
  if(PDat.equips[E_Primary1] != -1)
  {
    item = GetItemByName(PDat.inventory[PDat.equips[E_Primary1]].name);
    if(item != NULL)
    {
      self->layers[1] = LoadSprite(item->sprite,128,128);
    }
  }
  /*then the body*/
  if(PDat.equips[E_Body] != -1)
  {
    item = GetItemByName(PDat.inventory[PDat.equips[E_Body]].name);
    if(item != NULL)
    {
      self->layers[2] = LoadSprite(item->sprite,128,128);
    }
  }else self->layers[2] = LoadSprite("images/models/leather_armor.png",128,128);
  /*then the shield / secondary weapon*/
  if(PDat.equips[E_Secondary1] != -1)
  {
    item = GetItemByName(PDat.inventory[PDat.equips[E_Secondary1]].name);
    if(item != NULL)
    {
      self->layers[3] = LoadSprite(item->sprite,128,128);
    }
  }
  /*then the cape / cloak*/
  if(PDat.equips[E_Neck] != -1)
  {
    item = GetItemByName(PDat.inventory[PDat.equips[E_Neck]].name);
    if(item != NULL)
    {
      self->layers[4] = LoadSprite(item->sprite,128,128);
    }
  }
  /*the the hood*/
  if(PDat.equips[E_Head] != -1)
  {
    item = GetItemByName(PDat.inventory[PDat.equips[E_Head]].name);
    if(item != NULL)
    {
      self->layers[5] = LoadSprite(item->sprite,128,128);
    }
  }else self->layers[5] = LoadSprite("images/models/leather_hood.png",128,128);
}

void SpawnPlayer(Coord p)
{
  Entity *self = NULL;
  self = NewEntity();
  if(self == NULL)return;
  sprintf(self->name,"player");
  self->solid = 1;
  self->think = PlayerThink;
  self->ThinkRate = 30;
  self->update = UpdatePlayer;
  self->UpdateRate = 30;
  self->radius = 10;
  self->takesdamage = 1;
  self->p.x = p.x;
  self->p.y = p.y;
  self->p.z = 0;
  self->r.x = 0;
  self->s.x = self->s.y = self->s.z = 1;
  SetupPlayerSprite(self);
  /*set layers based on equipment!*/
  self->useslayers = 1;
  self->box.w = 1;
  self->box.d = 1;
  self->box.h = 1;
  self->box.x = self->box.w/2;
  self->box.y = self->box.h/2 + 1;
  self->box.z = self->box.d/2;
  self->shaderProg = 0;
  self->health = PDat.H;
  self->healthmax = PDat.HM;
  self->stamina = PDat.S;
  self->staminamax = PDat.SM;
  self->mana = PDat.M;
  self->manamax = PDat.MM;
  self->frame = 0;
  self->team = 1;
  self->sightdist = 9 * 32;
  self->sightrange = 90;
  self->accel = 0.25;         /*to be adjusted*/
  self->speed = 6;            /*to be calculated*/
  PlayerEnt = self;
}

int GetLevelProgressCount(LevelHist *lh)
{
  Progress *p;
  int count = 0;
  if(lh == NULL)return 0;
  p = lh->progress;
  while(p != NULL)
  {
    count++;
    p = p->next;
  }
  return count;
}

Progress *GetProgressByNameIndexMap(char map[80],char name[80],int index)
{
  Progress *p;
  LevelHist *lh;
  lh = GetLevelHistoryByName(map);
  if(lh == NULL)return NULL;
  p = lh->progress;
  while(p != NULL)
  {
    if(strcmp(p->map,map)==0)
    {
      if(strcmp(p->name,name)==0)
      {
        if(p->index == index)
        {
          return p;
        }
      }
    }
    p = p->next;
  }
  return NULL;
}

Progress *GetProgressByNameMap(char map[80],char name[80])
{
  Progress *p;
  LevelHist *lh;
  lh = GetLevelHistoryByName(map);
  if(lh == NULL)return NULL;
  p = lh->progress;
  while(p != NULL)
  {
    if(strcmp(p->map,map)==0)
    {
      if(strcmp(p->name,name)==0)
      {
        return p;
      }
    }
    p = p->next;
  }
  return NULL;
}

Progress *GetProgressByQuestName(char quest[80])
{
  Progress *p;
  LevelHist *lh;
  lh = PDat.levelhistory;
  while(lh != NULL)
  {
    p = lh->progress;
    while(p != NULL)
    {
      if(strcmp(p->name,quest)==0)return p;
      p = p->next;
    }
    lh = lh->next;
  }
  return NULL;
}


Progress *GetProgressByIndexMap(char map[80],int index)
{
  Progress *p;
  LevelHist *lh;
  lh = GetLevelHistoryByName(map);
  if(lh == NULL)return NULL;
  p = lh->progress;
  while(p != NULL)
  {
    if(strcmp(p->map,map)==0)
    {
      if(p->index == index)
      {
        return p;
      }
    }
    p = p->next;
  }
  return NULL;
}

Progress *NewProgress(char map[80])
{
  Progress *p;
  LevelHist *lh;
  lh = GetLevelHistoryByName(map);
  if(lh == NULL)return NULL;
  if(lh->progress == NULL)
  {
    /*first progress for this level*/
    lh->progress = (Progress *)malloc(sizeof(Progress));
    if(lh->progress == NULL)
    {
      fprintf(stderr,"Unable to allocate player progress.  FATAL!\n");
      return NULL;
    }
    memset(lh->progress,0,sizeof(Progress));
    strcpy(lh->progress->map,map);
    return lh->progress;
  }
  p = lh->progress;
  while(p->next != NULL)
  {
    p = p->next;
  }
  p->next = (Progress *)malloc(sizeof(Progress));
  if(p->next == NULL)
  {
    fprintf(stderr,"Unable to allocate player progress.  FATAL!\n");
    return NULL;
  }
  memset(p->next,0,sizeof(Progress));
  strcpy(p->next->map,map);
  return p->next;
}

void SetPlayerProgress(char map[80],char name[80], int index,char key[80])
{
  Progress *p;
  p = GetProgressByNameIndexMap(map,name,index);
  if(p == NULL)
  {
    p = NewProgress(map);
  }
  if(p == NULL)return;
  strcpy(p->name,name);
  p->index = index;
  strcpy(p->value,key);
}

int CanSeePoint(Coord s, Coord g, int targetrad, Entity *ignore)
{
  Coord v;
  Trace trace;
  int good;
  good = 0;
  v.x = g.x - s.x;
  v.y = g.y - s.y;
  v.z = 0;
  if(LevelTrace(s,v,0.5,0, -1, &trace))
  {
    if(PointInRect(trace.POC,g.x - targetrad, g.y - targetrad,2 * targetrad, 2 * targetrad ))
    {
      good = !EntTrace(s,v, 0.2, F_SOLID|F_TDAMAGE|F_SIGHT, &trace, ignore);
      if(!good)
      {
        if(PointInRect(trace.POC,g.x - targetrad, g.y - targetrad,2 * targetrad, 2 * targetrad))
        {
          good = 1;
        }
      }
    }
  }
  else
  {
    good = !EntTrace(PlayerEnt->p,v, 1, F_SOLID|F_TDAMAGE|F_SIGHT, &trace, ignore);
    if(!good)
    {
      if(PointInRect(trace.POC,g.x - targetrad, g.y - targetrad,2 * targetrad, 2 * targetrad))
      {
        good = 1;
      }
    }
  }
  return good;
}

void UpdateSeenMask(int sightrange)
{
  int i;
  int good;
  float x,y;
  float cx,cy;
  Trace trace;
  sightrange *= sightrange;
  for(i = 0;i < ThisLevel->lsize;i++)
  {
    if(ThisLevel->ldata[i] == 0)
    {
      cx = x = ((i % ThisLevel->w) * TILEW) + TILEW/2;
      cy = y = ((i / ThisLevel->w) * TILEH) + TILEH/2;
      if(PlayerEnt->p.x < (x - TILEW/2))x -= TILEW/2;
      else if(PlayerEnt->p.x > (x + TILEW/2))x += TILEW/2;
      if(PlayerEnt->p.y < (y - TILEH/2))y -= TILEH/2;
      else if(PlayerEnt->p.y > (y + TILEH/2))y += TILEH/2;
      if(RelativeSize( x - PlayerEnt->p.x, y - PlayerEnt->p.y) < (32*32*sightrange))
      {
        good = 0;
        if(LevelTrace(PlayerEnt->p,M_Coord(x - PlayerEnt->p.x,y - PlayerEnt->p.y,0),0.5,0, -1, &trace))
        {
          if(PointInRect(trace.POC,cx - TILEW/2 - 2, cy - TILEH/2 - 2,TILEW + 4, TILEH + 4 ))
          {
            good = !EntTrace(PlayerEnt->p,M_Coord(x - PlayerEnt->p.x,y - PlayerEnt->p.y,0), 0.2, F_SIGHT, &trace, PlayerEnt);
            if(!good)
            {
              if(PointInRect(trace.POC,cx - TILEW/2 - 2, cy - TILEH/2 - 2,TILEW + 4, TILEH + 4))
              {
                good = 1;
              }
            }
          }
        }
        else
        {
          good = !EntTrace(PlayerEnt->p,M_Coord(x - PlayerEnt->p.x,y - PlayerEnt->p.y,0), 1, F_SIGHT, &trace, PlayerEnt);
          if(!good)
          {
            if(PointInRect(trace.POC,cx - TILEW/2 - 2, cy - TILEH/2 - 2,TILEW + 4, TILEH + 4))
            {
              good = 1;
            }
          }
        }
        if(good)
        {
          ThisLevel->ldata[i] = 1;
        }
      }
    }
  }
}


/*

    --===  Player Stats Section ===--

*/

P_Race *GetRaceByName(char name[80])
{
  int i;
  for(i = 0;i < NumRaces;i++)
  {
    if(strcmp(name,RaceList[i].name)==0)
      return &RaceList[i];
  }
  return NULL;
}

P_Race *GetRaceByIndex(int id)
{
  if((id < 0)||(id >= NumRaces))return NULL;
  return &RaceList[id];
}

int GetRaceCount()
{
  return NumRaces;
}

P_Train *GetTrainByName(char name[80])
{
  int i;
  for(i = 0;i < NumTrains;i++)
  {
    if(strcmp(name,TrainList[i].name)==0)
      return &TrainList[i];
  }
  return NULL;
}

P_Train *GetTrainByIndex(int id)
{
  if((id < 0)||(id >= NumTrains))return NULL;
  return &TrainList[id];
}

int GetTrainingCount()
{
  return NumTrains - 1;
}

void DeleteLevelHistory(LevelHist *l)
{
  if(l != NULL)
  {
    DeleteLevelHistory(l->next);
    if(l->ldata != NULL)free(l->ldata);
    free(l);
    return;
  }
}

void ClearLevelHistory()
{
  if(PDat.levelhistory != NULL)
  {
    DeleteLevelHistory(PDat.levelhistory);
    PDat.levelhistory = NULL;
  }
}

void SelectLevelHistoryByName(char name[80])
{
  LevelHist *lh;
  lh = GetLevelHistoryByName(name);
  if(lh == NULL)
  {
    lh = NewLevelHistory(name, level.numtiles, level.w, level.h);
  }
  if(lh == NULL)
  {
    fprintf(stderr,"setting up level history for level %s failed\n",name);
    return;
  }
  ThisLevel = lh;
  strcpy(PDat.map,name);/*so we can restore the player to this map*/
}

int GetLevelHistoryCount()
{
  int count = 0;
  LevelHist *c;
  if(PDat.levelhistory == NULL)
  {
    return 0;
  }
  else c = PDat.levelhistory;
  while(c != NULL)
  {
    count++;
    c = c->next;
  }
  return count;
}

LevelHist *GetLevelHistoryByName(char name[80])
{
  LevelHist *c;
  if(PDat.levelhistory == NULL)
  {
    return NULL;
  }
  else c = PDat.levelhistory;
  while(c != NULL)
  {
    if(strcmp(name,c->name) == 0)
    {
      return c;
    }
    c = c->next;
  }
  return NULL;
}

LevelHist *NewLevelHistory(char name[80], int size, int w, int h)
{
  LevelHist *c;
  if(PDat.levelhistory == NULL)
  {
    PDat.levelhistory = (LevelHist *)malloc(sizeof(LevelHist));
    c = PDat.levelhistory;
    PDat.numvisited = 0;
  }
  else
  {
    c = PDat.levelhistory;
    while(c->next != NULL)
    {
      c = c->next;
    }
    c->next = (LevelHist *)malloc(sizeof(LevelHist));
    c = c->next;
    if(c == NULL)
    {
      fprintf(stderr,"unable to allocate a new history!\n");
    }
  }
  if(c == NULL)
  {
    fprintf(stderr,"unable to allocate a new player level history!\n");
    return NULL;
  }
  memset(c,0,sizeof(LevelHist));
  c->ldata = (int *)malloc(sizeof(int)*size);
  if(c->ldata == NULL)
  {
    fprintf(stderr,"unable to allocate a new player level history!\n");
    return NULL;
  }
  memset(c->ldata,0,sizeof(int)*size);
  c->lsize = size;
  c->w = w;
  c->h = h;
  strcpy(c->name,name);
  PDat.numvisited++;
  return c;
}


void DrawLevelUnknown()
{
  int i;
  int x,y;
  if(ThisLevel == NULL)return;
  if(blackout == NULL)
  {
    return;
  }
  for(i = 0;i < ThisLevel->lsize;i++)
  {
    if(ThisLevel->ldata[i] == 0)
    {
      x = ((i % ThisLevel->w)* TILEW) - Camera.x + TILEW/2;
      y = ((i / ThisLevel->w)* TILEH) - Camera.y + TILEH/2;
      if((x  + TILEW < 0)||(y  + TILEH < 0)||(x  - TILEW > Camera.w)||(y  - TILEH > Camera.h))continue;
      DrawSpriteStretchRot( blackout, x,y , 0, 1, 1, 0, 1 );
    }
  }
}

P_Item *GetNextPlayerItemByPlace(P_Item *last,int stock)
{
  if(last == NULL)
    last = &PDat.inventory[0];
  else last++;
  for(;last < &PDat.inventory[STOCKMAX];last++)
  {
    if((last->inuse) && (last->instock == stock) && (last->count > 0))
    {
      return last;
    }
  }
  return NULL;
}

int GetEquippedItemSlot(P_Item *pitem)
{
  int i;
  if(pitem == NULL)return -1;
  for(i = 0;i < EQUIPSLOTS;i++)
  {
    if(PDat.equips[i] == pitem->index)return i;
  }
  return -1;
}

int  IsItemEquipped(P_Item *pitem)
{
  int i;
  if(pitem == NULL)return -1;
  for(i = 0;i < EQUIPSLOTS;i++)
  {
    if(PDat.equips[i] == pitem->index)return 1;
  }
  return 0;
}

int GetPlayerInventoryCount(int stockpile)
{
  int i;
  int count = 0;
  for(i = 0;i < STOCKMAX;i++)
  {
    if((PDat.inventory[i].inuse)&&(PDat.inventory[i].instock == stockpile)&&(PDat.inventory[i].count > 0))
    {
      count++;
    }
  }
  return count;
}

P_Item *GetPlayerItemByIndex(int i)
{
  if((i < 0)||(i >= PDat.inventorycount))return NULL;
  return &PDat.inventory[i];
}

P_Item *GetPlayerItemByN(int n, int stockpile)
{
  int i;
  int c;
  c = 0;
  for(i = 0;i < STOCKMAX;i++)
  {
    if((PDat.inventory[i].inuse)&&(PDat.inventory[i].instock == stockpile)&&(PDat.inventory[i].count > 0))
    {
      c++;
      if(c == n)
        return &PDat.inventory[i];
    }
  }
  return NULL;
}

void SpendTool()
{
  P_Item *pitem;
  if(PDat.equips[E_Tool] == -1)return;
  /*no arrows to spend*/
  pitem = &PDat.inventory[PDat.equips[E_Tool]];
  TakePlayerItem(pitem);
}

void SpendArrow()
{
  P_Item *pitem;
  if(PDat.equips[E_Arrow] == -1)return;
  /*no arrows to spend*/
  pitem = &PDat.inventory[PDat.equips[E_Arrow]];
  TakePlayerItem(pitem);
}

int GetItemEquipmentSpot(int index)
{
  int i;
  for(i = 0;i < EQUIPSLOTS;i++)
  {
    if(PDat.equips[i] == index)return i;
  }
  return -1;
}

void CastingCheck(Item *item, int next)
{
  P_Skill *spell;
  char type[16];
  if(item == NULL)return;
  if(ItemHasSpecial(item,"magic") == 1)
  {
    strcpy(type,"magic");
  }
  else if(ItemHasSpecial(item,"necromancy") == 1)
  {
    strcpy(type,"necromancy");
  }
  else if(ItemHasSpecial(item,"favor") == 1)
  {
    strcpy(type,"favor");
  }
  else if(ItemHasSpecial(item,"alchemy") == 1)
  {
    strcpy(type,"alchemy");
  }
  else return;
  if((PDat.chosenspell == -1)||(next))
  {
    SelectNextSpellofType(type);
    return;
  }
  spell = GetSpellByName(PDat.spells[PDat.chosenspell]);
  if(spell != NULL)
  {
    if(strcmp(spell->type,type)!=0)
      SelectNextSpellofType(type);
  }
}

void ToggleWeapon(int first,int second)
{
  Item *item;
  int temp;
  if((first != E_Primary1)&&(first != E_Secondary1))
  {
    return;/*bad call*/
  }
  temp = PDat.equips[first];
  PDat.equips[first] = PDat.equips[first + 1];
  PDat.equips[first +  1] = temp;
  /*do 2 handed item check and swap secondary if needed*/
  item = GetItemByName(PDat.inventory[PDat.equips[first]].name);
  CastingCheck(item,0);
  if(ItemHasSpecial(item,"2handed") == 1)
  {
    /*swap out the second item too!*/
    if(PDat.equips[second] != -1)
    {
      temp = PDat.equips[second];
      PDat.equips[second] = PDat.equips[second + 1];
      PDat.equips[second +  1] = temp;
      item = GetItemByName(PDat.inventory[PDat.equips[second]].name);
      CastingCheck(item,0);
    }
  }
  /*do item check if secondary is 2 handed*/
  if(PDat.equips[second] != -1)
  {
    item = GetItemByName(PDat.inventory[PDat.equips[second]].name);
    if(ItemHasSpecial(item,"2handed") == 1)
    {
      /*swap out the second item too!*/
      if(PDat.equips[second] != -1)
      {
        temp = PDat.equips[second];
        PDat.equips[second] = PDat.equips[second + 1];
        PDat.equips[second +  1] = temp;
        item = GetItemByName(PDat.inventory[PDat.equips[second]].name);
        CastingCheck(item,0);
      }
    }
  }
}

void TakePlayerItemByName(char name[80])
{
  P_Item *pitem;
  pitem = GetPlayerItemByName(name);
  /*can't take it if the player doesn't have it.*/
  TakePlayerItem(pitem);
}

void TakePlayerItem(P_Item *pitem)
{
  Item *item;
  int e;
  if(pitem == NULL)return;
  /*can't take it if the player doesn't have it.*/
  item = GetItemByName(pitem->name);
  if(item == NULL)
  {
    fprintf(stderr,"trying to remove player item %s, which is not in the list!\n",pitem->name);
    /*this should never happen*/
    return;
  }
  if((item->stackable)&&(pitem->count > 1))
  {
    pitem->count--;
    return;
    /*done*/
  }
  e = GetItemEquipmentSpot(pitem->index);
  if(e != -1)
  {
    PDat.equips[e] = -1;
  }
  memset(pitem,0,sizeof(P_Item));
  PDat.inventorycount--;
}

int GivePlayerItem(char name[80])
{
  P_Item *pitem;
  Item *item;
  int i;
  item = GetItemByName(name);
  if(item == NULL)
  {
    fprintf(stderr,"trying to give the player item %s, which is not in the list!\n",name);
    /*not giving the player an invalid item*/
    return -2;
  }
  if(item->stackable)
  {
    pitem = GetPlayerItemByName(name);
    if((pitem != NULL)&&(pitem->instock == 0))
    {
      pitem->count++;
      return pitem->index;
    }
  }
  /*at this point, if we had the item on us, we would have incremented the counter by now.*/
  if(PDat.inventorycount >= STOCKMAX)
  {
    /*can't hold anything more.  Should not happen!*/
    return -1;
  }
  /*find first empty slot in player's inventory*/
  for(i = 0;i < STOCKMAX;i++)
  {
    if(PDat.inventory[i].inuse == 0)
    {
      PDat.inventory[i].inuse = 1;
      strcpy(PDat.inventory[i].name,name);
      PDat.inventory[i].count = 1;
      PDat.inventory[i].instock = 0;
      PDat.inventory[i].index = i;
      PDat.inventorycount++;
      return i;
    }
  }
  return -1;
}

P_Item *GetStockpileItemByName(char name[80])
{
  int i;
  for(i = 0;i < STOCKMAX;i++)
  {
    if((strcmp(PDat.inventory[i].name,name)==0)&&(PDat.inventory[i].instock))
    {
      return &PDat.inventory[i];
    }
  }
  return NULL;
}

P_Item *GetPlayerItemByName(char name[80])
{
  int i;
  for(i = 0;i < STOCKMAX;i++)
  {
    if((strcmp(PDat.inventory[i].name,name)==0)&&(!PDat.inventory[i].instock))
    {
      return &PDat.inventory[i];
    }
  }
  return NULL;
}

int GetGraceLevel()
{
  int i;
  int total = 0;
  for(i = 0;i < 6;i++)
  {
    total += PDat.attr[i].score;
  }
  return total;
}

void SpendPlayerGrace(int x)
{
  PDat.grace -= x;
  if(PDat.grace < 0)PDat.grace = 0;
}

int CalcGraceCost(int at)
{
  float rawcost;
  float trainfactor;
  /*calculates the cost in grace to upgraded the attribute at*/
  rawcost = GetGraceLevel();
  if(rawcost == 0)return 0;/*this should never happen*/
  rawcost = (rawcost * (rawcost +1))/50;
  trainfactor = PDat.attr[at].train * 0.01;
  rawcost -= trainfactor;
  /*adjust for alignments*/
  return rawcost;
}

int GetPlayerAttributeScore(int i)
{
  if((i < 0)||(i >= 6))return -1;
  return PDat.attr[i].score;
}

void RaisePlayerAttribute(int i)
{
  if((i < 0)||(i >= 6))return;
  PDat.attr[i].score++;
  PDat.attr[i].train = 0;
  CalcPlayerStats();
}

int GetPlayerGrace()
{
  return PDat.grace;
}

void GivePlayerGrace(int count)
{
  PDat.grace += count;
}

void GivePlayerGold(int count)
{
  PDat.gold += count;
}

int GetPlayerItemCount(char name[80])
{
  P_Item *item;
  item = GetPlayerItemByName(name);
  if(item != NULL)
  {
    return item->count;
  }
  return 0;
}

void EquipPlayerItem(int index,int where)
{
  int other;
  P_Item *pitem;
  Item *item;
  if((where < 0)||(where >= EQUIPSLOTS))return;
  pitem = GetPlayerItemByIndex(index);
  if(pitem == NULL)return;
  if(IsItemEquipped(pitem))
  {
    if(GetEquippedItemSlot(pitem) != -1)
    {
      PDat.equips[GetEquippedItemSlot(pitem)] = -1;
    }
  }
  PDat.equips[where] = index;
  switch(where)
  {
    case E_Primary1:
      other = E_Secondary1;
      break;
    case E_Primary2:
      other = E_Secondary2;
      break;
    case E_Secondary1:
      other = E_Primary1;
      break;
    case E_Secondary2:
      other = E_Primary2;
      break;
    default:
      return;
  }
  /*auto Un-equip the other item if it is two handed*/
  item = GetItemByName(PDat.inventory[PDat.equips[where]].name);
  if(ItemHasSpecial(item,"2handed") == 1)
  {
    PDat.equips[other] = -1;
  }
}

void SetPlayerClass(char name[80])
{
  int i;
  int e;
  int count;
  Item *item;
  P_Train *train;
  train = GetTrainByName(name);
  if(train == NULL)return;
  strcpy(PDat.train,name);
  GivePlayerAttributes(train->attributes);
  for(i = 0;i < 5;i++)
  {
    if(strlen(train->skills[i]) > 1)
    {
      GivePlayerSkill(train->skills[i]);
    }
   if(strlen(train->spells[i]) > 1)
    {
      GivePlayerSpell(train->spells[i]);
    }
  }
  for(i = 0;i < EQUIPSLOTS;i++)
  {
    if(strlen(train->equip[i]) > 1)
    {
      e = GivePlayerItem(train->equip[i]);
      EquipPlayerItem(e,i);
      item = GetItemByName(train->equip[i]);
      CastingCheck(item,0);
    }
  }
  for(i = 0;i < 5;i++)
  {
    if(train->itemcount[i] > 0)
    {
      for(count = 0;count < train->itemcount[i];count++)
      {
        GivePlayerItem(train->items[i]);
      }
    }
  }
  /*TODO the rest of the stats*/
}

void SetPlayerRace(char name[80])
{
  P_Race *race;
  race = GetRaceByName(name);
  if(race == NULL)return;
  strcpy(PDat.race,name);
  GivePlayerAttributes(race->attributes);
  /*TODO the rest of the stats*/
}

void ClearPlayer()
{
  ClearLevelHistory();
  memset(&PDat,0,sizeof(PlayerSave));
  memset(&PDat.equips,-1,sizeof(PDat.equips));
}

void NewPlayer(char race[80],char train[80])
{
  ClearLevelHistory();
  memset(&PDat,0,sizeof(PlayerSave));
  memset(&PDat.equips,-1,sizeof(PDat.equips));
  PDat.Version = VERSION;
  PDat.chosenspell = -1;
  strcpy(PDat.map,level.name);
  SetPlayerClass(train);
  SetPlayerRace(race);
  CalcPlayerResistance();
  CalcPlayerStats();
  PDat.H = PDat.HM;
  PDat.S = PDat.SM;
  PDat.M = PDat.MM;
}

void SetupPlayer(int x,int y)
{
  ThisLevel = GetLevelHistoryByName(level.name);
  if(ThisLevel == NULL)
  {
    ThisLevel = NewLevelHistory(level.name, level.numtiles, level.w, level.h);
    if(ThisLevel == NULL)return;
  }
  SpawnPlayer(M_Coord(x,y,0));
  CalcPlayerResistance();
  UpdateSeenMask(5);
}

void SaveLevelProgress(Progress *p,FILE *file)
{
  if((p == NULL)||(file == NULL))return;
  fwrite(p->map,sizeof(char)*80,1,file);
  fwrite(p->name,sizeof(char)*80,1,file);
  fwrite(&p->index,sizeof(int),1,file);
  fwrite(p->value,sizeof(char)*80,1,file);
}

void SaveLevelHistory(LevelHist *lh,FILE *file)
{
  int lhpcount = 0;
  Progress *p;
  if((lh == NULL)||(file == NULL))return;
  lhpcount = GetLevelProgressCount(lh);
  fwrite(lh->name,sizeof(char)*80,1,file);
  fwrite(&lh->lsize,sizeof(int),1,file);
  fwrite(&lh->w,sizeof(int),1,file);
  fwrite(&lh->h,sizeof(int),1,file);
  fwrite(lh->ldata,sizeof(int)*lh->lsize,1,file);
  fwrite(&lhpcount,sizeof(int),1,file);
  p = lh->progress;
  while(p != NULL)
  {
    /*save progress*/
    SaveLevelProgress(p,file);
    p = p->next;
  }
}

Progress *LoadPlayerLevelProgress(int count, FILE *file)
{
  int i;
  Progress *hp = NULL;
  Progress *p = NULL;
  for(i = 0;i < count;i++)
  {
    if(i == 0)
    {
      hp = (Progress *)malloc(sizeof(Progress));
      if(hp == NULL)
      {
        fprintf(stderr,"failed to allocate level progress!\n");
        return NULL;
      }
      p = hp;
    }
    else
    {
      p->next = (Progress *)malloc(sizeof(Progress));
      if(p->next == NULL)
      {
        fprintf(stderr,"failed to allocate level progress!\n");
        /*TODO free everything!*/
        return NULL;
      }
      p = p->next;
      p->next = NULL;
    }
    fread(p->map,sizeof(char)*80,1,file);
    fread(p->name,sizeof(char)*80,1,file);
    fread(&p->index,sizeof(int),1,file);
    fread(p->value,sizeof(char)*80,1,file);
  }
  return hp;
}

LevelHist *LoadPlayerLevelHistory(int count,FILE *file)
{
  int i;
  int lhpcount =  0;
  LevelHist *lh = NULL;
  LevelHist *c = NULL;
  for(i = 0;i < count;i++)
  {
    if(i == 0)
    {
      
      lh = (LevelHist *)malloc(sizeof(LevelHist));
      if(lh == NULL)
      {
        fprintf(stderr,"Unable to allocate player level history!\n");
        return NULL;
      }
      c = lh;
    }
    else
    {
      c->next = (LevelHist *)malloc(sizeof(LevelHist));
      if(c->next == NULL)
      {
        fprintf(stderr,"Unable to allocate player level history!\n");
        c = lh;
        while(c != NULL)
        {
          /*kill all previous allocations*/
          lh = c;
          c = c->next;
          /*free progress as well*/
          free(lh);
        }
        return NULL;
      }
      c = c->next;
      c->next = NULL;
    }
    fread(c->name,sizeof(char)*80,1,file);
    fread(&c->lsize,sizeof(int),1,file);
    fread(&c->w,sizeof(int),1,file);
    fread(&c->h,sizeof(int),1,file);
    c->ldata = (int *)malloc(sizeof(int)*c->lsize);
    if(c->ldata == NULL)
    {
      fprintf(stderr,"failed to allocate ldata for the level history!\n");
      /*TODO clear everything allocated*/
      return NULL;
    }
    fread(c->ldata,sizeof(int)*c->lsize,1,file);
    fread(&lhpcount,sizeof(int),1,file);
    if(lhpcount != 0)
    {
      c->progress = LoadPlayerLevelProgress(lhpcount, file);
      if(c->progress == NULL)
      {
        return NULL;
      }
    }
  }
  return lh;
}

int LoadPlayerData(char *path)
{
  int good = 1;
  int lhcount = 0;
  FILE *file;
  /*Load Base Player Data*/
  file = fopen(path,"rb");
  if(file == NULL)
  {
    fprintf(stderr,"Unable to load file %s\n",path);
    return 0;
  }
  /*TODO remove any allocated data from previous Player Dats*/
  memset(&PDat,0,sizeof(PDat));
  fread(&PDat,sizeof(PDat)-sizeof(LevelHist*),1,file);
  if(PDat.Version != VERSION)
  {
    NewMsg("Save is from a different version of the game!");
  }
  fread(&lhcount,sizeof(int),1,file);
  /*get player Levelhist count*/
  if(lhcount != 0)
  {
    PDat.levelhistory = LoadPlayerLevelHistory(lhcount,file);
    if(PDat.levelhistory == NULL)good = 0;
  }
  fclose(file);
  return good;
}

int LoadPlayerGame(char *name)
{
  char path[80];
  if(name == NULL)return 0;
  sprintf(path,"system/%s.sav",name);
  /*load player data from file*/
  if(!LoadPlayerData(path))return 0;
  /*load level based on player data.*/
  /*Spawn Player Based on Player's location*/
  return 1;
}

void SavePlayerGame()
{
  int lhcount = 0;
  LevelHist *lh;
  FILE *file;
  char filename[80];
  if(strlen(PDat.name) <= 1)
  {
    sprintf(filename,"system/unknown.sav");
  }
  else sprintf(filename,"system/%s.sav",PDat.name);
  /*TODO sync player ent info to PDat info*/
  PDat.H = PlayerEnt->health;
  PDat.S = PlayerEnt->stamina;
  PDat.M = PlayerEnt->mana;
  PDat.x = PlayerEnt->p.x;
  PDat.y = PlayerEnt->p.y;
  strcpy(PDat.map,level.name);
  file = fopen(filename,"wb");
  if(file == NULL)
  {
    fprintf(stderr,"unable to open player save file %s for saving!\n",filename);
    return;
  }
  /*write main player data*/
  fwrite(&PDat,sizeof(PDat)-sizeof(LevelHist*),1,file);
  /*write each level*/
  lhcount = GetLevelHistoryCount();
  fwrite(&lhcount,sizeof(int),1,file);
  lh = PDat.levelhistory;
  while(lh != NULL)
  {
    SaveLevelHistory(lh,file);
    lh = lh->next;
  }
  /*in each level write the progress*/
  fclose(file);
}

/*

      ---=== Player Stats Section ===---

*/

void AddAlignEvent(int score)
{
  int i;
  int total = 0;
  if(PDat.aligncount < 100 - 1)
  {
    PDat.align[PDat.aligncount] = score;
    PDat.aligncount++;
  }
  else
  {
    for(i = 0;i < 100;i++)
    {
      if(PDat.align[i] < score)
      {
        PDat.align[i] = score;
        break;
      }
    }
  }
  for(i = 0;i < PDat.aligncount;i++)
  {
    total += PDat.align[i];
  }
  if(PDat.aligncount != 0)PDat.alignment = total / PDat.aligncount;
}

void AddTendEvent(int score)
{
  int i;
  int total = 0;
  if(PDat.tendcount < 100 - 1)
  {
    PDat.tend[PDat.tendcount] = score;
    PDat.tendcount++;
  }
  else
  {
    for(i = 0;i < 100;i++)
    {
      if(PDat.tend[i] < score)
      {
        PDat.tend[i] = score;
        break;
      }
    }
  }
  for(i = 0;i < PDat.tendcount;i++)
  {
    total += PDat.tend[i];
  }
  if(PDat.tendcount != 0)PDat.tendency = total / PDat.tendcount;
}

void GivePlayerSkill(char name[80])
{
  int i;
  for(i = 0; i < SKILLMAX;i++)
  {
    if(strcmp(PDat.skills[i],name)==0)return;/*can't learn it twice*/
    if(strlen(PDat.skills[i]) == 0)
    {
      strcpy(PDat.skills[i],name);
      PDat.numskills++;
      return;
    }
  }
}

void GivePlayerSpell(char name[80])
{
  int i;
  for(i = 0; i < SPELLMAX;i++)
  {
    if(strcmp(PDat.spells[i],name)==0)return;/*can't learn it twice*/
    if(strlen(PDat.spells[i]) == 0)
    {
      strcpy(PDat.spells[i],name);
      PDat.numspells++;
      return;
    }
  }
}

float GetMagicDegree(char d[80])
{
  if(strcmp(d,"light") == 0)
  {
    return (PDat.attr[A_Foc].score * 0.5) + (PDat.attr[A_Spi].score * 0.35);
  }
  if(strcmp(d,"medium") == 0)
  {
    return (PDat.attr[A_Foc].score * 0.75) + (PDat.attr[A_Spi].score * 0.5);
  }
  if(strcmp(d,"heavy") == 0)
  {
    return (PDat.attr[A_Foc].score * 0.95) + (PDat.attr[A_Spi].score * 0.75);
  }
  if(strcmp(d,"great") == 0)
  {
    return (PDat.attr[A_Foc].score * 1.25) + (PDat.attr[A_Spi].score * 1);
  }
  else return 1;
}

float GetSpellDamage(P_Skill *spell)
{
  char *d;
  if(spell == NULL)return 0;
  /*TODO  Add in item spell bonuses*/
  d = GetSpellSpecialData(spell,"damage");
  if(d == NULL)d = GetSpellSpecialData(spell,"heal");
  if(d == NULL)return 0;
  return GetMagicDegree(d);
}

P_Skill *GetSpellByName(char name[80])
{
  int i;
  for(i = 0;i < NumSpells;i++)
  {
    if(strcmp(SpellList[i].name,name)==0)
    {
      return &SpellList[i];
    }
  }
  return NULL;
}

P_Skill *GetSkillByName(char name[80])
{
  int i;
  for(i = 0;i < NumSkills;i++)
  {
    if(strcmp(SkillList[i].name,name)==0)
    {
      return &SkillList[i];
    }
  }
  return NULL;
}

void TrainAttribute(int i, int train)
{
  /*to be adjusted for new game+*/
  PDat.attr[i].train+=train;
}

int SkillCheck(char skill[80],int bonus,int dif)
{
  int i;
  int count = 0;
  int total = 0;
  int train;
  for(i = 0;i < 6;i++)
  {
    if(SkillHasSpecial(GetSkillByName(skill),GetAttributeName(i)))
    {
      total += PDat.attr[i].score;
      count++;
    }
  }
  if(count != 0)total /= count;
  if(total + bonus + (random()*10) >= dif)
  {
    if(total >= dif)train = 2;
    else train = 4;
    for(i = 0;i < 6;i++)
    {
      if(SkillHasSpecial(GetSkillByName(skill),GetAttributeName(i)))
      {
        TrainAttribute(i,train);
      }
    }
    return  1;
  }
  for(i = 0;i < 6;i++)
  {
    if(SkillHasSpecial(GetSkillByName(skill),GetAttributeName(i)))
    {
      PDat.attr[i].train++;
    }
  }
  return 0;
}

int PlayerHasSkill(char name[80])
{
  int i;
  for(i = 0;i < SKILLMAX;i++)
  {
    if(strcmp(name,PDat.skills[i])==0)
      return 1;
  }
  return 0;
}

void SelectNextTool()
{
  Item *item;
  int i;
  int total;
  if(PDat.equips[E_Tool] == -1)
  {
    i = -1;
  }
  else
  {
    i = PDat.inventory[PDat.equips[E_Tool]].index;
  }
  total = PDat.inventorycount;
  do
  {
    i++;
    if(i >= PDat.inventorycount)i = 0;
    if(PDat.inventory[i].inuse)
    {
      if(PDat.inventory[i].instock == 0)
      {
        item = GetItemByName(PDat.inventory[i].name);
        if(item != NULL)
        {
          if(item->equips == E_Tool)
          {
            PDat.equips[E_Tool] = i;
            return;
          }
        }
      }
      total--;
    }
  }
  while(total >= 0);
  /*all else fails, select nothing, which is probably what it was to start.*/
  PDat.equips[E_Tool] = -1;
}

void SelectNextSpellofType(char type[16])
{
  int i,s;
  P_Skill *spell;
  s = PDat.chosenspell;
  i = s;
  if(s == -1)s = PDat.numspells;
  do
  {
    i++;
    if(i >= PDat.numspells)i = 0;
    if(i == s)break;
    /*all done*/
    spell = GetSpellByName(PDat.spells[i]);
    if(spell != NULL)
    {
      if(strcmp(spell->type,type)==0)
      {
        PDat.chosenspell = i;
        return;
      }
    }
  }while(1);
  /*couldn't find one that fit the type, make sure the current is of the type, or select nothing*/
  if(PDat.chosenspell == -1)return;
  spell = GetSpellByName(PDat.spells[s]);
  if(spell != NULL)
  {
    if(strcmp(spell->type,type)!=0)
    {
      PDat.chosenspell = -1;
      return;
    }
  }
}

void SelectNextSpell()
{
  Item *item;
  item = GetItemByName(PDat.inventory[PDat.equips[E_Secondary1]].name);
  CastingCheck(item,1);
}

void CalcPlayerLoad()
{
  
  /*TODO go through each item in the player's inventory and add up all the weights * count of what the player has  on his person.*/
}

void CalcPlayerStats()
{
  PDat.HM =(PDat.attr[0].score + PDat.attr[1].score)/2.0 * 12.5 + 25;
  PDat.SM =(PDat.attr[2].score + PDat.attr[3].score)/2.0 * 1.5 + 50;
  PDat.MM =(PDat.attr[4].score + PDat.attr[5].score)/2.0 * 10;
  PDat.carrymax = (PDat.attr[0].score + PDat.attr[1].score + PDat.attr[3].score )/3.0 * 10;
  PDat.speed = (PDat.attr[0].score + PDat.attr[3].score )/2.0;
}

void CalcPlayerResistance()
{
  int i;
  char *s;
  Item *item;
  int res;
  /*D_Physical, D_Pierce, D_Slash, D_Crush, D_Magic, D_Fire, D_Ice, D_Elec, D_Shadow, D_Light*/
  memset(PDat.resist,0,sizeof(PDat.resist));
  for(i = 0;i < EQUIPSLOTS;i++)
  {
    if(PDat.equips[i] != -1)
    {
      item = GetItemByName(PDat.inventory[PDat.equips[i]].name);
      if(item != NULL)
      {
        if(ItemHasSpecial(item,"shield"))
        {
          if(PlayerEnt != NULL)
          {
            PlayerEnt->guardrange = item->range;
            PlayerEnt->guardstrength = item->damage/100.0;
          }
        }
        if(ItemHasSpecial(item,"armor"))
        {
          PDat.resist[D_Physical] += item->damage;
        }
        if(ItemHasSpecial(item,"reselec"))
        {
          s = GetItemSpecialKey(item,"reselec");
          if(s != NULL)
          {
            sscanf(s,"%i",&res);
            PDat.resist[D_Elec] += res;
          }
        }
        if(ItemHasSpecial(item,"resice"))
        {
          s = GetItemSpecialKey(item,"resice");
          if(s != NULL)
          {
            sscanf(s,"%i",&res);
            PDat.resist[D_Ice] += res;
          }
        }
        if(ItemHasSpecial(item,"resfire"))
        {
          s = GetItemSpecialKey(item,"resfire");
          if(s != NULL)
          {
            sscanf(s,"%i",&res);
            PDat.resist[D_Fire] += res;
          }
        }
        if(ItemHasSpecial(item,"rescrush"))
        {
          s = GetItemSpecialKey(item,"rescrush");
          if(s != NULL)
          {
            sscanf(s,"%i",&res);
            PDat.resist[D_Crush] += res;
          }
        }
        if(ItemHasSpecial(item,"resslash"))
        {
          s = GetItemSpecialKey(item,"resslash");
          if(s != NULL)
          {
            sscanf(s,"%i",&res);
            PDat.resist[D_Slash] += res;
          }
        }
        if(ItemHasSpecial(item,"respierce"))
        {
          s = GetItemSpecialKey(item,"respierce");
          if(s != NULL)
          {
            sscanf(s,"%i",&res);
            PDat.resist[D_Pierce] += res;
          }
        }
        if(ItemHasSpecial(item,"resmagic"))
        {
          s = GetItemSpecialKey(item,"resmagic");
          if(s != NULL)
          {
            sscanf(s,"%i",&res);
            PDat.resist[D_Magic] += res;
          }
        }
        if(ItemHasSpecial(item,"reslight"))
        {
          s = GetItemSpecialKey(item,"reslight");
          if(s != NULL)
          {
            sscanf(s,"%i",&res);
            PDat.resist[D_Light] += res;
          }
        }
        if(ItemHasSpecial(item,"resshadow"))
        {
          s = GetItemSpecialKey(item,"resshadow");
          if(s != NULL)
          {
            sscanf(s,"%i",&res);
            PDat.resist[D_Shadow] += res;
          }
        }
      }
    }
  }
  if(PlayerEnt != NULL)
  {
    memcpy(PlayerEnt->resists,PDat.resist,sizeof(PDat.resist));
  }
}

int GetAttributeIndex(char name[80])
{
  if(strcmp(name,"Strength")==0)return A_Str;
  if(strcmp(name,"Endurance")==0)return A_End;
  if(strcmp(name,"Agility")==0)return A_Agi;
  if(strcmp(name,"Dexterity")==0)return A_Dex;
  if(strcmp(name,"Focus")==0)return A_Foc;
  if(strcmp(name,"Spirit")==0)return A_Spi;
  return -1;
}

char *GetAttributeName(int i)
{
  switch(i)
  {
    case A_Str:
      return "Strength";
      break;
    case A_End:
      return "Endurance";
      break;
    case A_Dex:
      return "Dexterity";
      break;
    case A_Agi:
      return "Agility";
      break;
    case A_Foc:
      return "Focus";
      break;
    case A_Spi:
      return "Spirit";
      break;
  }
  return " ";
}

void GivePlayerAttributes(int attr[6])
{
  int i;
  for(i = 0;i < 6;i++)
  {
    PDat.attr[i].score += attr[i];
  }
}

void LoadRaceList()
{
  int i = 0;
  int j = 0;
  FILE *file;
  char *c;
  char buf[512];
  file = fopen("system/racelist.def","r");
  if(file == NULL)
  {
    fprintf(stderr,"Unable to open race file\n");
    return;
  }
  memset(&RaceList,0,sizeof(RaceList));
  i = 0;
  j = 0;
  NumRaces = 0;
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
      NumRaces++;
      j = 0;
      if(i >= 10)
      {
        fprintf(stderr,"maximum number of races loaded from file!\n");
        return;
      }
      continue;
    }
    if(strcmp(buf,"<race>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(RaceList[i].name, 80, file);
      c = strchr(RaceList[i].name, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<attributes>") ==0)
    {
      fscanf(file, "%i %i %i %i %i %i",&RaceList[i].attributes[0],&RaceList[i].attributes[1], &RaceList[i].attributes[2], &RaceList[i].attributes[3], &RaceList[i].attributes[4],&RaceList[i].attributes[5]);
      continue;
    }
    if(strcmp(buf,"<alignment>") ==0)
    {
      fscanf(file, "%i %i",&RaceList[i].align,&RaceList[i].tend);
      continue;
    }
    if(strcmp(buf,"<mutation>") ==0)
    {
      if(j < MUTATIONS)
      {
        fgetc(file);  /*clear the space before the word*/
        fgets(RaceList[i].mutations[j], 80, file);
        c = strchr(RaceList[i].mutations[j], '\n');
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
  }
}

void LoadTrainingList()
{
  int i,j,k,l,m;  /*counting variables*/
  FILE *file;
  char *c;
  char buf[512];
  file = fopen("system/trainlist.def","r");
  if(file == NULL)
  {
    fprintf(stderr,"Unable to open training file\n");
    return;
  }
  memset(&TrainList,0,sizeof(TrainList));
  i = 0;
  j = 0;
  k = 0;
  l = 0;
  NumTrains = 0;
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
      NumTrains++;
      j = 0;
      l = 0;
      k = 0;
      if(i >= 20)
      {
        fprintf(stderr,"maximum number of training types loaded from file!\n");
        return;
      }
      continue;
    }
    if(strcmp(buf,"<class>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(TrainList[i].name, 80, file);
      c = strchr(TrainList[i].name, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<attributes>") ==0)
    {
      fscanf(file, "%i %i %i %i %i %i",&TrainList[i].attributes[0],&TrainList[i].attributes[1], &TrainList[i].attributes[2], &TrainList[i].attributes[3], &TrainList[i].attributes[4], &TrainList[i].attributes[5]);
      continue;
    }
    if(strcmp(buf,"<alignment>") ==0)
    {
      fscanf(file, "%i %i",&TrainList[i].align,&TrainList[i].tend);
      continue;
    }
    if(strcmp(buf,"<skill>") ==0)
    {
      if(j < 5)
      {
        fgetc(file);  /*clear the space before the word*/
        fgets(TrainList[i].skills[j], 80, file);
        c = strchr(TrainList[i].skills[j], '\n');
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
    if(strcmp(buf,"<spell>") ==0)
    {
      if(k < 5)
      {
        fgetc(file);  /*clear the space before the word*/
        fgets(TrainList[i].spells[k], 80, file);
        c = strchr(TrainList[i].spells[k], '\n');
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
    if(strcmp(buf,"<item>") ==0)
    {
      if(l < 5)
      {
        fscanf(file, "%i",&TrainList[i].itemcount[l]);
        fgetc(file);  /*clear the space before the word*/
        fgets(TrainList[i].items[l], 80, file);
        c = strchr(TrainList[i].items[l], '\n');
        /*replace trailing return with terminating character*/
        if(c != NULL) *c = '\0';
        l++;
      }
      else
      {
        fgets(buf, sizeof(buf), file);
      }
      continue;
    }
    if(strcmp(buf,"<head>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(TrainList[i].equip[E_Head], 80, file);
      c = strchr(TrainList[i].equip[E_Head], '\n');
      /*replace trailing return with terminating character*/
      if(c != NULL) *c = '\0';
      m++;
      continue;
    }
    if(strcmp(buf,"<body>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(TrainList[i].equip[E_Body], 80, file);
      c = strchr(TrainList[i].equip[E_Body], '\n');
      /*replace trailing return with terminating character*/
      if(c != NULL) *c = '\0';
      m++;
      continue;
    }
    if(strcmp(buf,"<legs>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(TrainList[i].equip[E_Legs], 80, file);
      c = strchr(TrainList[i].equip[E_Legs], '\n');
      /*replace trailing return with terminating character*/
      if(c != NULL) *c = '\0';
      m++;
      continue;
    }
    if(strcmp(buf,"<neck>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(TrainList[i].equip[E_Neck], 80, file);
      c = strchr(TrainList[i].equip[E_Neck], '\n');
      /*replace trailing return with terminating character*/
      if(c != NULL) *c = '\0';
      m++;
      continue;
    }
    if(strcmp(buf,"<ring1>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(TrainList[i].equip[E_Ring1], 80, file);
      c = strchr(TrainList[i].equip[E_Ring1], '\n');
      /*replace trailing return with terminating character*/
      if(c != NULL) *c = '\0';
      m++;
      continue;
    }
    if(strcmp(buf,"<ring2>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(TrainList[i].equip[E_Ring2], 80, file);
      c = strchr(TrainList[i].equip[E_Ring2], '\n');
      /*replace trailing return with terminating character*/
      if(c != NULL) *c = '\0';
      m++;
      continue;
    }
    if(strcmp(buf,"<primary>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(TrainList[i].equip[E_Primary1], 80, file);
      c = strchr(TrainList[i].equip[E_Primary1], '\n');
      /*replace trailing return with terminating character*/
      if(c != NULL) *c = '\0';
      m++;
      continue;
    }
    if(strcmp(buf,"<primary2>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(TrainList[i].equip[E_Primary2], 80, file);
      c = strchr(TrainList[i].equip[E_Primary2], '\n');
      /*replace trailing return with terminating character*/
      if(c != NULL) *c = '\0';
      m++;
      continue;
    }
    if(strcmp(buf,"<secondary>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(TrainList[i].equip[E_Secondary1], 80, file);
      c = strchr(TrainList[i].equip[E_Secondary1], '\n');
      /*replace trailing return with terminating character*/
      if(c != NULL) *c = '\0';
      m++;
      continue;
    }
    if(strcmp(buf,"<secondary2>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(TrainList[i].equip[E_Secondary2], 80, file);
      c = strchr(TrainList[i].equip[E_Secondary2], '\n');
      /*replace trailing return with terminating character*/
      if(c != NULL) *c = '\0';
      m++;
      continue;
    }
    if(strcmp(buf,"<arrow>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(TrainList[i].equip[E_Arrow], 80, file);
      c = strchr(TrainList[i].equip[E_Arrow], '\n');
      /*replace trailing return with terminating character*/
      if(c != NULL) *c = '\0';
      m++;
      continue;
    }
    if(strcmp(buf,"<tool>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(TrainList[i].equip[E_Tool], 80, file);
      c = strchr(TrainList[i].equip[E_Tool], '\n');
      /*replace trailing return with terminating character*/
      if(c != NULL) *c = '\0';
      m++;
      continue;
    }
  }
}


void LoadAbilityList(P_Skill ablist[], int *count, char filename[80],int max)
{
  int i = 0;
  int j = 0;
  FILE *file;
  char *c;
  char buf[512];
  if(count == NULL)return;
  if(ablist == NULL)return;
  file = fopen(filename,"r");
  if(file == NULL)
  {
    fprintf(stderr,"Unable to open file %s\n",filename);
    return;
  }
  i = 0;
  j = 0;
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
      *count = i;
      j = 0;
      if(i >= max)
      {
        fprintf(stderr,"maximum number of ability types loaded from %s!\n",filename);
        return;
      }
      continue;
    }
    if((strcmp(buf,"<skill>") ==0)||(strcmp(buf,"<spell>") ==0))
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(ablist[i].name, 80, file);
      c = strchr(ablist[i].name, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<icon>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(ablist[i].icon, 80, file);
      c = strchr(ablist[i].icon, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<desc>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(ablist[i].description, 512, file);
      c = strchr(ablist[i].description, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<sprite>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(ablist[i].sprite, 80, file);
      c = strchr(ablist[i].sprite, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<type>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(ablist[i].type, 16, file);
      c = strchr(ablist[i].type, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<cost>") ==0)
    {
      fscanf(file, "%i %i %i %i %i %i",&ablist[i].cost[0], &ablist[i].cost[1], &ablist[i].cost[2], &ablist[i].cost[3], &ablist[i].cost[4], &ablist[i].cost[5]);
      continue;
    }
    if(strcmp(buf,"<range>") ==0)
    {
      fscanf(file, "%i",&ablist[i].range);
      continue;
    }
    if(strcmp(buf,"<price>") ==0)
    {
      fscanf(file, "%i",&ablist[i].price);
      continue;
    }
    if(strcmp(buf,"<special>") ==0)
    {
      if(j < 16)
      {
        fgetc(file);  /*clear the space before the word*/
        fgets(ablist[i].special[j], 80, file);
        c = strchr(ablist[i].special[j], '\n');
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
  }
  
}

void LoadSkillList()
{
  memset(&SkillList,0,sizeof(SkillList));
  LoadAbilityList(SkillList, &NumSkills, "system/skilllist.def",32);
}

void LoadSpellList()
{
  memset(&SpellList,0,sizeof(SpellList));
  LoadAbilityList(SpellList, &NumSpells, "system/spelllist.def",64);
}

/*eol@eof*/

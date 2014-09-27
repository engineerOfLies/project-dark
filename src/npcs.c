#include "npcs.h"

extern Uint32 NOW;
extern PlayerSave PDat;
extern Entity *PlayerEnt;
extern Level level;
extern int deadframes[2];
extern int idleframes[2];
extern int BUTTON_W;
extern int BUTTON_H;
extern SDL_Rect Camera;
extern int Pausing;
extern int FontSize[4];

NPC NPCList[MAXNPCS];
int NumNPCS = 0;

void NPCDialogWindow(NPC *npc,int id);

char BuyingItem[80];
int  BuyingPrice;
int  QuestID;
int  NPCID;
NPC *QuestNPC = NULL;
/*
  NPC Entity Section
*/

void NPCDie(Entity *self,Entity *other)
{
  /*put in player history*/
  MonsterDie(self,other);
}

void UpdateNPC(Entity *self)
{
  switch(self->state)
  {
    case MS_Pathing:
    case MS_Attacking:
    case MS_Hostile:
    case MS_Stunned:
    case MS_Alert:
      UpdateMonster(self);
      break;
    case MS_Dead:
      if((self->frame < deadframes[0])||(self->frame > deadframes[1]))
      {
        self->frame = deadframes[0] - 1;
      }
      self->frame++;
      if(self->frame != deadframes[1])return;
      SetPlayerProgress(level.name,self->name, self->objindex,"dead");
      /*spawndeadbody, unless nocorpse*/
      self->state = MS_Deadbody;
      self->update = NULL;
      self->think = NULL;
      self->solid = 0;
      self->FloorDraw = 1;
      self->activate = NULL;
      self->touch = NULL;
      self->team = 0;
      return;
    case MS_Deadbody:
      return;
  }
  /*otherwise just run through idle animation*/
}

void NPCThink(Entity *self)
{
  switch(self->state)
  {
    case MS_Pathing:
    case MS_Hostile:
    case MS_Attacking:
    case MS_Alert:
      MonsterThink(self);
      break;
  }
}

void NPCAlert(Entity *self, Entity *other)
{
  self->team = 2;
  self->activate = NULL;
  self->think = MonsterThink;
  MonsterAlert(self, other);
}

void NPCTalk(Entity *self, Entity *other)
{
  int i;
  NPC *npc;
  i = (int)(random() * 10);
  /*start Dialog*/
  switch(i)
  {
    case 0:
      NewMessage("Hello Good Sir!",IndexColor(White));
      break;
    case 1:
      NewMessage("Greetings!",IndexColor(White));
      break;
    case 2:
      NewMessage("How are you doing?",IndexColor(White));
      break;
    case 3:
      NewMessage("Good Day",IndexColor(White));
      break;
    case 4:
      NewMessage("Haldo",IndexColor(White));
      break;
    case 5:
      NewMessage("Stay Safe",IndexColor(White));
      break;
    case 6:
      NewMessage("I hope you are well.",IndexColor(White));
      break;
    default:
      NewMessage("Hello Good Sir!",IndexColor(White));
      break;
  }
  npc = GetNPCByName(self->name);
  if(npc == NULL)return;
  NPCDialogWindow(npc,self->objindex);
}

void SpawnNPCGeneric(int x, int y,int r,Object *obj)
{
  Entity *self;
  char *buf;
  Progress *p = NULL;
  NPC *npc;
  Monster *monster;
  self = NewEntity();
  if(self == NULL)return;
  if(obj != NULL)
  {
    self->objindex = obj->id;
    p = GetProgressByIndexMap(level.name,obj->id);
    if((p != NULL)&&(strcmp(p->value,"dead")==0))
    {
      FreeEntity(self);
      return;
    }
  }
  else strcpy(self->name,"npc");
  if(!GetObjKey(obj->data,"name",&buf))
  {
    FreeEntity(self);
    return;
  }
  strcpy(self->name,buf);
  npc = GetNPCByName(self->name);
  if(npc == NULL)
  {
    fprintf(stderr,"unable to find NPC %s!",self->name);
    FreeEntity(self);
    return;
  }
  monster = GetMonsterByName(npc->monster);
  if(monster == NULL)
  {
    fprintf(stderr,"failed to retreive monster data for %s!\n",self->name);
    FreeEntity(self);
    return;
  }
  self->radius = monster->radius;
  self->p.x = x;
  self->p.y = y;
  self->p.z = 0;
  self->r.z = r;
  self->s.x = self->s.y = self->s.z = monster->scale;
  self->think = NPCThink;
  self->ThinkRate = 50;
  self->update = UpdateNPC;
  self->UpdateRate = 30;
  self->activate = NPCTalk;
  self->hit = NPCAlert;/*bad touch*/
  self->die = NPCDie;
  self->texture = LoadSprite(monster->sprite,monster->sx,monster->sy);
  self->box.w = 1;
  self->box.d = 1;
  self->box.h = 1;
  self->box.x = self->box.w/2;
  self->box.y = self->box.h/2;
  self->box.z = self->box.d/2;
  self->shaderProg = 0;
  self->team = 0;
  memcpy(self->data,obj->data,sizeof(obj->data));
  self->solid = 1;
  self->frame = 0;
  self->sightblock = 0;
  self->sightdist = monster->sightdistance;
  self->sightrange = monster->sightrange;
  self->takesdamage = 1;
  self->attackrange = 30;
  self->health = self->healthmax = monster->health;
  self->stamina = self->staminamax = monster->stamina;
  self->mana = self->manamax = monster->mana;
  self->accel = 0.25;         /*to be adjusted*/
  self->speed = 5 * monster->movespeed;            /*to be calculated*/
  self->state = MS_Idle;
  self->attackspeed = monster->attackspeed;
  if(self->attackspeed == 0)self->attackspeed = 1;
  SetMonsterCombatStats(self,monster,0);
}


/*

  NPC Dialog Windows

*/

void PurchaseItem()
{
  GivePlayerItem(BuyingItem);
  PDat.gold -= BuyingPrice;
  NewMsg("Thank you for your business!");
}

void PurchaseSpell()
{
  GivePlayerSpell(BuyingItem);
  PDat.gold -= BuyingPrice;
  NewMsg("Use this knowledge well!");
}

void PurchaseSkill()
{
  GivePlayerSkill(BuyingItem);
  PDat.gold -= BuyingPrice;
  NewMsg("I hope this comes in handy!");
}

int UpdateNPCShopWindow(HUDInfo *self,int pressID)
{
  char text[80];
  int c;
  int Good;
  NPC *npc;
  P_Skill *skill;
  Item *item;
  npc = (NPC *)self->ib;
  c = GetShopCount(npc);
  switch(pressID)
  {
    case 0:
      PopWindow(self->Handle);
      return 1;
      break;
  }
  if((pressID >= 1)&&(pressID <= c))
  {
    if(strcmp(npc->shop[pressID - 1].type,"item")==0)
    {
      item = GetItemByName(npc->shop[pressID - 1].name);
      if(item == NULL)return 0;
      if(PDat.gold >= (item->cost * npc->shop[pressID - 1].markup))
      {
        BuyingPrice = (item->cost * npc->shop[pressID - 1].markup);
        strcpy(BuyingItem,item->name);
        sprintf(text,"Buy %s for %iG?",BuyingItem,BuyingPrice);
        YesNo(text,PurchaseItem,NULL);
      }
      else
      {
        NewMessage("You can't afford this!",IndexColor(White));
        NewMessage("Come back when you have more money!",IndexColor(White));
      }
    }
    else if(strcmp(npc->shop[pressID - 1].type,"service")==0)
    {
      if(strcmp(npc->shop[pressID - 1].name,"Rest")==0)
      {
        /*TODO:advance the clock some time...*/
        /*TODO address status effects*/
        NewMsg("After some rest, you feel better.");
        PlayerEnt->health = PlayerEnt->healthmax;
        PlayerEnt->mana = PlayerEnt->manamax;
        PlayerEnt->stamina = PlayerEnt->staminamax;
      }
      else if(strcmp(npc->shop[pressID - 1].name,"Inventory")==0)
      {
        /*TODO Open up Stockpile menu*/
        NewMsg("To Be Added.");
      }
      else if(strcmp(npc->shop[pressID - 1].name,"Upgrade")==0)
      {
        /*TODO Open up Stockpile menu*/
        NewMsg("To Be Added.");
      }
    }
    else if(strcmp(npc->shop[pressID - 1].type,"spell")==0)
    {
      skill = GetSpellByName(npc->shop[pressID - 1].name);
      if(skill == NULL)return 0;
      if(PDat.gold >= (skill->price * npc->shop[pressID - 1].markup))
      {
        if(strcmp(skill->type,"favor")==0)
        {
          if(GetPlayerItemByName("Wood Relic")!= NULL)/*TODO add other favor castors as needed*/
          {
            BuyingPrice = (skill->price * npc->shop[pressID - 1].markup);
            strcpy(BuyingItem,skill->name);
            sprintf(text,"Learn the divine favor %s for %iG?",BuyingItem,BuyingPrice);
            YesNo(text,PurchaseSpell,NULL);
          }
          else
          {
            NewMsg("In order to learn this, you will need a Holy Relic");
          }
        }
        if(strcmp(skill->type,"alchemy")==0)
        {
          if(GetPlayerItemByName("White Stone")!= NULL)/*TODO add other favor castors as needed*/
          {
            BuyingPrice = (skill->price * npc->shop[pressID - 1].markup);
            strcpy(BuyingItem,skill->name);
            sprintf(text,"Learn how to make %ss for %iG?",BuyingItem,BuyingPrice);
            YesNo(text,PurchaseSpell,NULL);
          }
          else
          {
            NewMsg("In order to learn this, you will need a Philosopher's Stone!");
          }
        }
      }
      else
      {
        NewMsg("I'm afraid I can't afford to teach for free.");
      }
    }
    else if(strcmp(npc->shop[pressID - 1].type,"skill")==0)
    {
      skill = GetSkillByName(npc->shop[pressID - 1].name);
      if(skill == NULL)return 0;
      Good = 1;
      if(PDat.gold >= (skill->price * npc->shop[pressID - 1].markup))
      {
        if(SkillHasSpecial(skill,"Strength"))
        {
          if(PDat.attr[A_Str].score < 10)
          {
            Good = 0;
            NewMsg("You will need to have more strength than that to learn this skill.");
          }
        }
        if(SkillHasSpecial(skill,"Endurance"))
        {
          if(PDat.attr[A_End].score < 10)
          {
            Good = 0;
            NewMsg("I'm sorry, you just are not tough enough to handle this skill.");
          }
        }
        if(SkillHasSpecial(skill,"Agility"))
        {
          if(PDat.attr[A_Agi].score < 10)
          {
            Good = 0;
            NewMsg("You will need to work on your footwork before I can teach you this.");
          }
        }
        if(SkillHasSpecial(skill,"Dexterity"))
        {
          if(PDat.attr[A_Dex].score < 10)
          {
            Good = 0;
            NewMsg("I'm sorry, but you have stupid hands.  If you get new ones come back.");
          }
        }
        if(SkillHasSpecial(skill,"Focus"))
        {
          if(PDat.attr[A_Foc].score < 10)
          {
            Good = 0;
            NewMsg("If you can't concentrate,  I can't teach you.");
          }
        }
        if(SkillHasSpecial(skill,"Spirit"))
        {
          if(PDat.attr[A_Str].score < 10)
          {
            Good = 0;
            NewMsg("You lack the strength of will to understand what I haave to teach you.");
          }
        }
        if(Good)
        {
          BuyingPrice = (skill->price * npc->shop[pressID - 1].markup);
          strcpy(BuyingItem,skill->name);
          sprintf(text,"Learn the %s skill for %iG?",BuyingItem,BuyingPrice);
          YesNo(text,PurchaseSkill,NULL);
        }
      }
      else
      {
        NewMsg("Training Takes time and MONEY.  Come back when you have more of the later.");
      }
    }
    return 1;
  }
  return 0;
}

void DrawNPCShopWindow(HUDInfo *self)
{
  char text[80];
  NPC *npc;
  npc = (NPC *)self->ib;
  DrawImageWindow(self->box.x,self->box.y,self->box.w,self->box.h);
  if(npc != NULL)
  {
    sprintf(text,"%s Offers",npc->name);
    DrawTextCenteredShadow(text,self->box.x + self->box.w/2,self->box.y + 10,IndexColor(White),F_Medium,2);
  }
}

void NPCShopWindow(NPC *npc)
{
  P_Skill *skill;
  Item *item;
  int c;
  int i;
  int d;
  int fail = 0;
  char text[80];
  HUDInfo *self;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stdout,"Big problem here, can't make NPC Dialog window.\n");
    return;
  }
  self->stealinput = 1;
  c = GetShopCount(npc);
  self->windowupdate = UpdateNPCShopWindow;
  self->windowdraw = DrawNPCShopWindow;
  self->box.w = Camera.w * 0.4;
  self->box.h = (c * FontSize[F_Small]) + (BUTTON_H*2) + 10;
  self->box.x = Camera.w/2 - self->box.w/2;
  self->box.y = Camera.h * 0.2;
  PicButton(self,0,SDLK_RETURN,"Done",self->box.x + self->box.w/2  - (BUTTON_W/2),self->box.y + self->box.h - BUTTON_H - 4,BUTTON_W,BUTTON_H);
  self->ib = (void *)npc;
  for(i = 0; i < c; i++)
  {
    /*this will need to be corrected for items that are conditional*/
    for(d = 0;d < 4;d++)
    {
      if(!IsConditionMet(npc->shop[i].condition[d]))fail = 1;
    }
    if(!fail)
    {
      if(strcmp(npc->shop[i].type,"item")==0)
      {
        item = GetItemByName(npc->shop[i].name);
        if(item == NULL)continue;
        sprintf(text,"%s - %iG",npc->shop[i].name,(int)(item->cost * npc->shop[i].markup));
      }
      else if(strcmp(npc->shop[i].type,"spell")==0)
      {
        skill = GetSpellByName(npc->shop[i].name);
        if(skill == NULL)continue;
        sprintf(text,"(Spell) %s - %iG",npc->shop[i].name,(int)(skill->price * npc->shop[i].markup));
      }
      else if(strcmp(npc->shop[i].type,"skill")==0)
      {
        skill = GetSkillByName(npc->shop[i].name);
        if(skill == NULL)continue;
        sprintf(text,"(Skill) %s - %iG",npc->shop[i].name,(int)(skill->price * npc->shop[i].markup));
      }
      else if(strcmp(npc->shop[i].type,"service")==0)
      {
        sprintf(text,"%s",npc->shop[i].name);
      }
      TextButton(self,i + 1,SDLK_1 + i,text,self->box.x + self->box.w/2  - (BUTTON_W/2),self->box.y + BUTTON_H + ((FontSize[F_Small] + 1)*i),BUTTON_W,FontSize[F_Small] + 1);
    }
  }
}


void CompleteQuest()
{
  char buf[80];
  char text[80];
  char map[80];
  int  id;
  Progress *p;
  P_Item *pitem;
  char *b;
  int i;
  int g;
  if(QuestNPC == NULL)return;
  /*resolve win conditions*/
  for(i = 0; i < 4;i++)
  {
    if(strlen(QuestNPC->quests[QuestID].wincond[i]) > 1)
    {
      sscanf(QuestNPC->quests[QuestID].wincond[i],"%s",buf);
      if(strcmp(buf,"return")==0)
      {
        /*first type of win condition, return an item*/
        b = strchr(QuestNPC->quests[QuestID].wincond[i], ' ');
        b++;
        /*find item by b in player's inventory*/
        pitem = GetPlayerItemByName(b);
        if(pitem != NULL)
        {
          /*by now, this should be true*/
          TakePlayerItem(pitem);
        }
      }
      else if(strcmp(buf,"kill")==0)
      {
        sscanf(QuestNPC->quests[QuestID].wincond[i],"%s %s %i",buf,map,&id);
        p = GetProgressByIndexMap(map,id);
        if(p != NULL)
        {
          if(strcmp(p->value,"dead")== 0)
          {
            sprintf(text,"Well done killing that beast!");
            NewMsg(text);
          }
        }
      }
      else if(strcmp(buf,"save")==0)
      {
        sscanf(QuestNPC->quests[QuestID].wincond[i],"%s %s %i",buf,map,&id);
        p = GetProgressByIndexMap(map,id);
        if(p != NULL)
        {
          if(strcmp(p->value,"saved")== 0)
          {
            sprintf(text,"You are a true hero, Thank you!");
            NewMsg(text);
          }
        }
      }
    }
  }
  /*give rewards*/
  for(i = 0; i < 4;i++)
  {
    if(strlen(QuestNPC->quests[QuestID].reward[i]) > 1)
    {
      sscanf(QuestNPC->quests[QuestID].reward[i],"%s",buf);
      if(strcmp(buf,"gold")==0)
      {
        b = strchr(QuestNPC->quests[QuestID].reward[i], ' ');
        b++;
        sscanf(b,"%i",&g);
        GivePlayerGold(g);
        NewMessage("Here is some Gold!",IndexColor(Gold));
      }
      else if(strcmp(buf,"item")==0)
      {
        b = strchr(QuestNPC->quests[QuestID].reward[i], ' ');
        b++;
        GivePlayerItem(b);
        sprintf(text,"Have this %s.",b);
        NewMsg(text);
      }
      else if(strcmp(buf,"skill")==0)
      {
        b = strchr(QuestNPC->quests[QuestID].reward[i], ' ');
        b++;
        GivePlayerSkill(b);
        sprintf(text,"I will teach you the skill %s.",b);
        NewMsg(text);
      }
      else if(strcmp(buf,"spell")==0)
      {
        b = strchr(QuestNPC->quests[QuestID].reward[i], ' ');
        b++;
        GivePlayerSpell(b);
        sprintf(text,"I will teach you the spell %s.",b);
        NewMsg(text);
      }
      else if(strcmp(buf,"grace")==0)
      {
        b = strchr(QuestNPC->quests[QuestID].reward[i], ' ');
        b++;
        sscanf(b,"%i",&g);
        GivePlayerGrace(g);
        sprintf(text,"You have earned %i grace.",g);
        NewMsg(text);
      }
    }
  }
  SetPlayerProgress(level.name,QuestNPC->quests[QuestID].name, NPCID,"complete");
}

int UpdateNPCQuestWindow(HUDInfo *self,int pressID)
{
  int c;
  char text[80];
  int i;
  int fail;
  Progress *p;
  NPC *npc;
  npc = (NPC *)self->ib;
  c = GetQuestCount(npc);
  switch(pressID)
  {
    case 0:
      PopWindow(self->Handle);
      return 1;
      break;
  }
  if((pressID >= 1)&&(pressID <= c))
  {
    /*if already complete, thank the player for it again*/
    p = GetProgressByNameIndexMap(level.name,npc->quests[pressID - 1].name,self->state);
    if((p != NULL)&&(strcmp(p->value,"complete") == 0))
    {
      NewMsg("Yes, you were very helpful with that one, thank you.");
      return 1;
    }
    /*check for fail condiitions*/
    /*if player has met the win conditions, give player reward and mark complete*/
    if(p != NULL)
    {
      fail = 0;
      for(i = 0; i < 4;i++)
      {
        if(!IsConditionMet(npc->quests[pressID - 1].wincond[i]))fail =1;
      }
      if(!fail)
      {
        sprintf(text,"Complete Quest %s?",npc->quests[pressID - 1].name);
        QuestID = pressID - 1;
        NPCID = self->state;
        QuestNPC = npc;
        YesNo(text,CompleteQuest,NULL);
        return 1;
      }
    }
    /*otherwise the the player about it*/
    TextBlockWindow(npc->quests[pressID - 1].desc);
    SetPlayerProgress(level.name,npc->quests[pressID - 1].name, self->state,"offered");
    return 1;
  }
  return 0;
}

void DrawNPCQuestWindow(HUDInfo *self)
{
  char text[80];
  NPC *npc;
  npc = (NPC *)self->ib;
  DrawImageWindow(self->box.x,self->box.y,self->box.w,self->box.h);
  if(npc != NULL)
  {
    sprintf(text,"%s Requests",npc->name);
    DrawTextCenteredShadow(text,self->box.x + self->box.w/2,self->box.y + 10,IndexColor(White),F_Medium,2);
  }
}

int CheckOfferCond(Quest *quest)
{
  int i;
  for(i = 0;i < 4;i++)
  {
    if(!IsConditionMet(quest->offercond[i]))return 0;
  }
  return 1;
}

void NPCQuestWindow(NPC *npc, int id)
{
  int c;
  int i;
  int y;
  HUDInfo *self;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stdout,"Big problem here, can't make NPC Dialog window.\n");
    return;
  }
  self->state = id;
  self->stealinput = 1;
  c = GetQuestCount(npc);
  self->windowupdate = UpdateNPCQuestWindow;
  self->windowdraw = DrawNPCQuestWindow;
  self->box.w = Camera.w * 0.4;
  self->box.h = (c * FontSize[F_Small]) + (BUTTON_H*2) + 10;
  self->box.x = Camera.w/2 - self->box.w/2;
  self->box.y = Camera.h * 0.2;
  PicButton(self,0,SDLK_RETURN,"Done",self->box.x + self->box.w/2  - (BUTTON_W/2),self->box.y + self->box.h - BUTTON_H - 4,BUTTON_W,BUTTON_H);
  self->ib = (void *)npc;
  for(i = 0, y = 0; i < c; i++)
  {
    /*this will need to be corrected for items that are conditional*/
    if(CheckOfferCond(&npc->quests[i]))
    {
      TextButton(self,i + 1,SDLK_1 + i,npc->quests[i].name,self->box.x + self->box.w/2  - (BUTTON_W/2),self->box.y + BUTTON_H + ((FontSize[F_Small] + 1)*i),BUTTON_W,FontSize[F_Small]);
      y++;
    }
  }
}

int UpdateNPCTalkWindow(HUDInfo *self,int pressID)
{
  int c;
  NPC *npc;
  npc = (NPC *)self->ib;
  c = GetDialogCount(npc);
  switch(pressID)
  {
    case 0:
      PopWindow(self->Handle);
      return 1;
      break;
  }
  if((pressID >= 1)&&(pressID <= c))
  {
    TextBlockWindow(npc->dialog[pressID - 1].dialog);
/*    NewMsg(npc->dialog[pressID - 1].dialog);*/
    return 1;
  }
  return 0;
}

void DrawNPCTalkWindow(HUDInfo *self)
{
  char text[80];
  NPC *npc;
  npc = (NPC *)self->ib;
  DrawImageWindow(self->box.x,self->box.y,self->box.w,self->box.h);
  if(npc != NULL)
  {
    sprintf(text,"%s",npc->name);
    DrawTextCenteredShadow(text,self->box.x + self->box.w/2,self->box.y + 10,IndexColor(White),F_Medium,2);
  }
}

void NPCTalkWindow(NPC *npc)
{
  int c;
  int i;
  HUDInfo *self;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stdout,"Big problem here, can't make NPC Dialog window.\n");
    return;
  }
  c = GetDialogCount(npc);
  self->stealinput = 1;
  self->windowupdate = UpdateNPCTalkWindow;
  self->windowdraw = DrawNPCTalkWindow;
  self->box.w = Camera.w * 0.4;
  self->box.h = (c * FontSize[F_Small]) + (BUTTON_H*2.5) + 10;
  self->box.x = Camera.w/2 - self->box.w/2;
  self->box.y = Camera.h * 0.2;
  PicButton(self,0,SDLK_RETURN,"Done",self->box.x + self->box.w/2  - (BUTTON_W/2),self->box.y + self->box.h - BUTTON_H - 4,BUTTON_W,BUTTON_H);
  self->ib = (void *)npc;
  for(i = 0; i < c; i++)
  {
    /*this will need to be corrected for items that are conditional*/
    if(IsConditionMet(npc->dialog[i].dialogcond))
    {
      TextButton(self,i + 1,SDLK_1 + i,npc->dialog[i].topic,self->box.x + self->box.w/2  - (BUTTON_W/2),self->box.y + BUTTON_H + ((FontSize[F_Small] + 1)*i),BUTTON_W,FontSize[F_Small]);
    }
  }
}

void DrawNPCDialogWindow(HUDInfo *self)
{
  NPC *npc;
  npc = (NPC *)self->ib;
  DrawImageWindow(self->box.x,self->box.y,self->box.w,self->box.h);
  if(npc != NULL)
  {
    DrawTextCenteredShadow(npc->name,self->box.x + self->box.w/2,self->box.y + 10,IndexColor(White),F_Medium,2);
  }
}


int UpdateNPCDialogWindow(HUDInfo *self,int pressID)
{
  switch(pressID)
  {
    case 3:
      NPCQuestWindow((NPC *)self->ib,self->state);
      return 1;
    case 2:
      NPCShopWindow((NPC *)self->ib);
      return 1;
    case 1:
      NPCTalkWindow((NPC *)self->ib);
      return 1;
    case 0:
      HideMouse();
      Pausing--;
      PopWindow(self->Handle);
      return 1;
  }
  return 0;
}

void NPCDialogWindow(NPC *npc,int id)
{
  int c = 0;
  HUDInfo *self;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stdout,"Big problem here, can't make NPC Dialog window.\n");
    return;
  }
  self->state = id;
  self->stealinput = 1;
  self->windowupdate = UpdateNPCDialogWindow;
  self->windowdraw = DrawNPCDialogWindow;
  self->box.w = Camera.w/3;
  self->box.x = Camera.w/2 - self->box.w/2;
  self->box.y = Camera.h * 0.2;
  self->ib = (void *)npc;
  /*Talk Option, if available*/
  if(GetDialogCount(npc))
  {
    c++;
    PicButton(self,1,SDLK_1,"Talk",self->box.x + self->box.w/2  - (BUTTON_W/2),self->box.y + (BUTTON_H + 4)*c,BUTTON_W,BUTTON_H);
  }
  /*Shop Option, if Available*/
  if(GetShopCount(npc))
  {
    c++;
    PicButton(self,2,SDLK_2,"Shop",self->box.x + self->box.w/2  - (BUTTON_W/2),self->box.y + (BUTTON_H + 4)*c,BUTTON_W,BUTTON_H);
  }
  /*Quests Option, if Available*/
  if(GetQuestCount(npc))
  {
    c++;
    PicButton(self,3,SDLK_3,"Quests",self->box.x + self->box.w/2  - (BUTTON_W/2),self->box.y + (BUTTON_H + 4)*c,BUTTON_W,BUTTON_H);
  }
  if(c == 0)
  {
    PopWindow(self->Handle);
    return;
  }
  self->box.h = (BUTTON_H + 4)*(c + 2);
  PicButton(self,0,SDLK_RETURN,"Done",self->box.x + self->box.w/2  - (BUTTON_W/2),self->box.y + self->box.h - BUTTON_H - 4,BUTTON_W,BUTTON_H);
  Pausing++;
  ShowMouse();
}

/*
  Definition Loading
*/

int IsConditionMet(char cond[80])
{
  P_Item *pitem;
  Progress *p;
  int id;
  int not = 0;
  char *b;
  char buf[80];
  char map[80];
  char name[80];
  int fail = 0;
  if(strlen(cond) > 1)
  {
    sscanf(cond,"%s",buf);
    if(strcmp(buf,"not")==0)
    {
      not = 1;
      sscanf(cond,"%s %s",name,buf);
    }
    if((strcmp(buf,"return")==0)||(strcmp(buf,"item")==0))
    {
      /*first type of win condition, return an item*/
      b = strchr(cond, ' ');
      if(b == NULL)return 1;
      b++;
      /*find item by b in player's inventory*/
      pitem = GetPlayerItemByName(b);
      if(pitem == NULL)fail = 1;
    }
    /*else if other conditions*/
    else if(strcmp(buf,"quest")==0)
    {
      sscanf(cond,"%s %s",buf,map);
      b = strchr(cond,' ');
      if(b != NULL)
      {
        b++;
        b = strchr(b,' ');
        if(b != NULL)b++;
      }
      p = GetProgressByQuestName(b);
      if(strcmp(map,"complete")==0)
      {
        if(p == NULL)fail = 1;
        else if(strcmp(p->value,"complete")!=0)fail = 1;
      }
      else if(strcmp(map,"incomplete")==0)
      {
        if(p != NULL)
        {
          if(strcmp(p->value,"complete")==0)fail = 1;
        }
      }
    }
    else if(strcmp(buf,"kill")==0)
    {
      sscanf(cond,"%s %s %i",buf,map,&id);
      p = GetProgressByIndexMap(map,id);
      if(p != NULL)
      {
        if(strcmp(p->value,"dead")!= 0)fail = 1;
      }
      else fail = 1;
    }
    else if(strcmp(buf,"save")==0)
    {
      sscanf(cond,"%s %s %s",buf,map,name);
      p = GetProgressByNameMap(map,name);
      if(p != NULL)
      {
        if(strcmp(p->value,"saved")!= 0)fail = 1;
      }
      else fail = 1;
    }
    else fail = 1;
  }
  if(not)return fail;
  return !fail;
}

int GetDialogCount(NPC *npc)
{
  int i,c = 0;
  for(i = 0;i < 8;i++)
  {
    if(npc->dialog[i].used)
    {
      /*TODO check the conditions before adding this one*/
      c++;
    }
  }
  return c;
}

int GetShopCount(NPC *npc)
{
  int i,c = 0;
  for(i = 0;i < 16;i++)
  {
    if(npc->shop[i].used)
    {
      /*TODO check the conditions before adding this one*/
      c++;
    }
  }
  return c;
}

int GetQuestCount(NPC *npc)
{
  int i,c = 0;
  for(i = 0;i < 8;i++)
  {
    if(npc->quests[i].used)
    {
      /*TODO check the conditions before adding this one*/
      c++;
    }
  }
  return c;
}



NPC *GetNPCByName(char name[80])
{
  int i;
  for(i = 0;i < NumNPCS;i++)
  {
    if(strcmp(name,NPCList[i].name)==0)
      return &NPCList[i];
  }
  return NULL;
}


void LoadQuest(FILE *file,int n)
{
  int i,j,k,l,m;
  char buf[512];
  char *c;
  i = j = k = l = m = 0;
  if(file == NULL)return;
  while(fscanf(file, "%s", buf) != EOF)
  {
    if(strcmp(buf,"#") ==0)
    {
      fgets(buf, sizeof(buf), file);
      continue;/*ignore the rest of the line.*/
    }
    if(strcmp(buf,"</quest>") ==0)
    {
      i++;
      j = k = l = m = 0;
      if(i >= 8)return;
      continue;
    }
    if(strcmp(buf,"<quest>") ==0)
    {
      NPCList[n].quests[i].used = 1;
      fgetc(file);  /*clear the space before the word*/
      fgets(NPCList[n].quests[i].name, 80, file);
      c = strchr(NPCList[n].quests[i].name, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<qdesc>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(NPCList[n].quests[i].desc, 512, file);
      c = strchr(NPCList[n].quests[i].desc, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<offer>") ==0)
    {
      if(j >= 4)
      {
        fgets(buf, sizeof(buf), file);
        continue;/*ignore the rest of the line.*/
      }
      fgetc(file);  /*clear the space before the word*/
      fgets(NPCList[n].quests[i].offercond[j], 80, file);
      c = strchr(NPCList[n].quests[i].offercond[j], '\n');
      j++;
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<fail>") ==0)
    {
      if(k >= 4)
      {
        fgets(buf, sizeof(buf), file);
        continue;/*ignore the rest of the line.*/
      }
      fgetc(file);  /*clear the space before the word*/
      fgets(NPCList[n].quests[i].failcond[k], 80, file);
      c = strchr(NPCList[n].quests[i].failcond[k], '\n');
      k++;
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<win>") ==0)
    {
      if(l >= 4)
      {
        fgets(buf, sizeof(buf), file);
        continue;/*ignore the rest of the line.*/
      }
      fgetc(file);  /*clear the space before the word*/
      fgets(NPCList[n].quests[i].wincond[l], 80, file);
      c = strchr(NPCList[n].quests[i].wincond[l], '\n');
      l++;
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<reward>") ==0)
    {
      if(m >= 4)
      {
        fgets(buf, sizeof(buf), file);
        continue;/*ignore the rest of the line.*/
      }
      fgetc(file);  /*clear the space before the word*/
      fgets(NPCList[n].quests[i].reward[m], 40, file);
      c = strchr(NPCList[n].quests[i].reward[m], '\n');
      m++;
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"</questlist>") == 0)
    {
      return;
    }
  }
}

void LoadShop(FILE *file,int n)
{
  int i,j;
  char buf[512];
  char *c;
  i = j = 0;
  if(file == NULL)return;
  while(fscanf(file, "%s", buf) != EOF)
  {
    if(strcmp(buf,"#") ==0)
    {
      fgets(buf, sizeof(buf), file);
      continue;/*ignore the rest of the line.*/
    }
    if(strcmp(buf,"</item>") ==0)
    {
      i++;
      j = 0;
      if(i >= 16)return;
      continue;
    }
    if(strcmp(buf,"<item>") ==0)
    {
      NPCList[n].shop[i].used = 1;
      fgetc(file);  /*clear the space before the word*/
      fgets(NPCList[n].shop[i].name, 80, file);
      c = strchr(NPCList[n].shop[i].name, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<markup>") ==0)
    {
      fscanf(file, "%f",&NPCList[n].shop[i].markup);
      continue;
    }
    if(strcmp(buf,"<type>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(NPCList[n].shop[i].type, 16, file);
      c = strchr(NPCList[n].shop[i].type, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<condition>") ==0)
    {
      if(j >= 4)
      {
        fgets(buf, sizeof(buf), file);
        continue;/*ignore the rest of the line.*/
      }
      fgetc(file);  /*clear the space before the word*/
      fgets(NPCList[n].shop[i].condition[j], 80, file);
      c = strchr(NPCList[n].shop[i].condition[j], '\n');
      j++;
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"</shoplist>") == 0)
    {
      return;
    }
  }
}

void LoadDialog(FILE *file,int n)
{
  int i;
  char buf[512];
  char *c;
  i = 0;
  if(file == NULL)return;
  while(fscanf(file, "%s", buf) != EOF)
  {
    if(strcmp(buf,"#") ==0)
    {
      fgets(buf, sizeof(buf), file);
      continue;/*ignore the rest of the line.*/
    }
    if(strcmp(buf,"</message>") ==0)
    {
      i++;
      if(i >= 8)return;
      continue;
    }
    if(strcmp(buf,"<message>") ==0)
    {
      NPCList[n].dialog[i].used = 1;
      fgetc(file);  /*clear the space before the word*/
      fgets(NPCList[n].dialog[i].topic, 32, file);
      c = strchr(NPCList[n].dialog[i].topic, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<dialog>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(NPCList[n].dialog[i].dialog, 256, file);
      c = strchr(NPCList[n].dialog[i].dialog, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<dialogcond>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(NPCList[n].dialog[i].dialogcond, 80, file);
      c = strchr(NPCList[n].dialog[i].dialogcond, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"</dialoglist>") == 0)
    {
      return;
    }
  }
}

void LoadNPCList()
{
  int i = 0;
  int j = 0;
  int k = 0;
  FILE *file;
  char *c;
  char buf[512];
  file = fopen("system/npclist.def","r");
  if(file == NULL)
  {
    fprintf(stderr,"Unable to open npc file\n");
    return;
  }
  memset(&NPCList,0,sizeof(NPCList));
  i = 0;
  j = 0;
  k = 0;
  NumNPCS = 0;
  while(fscanf(file, "%s", buf) != EOF)
  {
    if(strcmp(buf,"#") ==0)
    {
      fgets(buf, sizeof(buf), file);
      continue;/*ignore the rest of the line.*/
    }
    if(strcmp(buf,"</NPC>") ==0)
    {
      i++;
      NumNPCS++;
      if(i >= MAXNPCS)
      {
        fprintf(stderr,"maximum number of NPCS loaded from file!\n");
        return;
      }
      continue;
    }
    if(strcmp(buf,"<NPC>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(NPCList[i].name, 80, file);
      c = strchr(NPCList[i].name, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<monster>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(NPCList[i].monster, 80, file);
      c = strchr(NPCList[i].monster, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<questlist>") ==0)
    {
      LoadQuest(file,i);
      continue;
    }
    if(strcmp(buf,"<shoplist>") ==0)
    {
      LoadShop(file,i);
      continue;
    }
    if(strcmp(buf,"<dialoglist>") ==0)
    {
      LoadDialog(file,i);
      continue;
    }
  }
  fclose(file);
}


/*eol@eof*/

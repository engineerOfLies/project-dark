#include "levelobj.h"
#include "levelmesh.h"
#include "player.h"
#include "monsters.h"
#include "npcs.h"
#include "c_collide.h"
#include "projectiles.h"

extern Entity *PlayerEnt;
extern Level level;
extern int LevelOutside;
extern int BUTTON_W;
extern int BUTTON_H;
extern SDL_Rect Camera;
extern int Pausing;
extern int FontSize[4];
extern int deadframes[2];

void SpawnBody(int x, int y,int r,Object *obj);
void SpawnDoor(int x, int y,int r,Object *obj);
void SpawnChest(int x, int y,int r,Object *obj);
void SpawnStairs(int x, int y,int up,int rad,Object *obj);
void SpawnGateway(int x, int y,float r,Object *obj);
void SpawnBlocker(int x, int y);
void SpawnAlter(int x, int y,int r,Object *obj);
void SpawnFloorSwitch(int x, int y,int r,Object *obj);
void SpawnLevelExit(int x, int y,float r,Object *obj);
void SpawnClimbable(int x, int y,float r,Object *obj);
void StairsTouch(Entity *self,Entity *other);
void SpawnBarrel(int x, int y,float r,Object *obj);
void SpawnBench(int x, int y,float r,Object *obj);
void SpawnHostage(int x, int y,float r,Object *obj);
void SpawnTrap(int x, int y,float r,Object *obj);
void ParseInfoTag(Object *obj);
void SpawnItemDropObject(int x, int y,int r,Object *obj);
void DoorClose(Entity *self);
void DoorOpen(Entity *self);
void SpawnPlayerInfoTag(int x, int y,int r,Object *obj);
void SpawnPlayerInfo(int x, int y,int r,Object *obj);


int RaiseAttribute;

void SpawnGenericObject(Object *obj)
{
  int x,y;
  float rad = 0;    /*common enough to put here*/
  char *buf;
  char *b;
  if(obj == NULL)return;
  x = (obj->x * TILEW) + TILEW/2;
  y = (obj->y * TILEH) + TILEH/2;
  if(GetObjKey(obj->data,"spawncond",&buf))
  {
    b = strchr(buf,' ');
    if(b != NULL)
    {
      b++;
      if(!IsConditionMet(b))return;
    }
  }
  if(GetObjKey(obj->data,"facing",&buf))
  {
    sscanf(buf,"%f",&rad);
    rad = 360 - rad;
  }
  if(strcmp(obj->name,"chest") == 0)
  {
    SpawnChest(x,y,rad,obj);
    return;
  }
  if(strcmp(obj->name,"v_door") == 0)
  {
    SpawnDoor(x,y,0,obj);
    return;
  }
  if(strcmp(obj->name,"h_door") == 0)
  {
    SpawnDoor(x,y,1,obj);
    return;
  }
  if(strcmp(obj->name,"stairsup") == 0)
  {
    SpawnStairs(x,y,1,rad,obj);
    return;
  }
  if(strcmp(obj->name,"stairsdown") == 0)
  {
    SpawnStairs(x,y,0,rad,obj);
    return;
  }
  if(strcmp(obj->name,"floorswitch") == 0)
  {
    SpawnFloorSwitch(x,y,rad,obj);
    return;
  }
  if(strcmp(obj->name,"gateway") == 0)
  {
    SpawnGateway(x,y,rad,obj);
    return;
  }
  if(strcmp(obj->name,"levelexit") == 0)
  {
    SpawnLevelExit(x,y,rad,obj);
    return;
  }
  if(strcmp(obj->name,"hostage") == 0)
  {
    SpawnHostage(x,y,rad,obj);
    return;
  }
  if(strcmp(obj->name,"monster") == 0)
  {
    SpawnMonsterGeneric(x,y,rad,obj);
    return;
  }
  if(strcmp(obj->name,"npc") == 0)
  {
    SpawnNPCGeneric(x,y,rad,obj);
    return;
  }
  if(strcmp(obj->name,"infotag") == 0)
  {
    ParseInfoTag(obj);
  }
  if(strcmp(obj->name,"playerinfotag") == 0)
  {
    SpawnPlayerInfoTag(x,y,rad,obj);
  }
  if(strcmp(obj->name,"daylight") == 0)
  {
    LevelOutside = 1;
    return;
  }
  if(strcmp(obj->name,"pathblocker") == 0)
  {
    SpawnBlocker(x,y);
    return;
  }
  if(strcmp(obj->name,"alter") == 0)
  {
    SpawnAlter(x,y,rad,obj);
    return;
  }
  if(strcmp(obj->name,"barrel") == 0)
  {
    SpawnBarrel(x,y,rad,obj);
    return;
  }
  if(strcmp(obj->name,"bench") == 0)
  {
    SpawnBench(x,y,rad,obj);
    return;
  }
  if(strcmp(obj->name,"trap") == 0)
  {
    SpawnTrap(x,y,rad,obj);
    return;
  }
  if(strcmp(obj->name,"climbable") == 0)
  {
    SpawnClimbable(x,y,rad,obj);
    return;
  }
  if(strcmp(obj->name,"deadbody") == 0)
  {
    SpawnBody(x,y,rad,obj);
    return;
  }
  if(strcmp(obj->name,"itemdrop") == 0)
  {
    SpawnItemDropObject(x,y,rad,obj);
    return;
  }
}

/* Trap Section */

/*attempt disarm if done by player, otherwise toggle trap state*/
void DisarmTrap(Entity *self,Entity *other)
{
  if(strcmp(other->name,"_Switch")==0)
  {
    if(self->state == OS_Disabled)return;
    if(self->state == OS_Closed)
    {
      self->state = OS_Open;
      SetPlayerProgress(level.name,self->name, self->objindex,"open");
    }
    else if(self->state == OS_Open)
    {
      self->state = OS_Closed;
      SetPlayerProgress(level.name,self->name, self->objindex,"closed");
    }
  }
  else
  {
    /*disarm trap test*/
    SetPlayerProgress(level.name,self->name, self->objindex,"disabled");
  }
}

void ArrowTrapUpdate(Entity *self)
{
  switch(self->state)
  {
    case OS_Disabled:
      return;
    case OS_Closed:
      return;
    case OS_Open:
      if(self->Acd <= 0)
      {
        ShootArrow(self);
        self->Acd = self->attackspeed;
      }
      if(self->Acd > 0)self->Acd--;
      break;
  }
}

void SpawnArrowTrap(Entity *self,Object *obj)
{
  char *b;
  int rate;
  strcpy(self->name,"arrowtrap");
  self->texture = LoadSprite("images/models/arrow_trap.png",32,32);
  self->update = ArrowTrapUpdate;
  self->UpdateRate = 30;
  if(GetObjKey(obj->data,"rate",&b))
  {
    sscanf(b,"%i",&rate);
    self->attackspeed = rate;
  }
  if(GetObjKey(obj->data,"damage",&b))
  {
    sscanf(b,"%i",&rate);
    self->monsterdamage = rate;
  }
}

void SpawnTrap(int x, int y,float r,Object *obj)
{
  char *b;
  Progress *p;
  Entity *self;
  if(obj == NULL)return;
  p = GetProgressByNameIndexMap(level.name,self->name,obj->id);
  if((p != NULL)&&(strcmp(p->value,"disabled")==0))return;
  self = NewEntity();
  if(self == NULL)return;
  memcpy(self->data,obj->data,sizeof(obj->data));
  self->objindex = obj->id;
  self->radius = TILEW/2 - 0.2;
  self->p.x = x;
  self->p.y = y;
  self->p.z = 0;
  self->r.z = r;
  self->s.x = self->s.y = self->s.z = 0.8;
  self->touch = DisarmTrap;
  self->activate = DisarmTrap;
  self->shaderProg = 0;
  self->team = 0;
  self->FloorDraw = 1;
  self->sightblock = 0;
  self->frame = 0;
  self->solid = 0;
  self->attackrange = 100;
  if(GetObjKey(obj->data,"open",NULL))
  {
    self->state = OS_Open;
  }
  else if(GetObjKey(obj->data,"closed",NULL))
  {
    self->state = OS_Closed;
  }
  if(GetObjKey(obj->data,"trap",&b))
  {
    if(strcmp(b,"arrow")==0)
    {
      SpawnArrowTrap(self,obj);
    }
    /*other traps to come*/
  }
  p = GetProgressByNameIndexMap(level.name,self->name,obj->id);
  if(p != NULL)
  {
    if(strcmp(p->value,"closed")==0)self->state = OS_Closed;
    else if(strcmp(p->value,"open")==0)self->state = OS_Open;
  }
}

/*Hostage / Prisoner*/

void UpdateHostage(Entity *self)
{
  switch(self->state)
  {
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
}

void FreeHostage(Entity *self, Entity *other)
{
  char *buf;
  /*TODO Adjust Karma*/
  SetPlayerProgress(level.name,self->name, self->objindex,"saved");
  if(GetObjKey(self->data,"desc",&buf))
  {
    TextBlockWindow(buf);
    VectorClear(other->v);
    VectorClear(other->mv);
  }
  else 
  {
    NewMsg("You open the shackles, freeing this prisoner.");
    NewMsg("Thank you! Thank you!");
    NewMsg("The prisoner quickly escapes.");
  }
  FreeEntity(self);
}

void HitHostage(Entity *self, Entity *other)
{
  NewMsg("Please, No!");
  /*TODO Affect karma*/
}

void SpawnHostage(int x, int y,float r,Object *obj)
{
  Progress *p;
  char *buf;
  Entity *self;
  self = NewEntity();
  if(self == NULL)return;
  if(obj != NULL)
  {
    self->objindex = obj->id;
    p = GetProgressByIndexMap(level.name,obj->id);
    if((p != NULL)&&((strcmp(p->value,"dead")==0)||(strcmp(p->value,"saved")==0)))
    {
      FreeEntity(self);
      return;
    }
  }
  else return;
  if(!GetObjKey(obj->data,"name",&buf))
  {
    FreeEntity(self);
    return;
  }
  strcpy(self->name,buf);
  self->p.x = x;
  self->p.y = y;
  self->p.z = 0;
  self->r.z = r;
  self->update = UpdateHostage;
  self->UpdateRate = 30;
  self->die = MonsterDie;
  self->activate = FreeHostage;
  VectorClear(self->v);
  self->texture = LoadSprite("images/models/peasant2.png",128,128);
  self->frame = 48;
  self->s.x = self->s.y = self->s.z = 1;
  self->health = 10;
  self->team = 0;
  self->radius = 16;
  self->solid = 1;
  AddEntityToSpace(self);
  memcpy(self->data,obj->data,sizeof(obj->data));
}


/*info tag*/

void ParseInfoTag(Object *obj)
{
  Progress *p;
  char *buf;
  if(GetObjKey(obj->data,"first",&buf))
  {
    p = GetProgressByNameIndexMap(level.name,"first",obj->id);
    if((p == NULL)||(strcmp(p->value,"visited")!=0))
    {
      SetPlayerProgress(level.name,"first", obj->id,"visited");
      TextBlockWindow(buf);
      VectorClear(PlayerEnt->v);
      VectorClear(PlayerEnt->mv);
    }
  }
}

/*player info tag*/

void ActivatePlayerInfo(Entity *self,Entity *other)
{
  char *buf;
  if(other != PlayerEnt)return;
  SetPlayerProgress(level.name,self->name, self->objindex,"informed");
  if(GetObjKey(self->data,"info",&buf))
  {
    TextBlockWindow(buf);
    VectorClear(other->v);
    VectorClear(other->mv);
  }
  FreeEntity(self);
}

void SpawnPlayerInfoTag(int x, int y,int r,Object *obj)
{
  Entity *self;
  Progress *p;
  p = GetProgressByNameIndexMap(level.name,obj->name,obj->id);
  if(p != NULL)
  {
    if(strcmp(p->value,"informed")==0)
    {
      return;
    }
  }
  self = NewEntity();
  if(self == NULL)return;
  strcpy(self->name,obj->name);
  self->radius = TILEW/2 - 0.2;
  self->p.x = x;
  self->p.y = y;
  self->r.z = r;
  self->touch = ActivatePlayerInfo;
  self->objindex = obj->id;
  memcpy(self->data,obj->data,sizeof(obj->data));
  self->frame = 0;
  self->state = OS_Open;
  self->solid = 1;
  AddEntityToSpace(self);
}


/*the Alter Section*/

void DrawAlterWindow(HUDInfo *self)
{
  char buf[80];
  int i;
  DrawImageWindow(self->box.x,self->box.y,self->box.w,self->box.h);
  DrawTextCenteredShadow("Dedicate your Grace to God",self->box.x + self->box.w/2,self->box.y + 10,IndexColor(White),F_Medium,2);
  sprintf(buf,"Grace Level: %i  Earned Grace: %i",GetGraceLevel(),GetPlayerGrace());
  DrawTextCenteredShadow(buf,self->box.x + self->box.w/2,self->box.y + 10 + FontSize[F_Medium] + 1, IndexColor(Gold),F_Medium,2);
  for(i = 0;i < 6;i++)
  {
    /*display cost in grace of the attribute*/
    
    sprintf(buf,"Level %i. Dedicate %i Grace to Raise",GetPlayerAttributeScore(i),CalcGraceCost(i));
    DrawTxtShadow(buf,self->box.x + self->box.w * 0.4,self->box.y + BUTTON_H  + ((FontSize[F_Small] + 2)*(i + 1)), IndexColor(Silver),F_Small,1);
  }
}


void SpendGrace()
{
  if((RaiseAttribute >= 0)&&(RaiseAttribute < 6))
  {
    SpendPlayerGrace(CalcGraceCost(RaiseAttribute));
    RaisePlayerAttribute(RaiseAttribute);
  }
}

int UpdateAlterWindow(HUDInfo *self,int pressID)
{
  char text[80];
  switch(pressID)
  {
    case 0:
      HideMouse();
      Pausing--;
      PopWindow(self->Handle);
      return 1;
  }
  if((pressID > 0)&&(pressID <= 6))
  {
    if(GetPlayerGrace() >= CalcGraceCost(pressID - 1))
    {
      RaiseAttribute = pressID -1;
      sprintf(text,"Dedicate %i Grace to receive a boost to %s?",CalcGraceCost(pressID - 1),GetAttributeName(RaiseAttribute));
      YesNo(text,SpendGrace,NULL);
    }
  }
  return 0;
}

void AlterWindow()
{
  int i;
  HUDInfo *self;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stderr,"Big problem here, can't make Alter window.\n");
    return;
  }
  Pausing++;
  self->stealinput = 1;
  self->windowupdate = UpdateAlterWindow;
  self->windowdraw = DrawAlterWindow;
  self->box.w = Camera.w * 0.6;
  self->box.h = ((FontSize[F_Small] + 2) * 6) + (BUTTON_H * 2) + FontSize[F_Medium];
  self->box.x = Camera.w/2 - self->box.w/2;
  self->box.y = Camera.h * 0.15;
  ShowMouse();
  PicButton(self,0,SDLK_RETURN,"Done",self->box.x + self->box.w/2  - (BUTTON_W/2),self->box.y + self->box.h - BUTTON_H - 4,BUTTON_W,BUTTON_H);
  /*Talk Option, if available*/
  for(i = 0;i < 6;i++)
  {
    TextButton(self,1 + i,SDLK_1 + i,GetAttributeName(i),self->box.x + self->box.w/2 - (BUTTON_W * 1.6),self->box.y + BUTTON_H  + ((FontSize[F_Small] + 2)*(i + 1)),BUTTON_W,FontSize[F_Small]);
  }
}

void ActivateAlter(Entity *self,Entity *you)
{
  if(you == PlayerEnt)
  {
    AlterWindow();
  }
}

void SpawnAlter(int x, int y,int r,Object *obj)
{
  Entity *self;
  self = NewEntity();
  if(self == NULL)return;
  if(obj == NULL)return;
  strcpy(self->name,obj->name);
  self->radius = TILEW/2;
  self->p.x = x;
  self->p.y = y;
  self->p.z = 0;
  self->r.z = r;
  self->s.x = self->s.y = self->s.z = 1;
  self->activate = ActivateAlter;
  self->texture = LoadSprite("images/models/alter.png",64,64);
  self->team = 0;
  self->solid = 1;
  AddEntityToSpace(self);
  self->sightblock = 0;
  self->objindex = obj->id;
  memcpy(self->data,obj->data,sizeof(obj->data));
}

int UpdateClimbWindow(HUDInfo *self,int pressID)
{
  Entity *me;
  char *b;
  char key[80];
  int dif = 0;
  int x,y;
  int dx,dy;
  me = (Entity *)self->ib;
  switch(pressID)
  {
    case 1:
      if(me == NULL)return 0;
      GetObjKey(me->data,"diff",&b);
      if(SkillCheck("Climbing",0,dif))
      {
        x = (int)(me->p.x / TILEW);
        y = (int)(me->p.y / TILEH);
        switch(self->state)
        {
          case D_Left:
            strcpy(key,"left");
            break;
          case D_Right:
            strcpy(key,"right");
            break;
          case D_Up:
            strcpy(key,"up");
            break;
          case D_Down:
            strcpy(key,"down");
            break;
          default:
            strcpy(key,"left");
        }
        GetObjKey(me->data,key,&b);
        if(b != NULL)
        {
          if(strcmp(b,"level")==0)
          {
            NewMsg("You swiftly climb the wall.");
            StairsTouch(me,PlayerEnt);
            HideMouse();
            Pausing--;
            PopWindow(self->Handle);
            /*treat like stairs*/
            return 1;
          }
          sscanf(b,"%s %i %i",key,&dx,&dy);
          if(TileWalkable(x + dx, y + dy,PlayerEnt))
          {
            PlayerEnt->p.x = ((x + dx) * TILEW) + TILEW/2;
            PlayerEnt->p.y = ((y + dy) * TILEH) + TILEH/2;
            UpdateSeenMask(5);
            NewMsg("You swiftly climb the wall.");
          }
          else
          {
            NewMsg("You could scale this wall, but your path is  blocked!");
          }
        }
      }
      else
      {
        NewMsg("This wall is proving too difficult to climb now.");
      }
    case 0:
      HideMouse();
      Pausing--;
      PopWindow(self->Handle);
      return 1;
  }
  return 0;
}

void DrawClimbWindow(HUDInfo *self)
{
  Entity *me;
  char *b;
  me = (Entity *)self->ib;
  DrawImageWindow(self->box.x,self->box.y,self->box.w,self->box.h);
  DrawTextCenteredShadow("This Wall Looks Climbable",self->box.x + self->box.w/2,self->box.y + 10,IndexColor(White),F_Medium,2);
  GetObjKey(me->data,"desc",&b);
  if(b != NULL)
  {
    DrawTextBlock(b,self->box.x + (self->box.w *0.05) + 1, self->box.y + (self->box.w *0.05) + 1,IndexColor(DarkGrey),F_Small,self->box.w *0.9);
  }
}

void ClimbWindow(Entity *wall, int dir)
{
  char *b;
  HUDInfo *self;
  if(wall == NULL)return;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stderr,"Big problem here, can't make Alter window.\n");
    return;
  }
  Pausing++;
  GetObjKey(wall->data,"desc",&b);
  self->state = dir;
  self->stealinput = 1;
  self->windowupdate = UpdateClimbWindow;
  self->windowdraw = DrawClimbWindow;
  self->box.w = Camera.w / 2;
  if(b != NULL)
  {
    self->box.h = GetTextBlockHeight(b,F_Small,self->box.w*0.8) + (self->box.w * 0.1) + BUTTON_H + 8;
  }
  else self->box.h = BUTTON_H * 3;
  self->box.x = Camera.w/2 - self->box.w/2;
  self->box.y = Camera.h * 0.15;
  self->ib = (void *)wall;
  ShowMouse();
  PicButton(self,0,SDLK_RETURN,"Done",self->box.x + self->box.w/2  + 4,self->box.y + self->box.h - BUTTON_H - 4,BUTTON_W,BUTTON_H);
  PicButton(self,1,SDLK_c,"Climb",self->box.x + self->box.w/2  - BUTTON_W - 4,self->box.y + self->box.h - BUTTON_H - 4,BUTTON_W,BUTTON_H);
}

void ActivateClimb(Entity *self,Entity *you)
{
  int dir;
  if(you == PlayerEnt)
  {
    if(PlayerHasSkill("Climbing"))
    {
      if(you->p.x < (self->p.x - self->radius))dir = D_Left;
      else if(you->p.x > (self->p.x + self->radius))dir = D_Right;
      else if(you->p.y > (self->p.y + self->radius))dir = D_Down;
      else if(you->p.y < (self->p.y - self->radius))dir = D_Up;
      else return;
      ClimbWindow(self,dir);
    }
    else
    {
      NewMsg("The face of this wall looks odd.");
    }
  }
}

void SpawnClimbable(int x, int y,float r,Object *obj)
{
  Entity *self;
  self = NewEntity();
  if(self == NULL)return;
  if(obj == NULL)return;
  strcpy(self->name,obj->name);
  self->radius = 18;
  self->p.x = x;
  self->p.y = y;
  self->p.z = 0;
  self->r.z = r;
  self->s.x = self->s.y = self->s.z = 1;
  self->FloorDraw = 1;
  self->activate = ActivateClimb;
  self->texture = LoadSprite("images/effects/climbable.png",62,62);
  self->team = 0;
  self->solid = 0;
  self->sightblock = 0;
  self->objindex = obj->id;
  memcpy(self->data,obj->data,sizeof(obj->data));
}

void SpawnBlocker(int x, int y)
{
  Entity *self;
  self = NewEntity();
  if(self == NULL)return;
  sprintf(self->name,"blocker");
  self->p.x = x;
  self->p.y = y;
  self->p.z = 0;
  self->r.z = 0;
  VectorClear(self->v);
  self->s.x = self->s.y = self->s.z = 1;
  self->team = 0;
  self->radius = 18;
  self->solid = 1;
  AddEntityToSpace(self);
}

void ActivateInfo(Entity *self,Entity *other)
{
  char *b;
  Progress *p = NULL;
  if(other != PlayerEnt)return;
  p = GetProgressByIndexMap(level.name,self->objindex);
  if(p != NULL)
  {
    if(strcmp(p->value,"observed")==0)
    {
      return;
    }
  }
  if(GetObjKey(self->data,"info",&b))
  {
    TextBlockWindow(b);
    SetPlayerProgress(level.name,self->name, self->objindex,"observed");
  }
}

void SpawnPlayerInfo(int x, int y,int r,Object *obj)
{
  Entity *self;
  Progress *p = NULL;
  p = GetProgressByIndexMap(level.name,obj->id);
  if(p != NULL)
  {
    if(strcmp(p->value,"observed")==0)
    {
      return;
    }
  }
  self = NewEntity();
  if(self == NULL)return;
  sprintf(self->name,"playerinfo");
  self->radius = TILEW/2;
  self->objindex = obj->id;
  memcpy(self->data,obj->data,sizeof(obj->data));
  self->p.x = x;
  self->p.y = y;
  self->p.z = 0;
  self->touch = ActivateInfo;
  self->team = 0;
  self->solid = 0;
  self->FloorDraw = 1;
  self->sightblock = 0;
}


/*barrels destructible*/

void Destructible(Entity *self,Entity *other)
{
  char buf[80];
  char *b;
  int count;
  if(GetObjKey(self->data,"drop",&b))
  {
    sscanf(b,"%s",buf);
    if(strcmp(buf,"item")==0)
    {
      b = strchr(b,' ');
      b++;
      SpawnItemDrop(self->p.x + crandom() * 4, self->p.y + crandom() * 4,b, 1);
    }
    else if(strcmp(buf,"gold")==0)
    {
      sscanf(b,"%s %i",buf,&count);
      SpawnItemDrop(self->p.x + crandom() * 4, self->p.y + crandom() * 4,"gold", count);
    }
  }
  SetPlayerProgress(level.name,self->name, self->objindex,"broken");
  FreeEntity(self);
}

void SpawnBarrel(int x, int y,float r,Object *obj)
{
  Progress *p;
  Entity *self;
  self = NewEntity();
  if(self == NULL)return;
  p = GetProgressByIndexMap(level.name,obj->id);
  if(p != NULL)
  {
    if(strcmp(p->value,"broken")==0)
      return;
  }
  sprintf(self->name,"barrel");
  self->p.x = x;
  self->p.y = y;
  self->p.z = 0;
  self->r.z = r;
  self->die = Destructible;
  VectorClear(self->v);
  self->texture = LoadSprite("images/models/barrel.png",48,48);
  self->s.x = self->s.y = self->s.z = 1;
  self->objindex = obj->id;
  self->takesdamage = 1;
  self->health = 20;
  self->team = 0;
  self->radius = 16;
  self->solid = 1;
  AddEntityToSpace(self);
  memcpy(self->data,obj->data,sizeof(obj->data));
}

void ActivateBench(Entity *self,Entity *you)
{
  char *k;
  int id;
  Entity *target;
  if(!GetObjKey(self->data,"target",&k))return;
  sscanf(k,"%i",&id);
  target = GetEntityByObjIndex(id);
  if(target == NULL)return;
  if(target->activate == NULL)return;
  target->activate(target,you);
}

void SpawnBench(int x, int y,float r,Object *obj)
{
  Entity *self;
  self = NewEntity();
  if(self == NULL)return;
  sprintf(self->name,"bench");
  self->p.x = x;
  self->p.y = y;
  self->p.z = 0;
  self->r.z = r;
  self->activate = ActivateBench;
  VectorClear(self->v);
  self->texture = LoadSprite("images/models/bench.png",48,48);
  self->s.x = self->s.y = self->s.z = 1;
  self->objindex = obj->id;
  self->team = 0;
  self->radius = 16;
  self->solid = 1;
  AddEntityToSpace(self);
  memcpy(self->data,obj->data,sizeof(obj->data));
}

void ActivateDoor(Entity *me,Entity *you)
{
  Coord v = {0,0,0};  /*push vector*/
  char *buf;
  int dif;
  Progress *p;
  float dist;
  float optimal;
  if(me == NULL)return;
  if(me->state == OS_Closed)
  {
    if(GetObjKey(me->data,"barred",&buf))
    {
      if(strcmp(you->name,"_Switch")!=0)
      {
        NewMsg("This door is sealed tight and will not open.");
        return;
      }
    }
    else if(GetObjKey(me->data,"locked",&buf))
    {
      p = GetProgressByNameIndexMap(level.name,"lock",me->objindex);
      if((p == NULL)||((p!= NULL)&&(strcmp(p->value,"unlocked")!=0)))
      {
        NewMsg("Door is Locked.");
        /*check for lock picking!*/
        sscanf(buf,"%i",&dif);
        if((GetObjKey(me->data,"key",&buf))&&(GetPlayerItemByName(buf) != NULL))
        {
          NewMsg("You use the key to open the door!");
          SetPlayerProgress(level.name,"lock", me->objindex,"unlocked");
        }
        else if(PlayerHasSkill("Lock Picking"))
        {
          if(GetPlayerItemByName("Lock Picks") != NULL)
          {
            if(SkillCheck("Lock Picking",0,dif))
            {
              NewMsg("You managed to pick the lock and open the door.");
              SetPlayerProgress(level.name,"lock", me->objindex,"unlocked");
              GivePlayerGrace(dif);
            }
            else
            {
              NewMsg("You failed to pick this lock.");
              return;
            }
          }
          else
          {
            NewMsg("You need some tools in order to pick this lock!");
            return;
          }
        }
        else
        {
          NewMsg("You can't open it!");
          return;
        }
      }
    }
    me->state = OS_Open;
    me->think = DoorOpen;
    RemoveEntityFromSpace(me);
    me->solid = 0;
    me->sightblock = 0;
    if(you != NULL)
    {
      you->Gcd = 1;
      UpdateSeenMask(5);
    }
    SetPlayerProgress(level.name,me->name, me->objindex,"open");
  }
  else
  {
    if(GetObjKey(me->data,"barred",&buf))
    {
      if(strcmp(you->name,"_Switch")!=0)
      {
        NewMsg("This door will not close.");
        return;
      }
    }
    if(you != NULL)
    {
      v.x = you->p.x - me->p.x;
      v.y = you->p.y - me->p.y;
      dist = RelativeSize(v.x, v.y);
      optimal = me->radius + you->radius + FUDGE;
      if(dist < (optimal * optimal))
      {
        dist = sqrt(dist);
        dist += FUDGE;
        Normalize(&v);
        VectorScale(v,(optimal - dist)+2,v);
        you->v.x = 0;
        you->v.y = 0;
        you->Gcd = 1;
        PushEntity(you,v);
      }
    }
    me->state = OS_Closed;
    me->think = DoorClose;
    me->solid = 1;
    AddEntityToSpace(me);
    me->sightblock = 0;
    AddEntityToSpace(me);
    SetPlayerProgress(level.name,me->name, me->objindex,"closed");
  }
}


void SpawnDoor(int x, int y,int r,Object *obj)
{
  Entity *self;
  Progress *p;
  TileInfo *tinfo;
  int id;
  self = NewEntity();
  if(self == NULL)return;
  strcpy(self->name,obj->name);
  self->radius = TILEW/2 - 0.1;
  self->p.x = x;
  self->p.y = y;
  self->p.z = 0;
  self->r.z = 0;
  self->s.x = self->s.y = self->s.z = 0.95;
  self->activate = ActivateDoor;
  if(r)
  {
    self->layers[0] = LoadSprite("images/models/door_h_anim.png",64,64);
  }
  else
  {
    self->layers[0] = LoadSprite("images/models/door_v_anim.png",64,64);
  }
  self->ThinkRate = 30;
  self->box.w = 1;
  self->box.d = 1;
  self->box.h = 1;
  self->box.x = self->box.w/2;
  self->box.y = self->box.h/2 + 1;
  self->box.z = self->box.d/2;
  self->shaderProg = 0;
  self->team = 0;
  self->objindex = obj->id;
  self->useslayers = 1;
  memcpy(self->data,obj->data,sizeof(obj->data));
  if(GetObjKey(self->data,"secret",NULL))
  {
    p = GetProgressByNameIndexMap(level.name,"secret",obj->id);
    if((p == NULL)||(strcmp(p->value,"revealed")!=0))
    {
      self->layerframes[1] = 0;
      id = GetLevelTileXY(obj->x - 1, obj->y);
      if(id == 0)id = GetLevelTileXY(obj->x, obj->y - 1);
      if(id == 0)id = GetLevelTileXY(obj->x, obj->y + 1);
      if(id == 0)id = GetLevelTileXY(obj->x + 1, obj->y);
      if(id != 0)
      {
        /*just give up if this isn't blocking a hole in the wall.*/
        tinfo = GetTileInforByID(id);
        if(tinfo != NULL)
        {
          self->layers[1] = LoadSprite(tinfo->tile,TILEW,TILEH);
        }
      }
    }
  }
  p = GetProgressByNameIndexMap(level.name,self->name,obj->id);
  if((p != NULL)&&(strcmp(p->value,"open")==0))
  {
    self->layerframes[0] = 4;
    self->solid = 0;
    self->sightblock = 0;
    self->state = OS_Open;
    return;
  }
  if((p != NULL)&&(strcmp(p->value,"closed")==0))
  {
    self->solid = 1;
    self->layerframes[0] = 0;
    self->sightblock = 1;
    self->state = OS_Closed;
    AddEntityToSpace(self);
    return;
  }
  if(!GetObjKey(self->data,"open",NULL))
  {
    /*then we are closed, so add a tile to the mesh*/
    self->solid = 1;
    self->frame = 0;
    self->sightblock = 1;
    self->state = OS_Closed;
    AddEntityToSpace(self);
  }
  else
  {
    self->layerframes[0] = 4;
    self->solid = 0;
    self->sightblock = 0;
    self->state = OS_Open;
  }
}

void ActivateSwitch(Entity *self,Entity *you)
{
  char *k;
  int id;
  Entity *target;
  if(self->state == OS_Closed)return;
  if(!GetObjKey(self->data,"target",&k))return;
  sscanf(k,"%i",&id);
  target = GetEntityByObjIndex(id);
  if(target == NULL)return;
  if(target->activate == NULL)return;
  self->state = OS_Closed;
  self->frame = 1;
  self->solid = 0;
  RemoveEntityFromSpace(self);
  target->activate(target,self);
  SetPlayerProgress(level.name,self->name, self->objindex,"pressed");
  /*when activated by a switch, it will do more than normal*/
}

void SpawnFloorSwitch(int x, int y,int r,Object *obj)
{
  Entity *self;
  Progress *p;
  self = NewEntity();
  if(self == NULL)return;
  strcpy(self->name,"_Switch");
  self->radius = TILEW/2 - 0.2;
  self->p.x = x;
  self->p.y = y;
  self->p.z = 0;
  self->r.z = r;
  self->s.x = self->s.y = self->s.z = 0.8;
  self->touch = ActivateSwitch;
  self->texture = LoadSprite("images/models/button_switch.png",56,56);
  self->shaderProg = 0;
  self->team = 0;
  self->FloorDraw = 1;
  self->objindex = obj->id;
  memcpy(self->data,obj->data,sizeof(obj->data));
  p = GetProgressByNameIndexMap(level.name,self->name,obj->id);
  self->sightblock = 0;
  if(p != NULL)
  {
    if(strcmp(p->value,"pressed")==0)
    {
      self->frame = 1;
      self->state = OS_Closed;
      self->solid = 0;
      return;
    }
  }
  self->frame = 0;
  self->state = OS_Open;
  self->solid = 1;
  AddEntityToSpace(self);
}

void DoorClose(Entity *self)
{
  self->layerframes[0]--;
  if(self->layerframes[0] == 0)
  {
    self->think = NULL;
  }
}

void DoorOpen(Entity *self)
{
  self->layerframes[0]++;
  if(self->layerframes[0] == 4)
  {
    self->think = NULL;
  }
}


void ChestOpen(Entity *self)
{
  self->frame++;
  if(self->frame == 5)
  {
    self->think = NULL;
  }
}

void ActivateChest(Entity *self,Entity *you)
{
  int openit = 1;
  int i;
  int last = -1;
  char text[80];
  char *buf;
  char *item;
  int dif = 0;
  int itemcount;
  char *txt;
  if(self == NULL)return;
  if(self->state == OS_Closed)
  {
    if(GetObjKey(self->data,"locked",&buf))
    {
      openit = 0;
      NewMessage("Chest is Locked",IndexColor(White));
      if(PlayerHasSkill("Lock Picking"))
      {
        if(GetPlayerItemByName("Lock Picks") != NULL)
        {
          sscanf(buf,"%i",&dif);
          if(SkillCheck("Lock Picking",0,dif))
          {
            NewMsg("You managed to pick the lock and open the chest.");
            GivePlayerGrace(dif);
            openit = 1;
          }
          else
          {
            NewMsg("You failed to pick this lock.");
            return;
          }
        }
        else
        {
          NewMsg("You need some tools in order to pick this lock!");
          return;
        }
      }
      else
      {
        NewMsg("You can't open it!");
        return;
      }
    }
    if(GetObjKey(self->data,"trapped",NULL))
    {
      NewMessage("Chest is Trapped!",IndexColor(White));
      /*check if the player has an ability to detect traps.*/
      /*if so, see if it was successful*/
      /*if player has an ability to disarm traps, ask if player wants to try it.*/
      /*all else fails, sset off trap*/
      openit = 0;
    }
    if(openit == 1)
    {
      if(GetObjKey(self->data,"gold",&txt))
      {
        sscanf(txt,"%i",&itemcount);
        GivePlayerGold(itemcount);
        sprintf(text,"Found %i Gold!",itemcount);
        NewMessage(text,IndexColor(Gold));
      }
      while(GetNextObjKey(self->data,"item",&item, &last))
      {
        sscanf(item,"%i",&itemcount);
        item = strchr(item,' ');
        item++;
        if(itemcount > 1)
        {
          sprintf(text,"Found %i %ss!",itemcount,item);
          NewMessage(text,IndexColor(White));
        }
        else
        {
          sprintf(text,"Found a %s!",item);
          NewMessage(text,IndexColor(White));
        }
        for(i = 0;i < itemcount;i++)
        {
          GivePlayerItem(item);
        }
      }
      self->state = OS_Open;
      self->think = ChestOpen;
      self->ThinkRate = 90;
      SetPlayerProgress(level.name,self->name, self->objindex,"open");
    }
  }
}


void SpawnChest(int x, int y,int r,Object *obj)
{
  Entity *self;
  Progress *p = NULL;
  self = NewEntity();
  if(self == NULL)return;
  sprintf(self->name,"chest");
  self->radius = TILEW/2 - 1;
  self->p.x = x;
  self->p.y = y;
  self->p.z = 0;
  self->r.z = r;
  self->s.x = self->s.y = self->s.z = 1;
  self->activate = ActivateChest;
  self->texture = LoadSprite("images/models/chest.png",48,48);
  self->team = 0;
  self->solid = 1;
  AddEntityToSpace(self);
  self->sightblock = 0;
  self->objindex = obj->id;
  memcpy(self->data,obj->data,sizeof(obj->data));
  p = GetProgressByIndexMap(level.name,obj->id);
  if(p != NULL)
  {
    if(strcmp(p->value,"open")==0)
    {
      self->frame = 5;
      self->state = OS_Open;
      return;
    }
  }
  if(GetObjKey(self->data,"open",NULL))
  {
    self->frame = 5;
    self->state = OS_Open;
  }
  else
  {
    self->frame = 0;
    self->state = OS_Closed;
  }
}

void SpawnItemDropObject(int x, int y,int r,Object *obj)
{
  Progress *p;
  char *n;
  int count = 1;
  Entity *self;
  p = GetProgressByIndexMap(level.name,obj->id);
  if(p != NULL)
  {
    if(strcmp(p->value,"taken")==0)
    {
      return;
    }
  }
  if(GetObjKey(obj->data,"count",&n))
  {
    sscanf(n,"%i",&count);
  }
  if(GetObjKey(obj->data,"item",&n))
  {
    self = SpawnItemDrop(x,y,n,count);
    if(self != NULL)
    {
      self->objindex = obj->id;
      memcpy(self->data,obj->data,sizeof(obj->data));
    }
  }
}

void ActivateBody(Entity *self,Entity *you)
{
  char *s;
  int i;
  if(you != PlayerEnt)
  {
    /*this is when an item is picked up by the player...*/
    SetPlayerProgress(level.name,"item", self->objindex,"taken");
    return;
  }
  if(GetObjKey(self->data,"desc",&s))
  {
    TextBlockWindow(s);
  }
  else
  {
    i = (int)(random()*10);
    switch(i)
    {
      case 0:
        NewMsg("This guy didn't make it.");
        break;
      case 1:
        NewMsg("I hope I don't end up like this poor fellow.");
        break;
      case 2:
        NewMsg("This could be me soon...");
        break;
      case 3:
        NewMsg("This guy didn't make it.");
        break;
      case 4:
        NewMsg("I should say a prayer.");
        break;
      case 5:
        NewMsg("This one's journey is over...");
        break;
      case 6:
        NewMsg("Rest in Peace");
        break;
      default:
        NewMsg("Poor Soul...");
    }
  }
}

/*deadbody*/

void SpawnBody(int x, int y,int r,Object *obj)
{
  char *s;
  char buf[80];
  Entity *self;
  Entity *item;
  Progress *p = NULL;
  p = GetProgressByIndexMap(level.name,obj->id);
  if(p != NULL)
  {
    if(strcmp(p->value,"destroyed")==0)
    {
      return;
    }
  }
  self = NewEntity();
  if(self == NULL)return;
  sprintf(self->name,"deadbody");
  self->radius = TILEW/3;
  self->objindex = obj->id;
  memcpy(self->data,obj->data,sizeof(obj->data));
  self->state = MS_Deadbody;
  self->p.x = x;
  self->p.y = y;
  self->p.z = 0;
  self->r.z = r;
  self->activate = ActivateBody;
  if(GetObjKey(self->data,"sprite",&s))
  {
    sprintf(buf,"images/models/%s",s);
    self->texture = LoadSprite(buf,128,128);
  }
  else
  {
    self->texture = LoadSprite("images/models/deadbody1.png",128,128);
  }
  if(GetObjKey(self->data,"item",&s))
  {
    p = GetProgressByNameIndexMap(level.name,"item",obj->id);
    if(p == NULL)
    {
      item = SpawnItemDrop(x, y,s,1);
      if(item != NULL)
      {
        item->owner = self;
      }
    }
  }
  self->team = 0;
  self->solid = 1;
  self->FloorDraw = 1;
  AddEntityToSpace(self);
  self->sightblock = 0;
  if(GetObjKey(self->data,"open",NULL))
  {
    self->frame = 5;
    self->state = OS_Open;
  }
  else
  {
    self->frame = 0;
    self->state = OS_Closed;
  }
}

/*
  Exits
*/

void StairsTouch(Entity *self,Entity *other)
{
  if(self == NULL)return;
  if(other == NULL)return;
  if(other != PlayerEnt)return;
  other->v.x = 0;
  other->v.y = 0;
  other->mv.x = 0;
  other->mv.y = 0;
  other->target = NULL;
  ExitLevel(self->data);
}

void SpawnStairs(int x, int y,int up,int rad,Object *obj)
{
  Entity *self;
  self = NewEntity();
  if(self == NULL)return;
  sprintf(self->name,"stairs");
  self->p.x = x;
  self->p.y = y;
  self->p.z = 0;
  self->r.z = rad;
  VectorClear(self->v);
  self->s.x = self->s.y = self->s.z = 1;
  self->FloorDraw = 1;
  self->touch = StairsTouch;
  if(up)
  {
    self->texture = LoadSprite("images/models/stairs_up.png",32,32);
  }
  else
  {
    self->texture = LoadSprite("images/models/stairs_down.png",32,32);
  }
  self->box.w = 1;
  self->box.d = 1;
  self->box.h = 1;
  self->box.x = self->box.w/2;
  self->box.y = self->box.h/2 + 1;
  self->box.z = self->box.d/2;
  self->shaderProg = 0;
  self->team = 0;
  self->radius = TILEW/2;
  memcpy(self->data,obj->data,sizeof(obj->data));
  self->solid = 1;
  AddEntityToSpace(self);
}

void GatewayTouch(Entity *self,Entity *other)
{
  if(self->state == OS_Open)
  {
    StairsTouch(self,other);
    return;
  }
  if(other == PlayerEnt)
  {
    if(GetPlayerItemByName("Gate Stone"))
    {
      TakePlayerItemByName("Gate Stone");
      SetPlayerProgress(level.name,self->name, self->objindex,"fixed");
      self->state = OS_Open;
      self->frame = 1;
      GivePlayerGrace(50);
      NewMsg("You replace the missing Gate Stone.");
      NewMsg("The Rune Gate glows for second.");
    }
    else
    {
      NewMsg("This Rune Gate is not working.");
    }
  }
}

void SpawnGateway(int x, int y,float r,Object *obj)
{
  Progress *p;
  Entity *self;
  self = NewEntity();
  if(self == NULL)return;
  if(obj == NULL)return;
  strcpy(self->name,obj->name);
  self->objindex = obj->id;
  self->p.x = x;
  self->p.y = y;
  self->p.z = 0;
  self->r.z = r;
  self->FloorDraw = 1;
  VectorClear(self->v);
  self->s.x = self->s.y = self->s.z = 1;
  memcpy(self->data,obj->data,sizeof(obj->data));
  p = GetProgressByIndexMap(level.name,obj->id);
  self->activate = GatewayTouch;
  self->FloorDraw = 1;
  self->texture = LoadSprite("images/models/gateway.png",96,96);
  if(p != NULL)
  {
    if(strcmp(p->value,"fixed")==0)
    {
      self->state = OS_Open;
      self->frame = 1;
      return;
    }
  }
  else
  {
    if(GetObjKey(self->data,"broken",NULL))
    {
      self->state = OS_Closed;
    }
    else self->state = OS_Open;
  }
  if(self->state == OS_Open)
  {
    self->frame = 1;
  }
  else
  {
    self->frame = 0;
  }
  self->shaderProg = 0;
  self->team = 0;
  self->radius = 18;
  self->solid = 1;
  AddEntityToSpace(self);
}

void SpawnLevelExit(int x, int y,float r,Object *obj)
{
  Entity *self;
  self = NewEntity();
  if(self == NULL)return;
  if(obj == NULL)return;
  strcpy(self->name,obj->name);
  self->p.x = x;
  self->p.y = y;
  self->p.z = 0;
  self->r.z = r;
  VectorClear(self->v);
  self->s.x = self->s.y = self->s.z = 1;
  memcpy(self->data,obj->data,sizeof(obj->data));
  self->activate = StairsTouch;
  self->FloorDraw = 1;
  self->texture = LoadSprite("images/effects/levelexit.png",32,32);
  self->frame = 0;
  self->shaderProg = 0;
  self->team = 0;
  self->radius = 16;
  self->solid = 1;
  AddEntityToSpace(self);
}

int GetNextObjKey(char data[MAXOBJKEYS][80],char *key,char **keyval, int *last)
{
  int i;
  if(key == NULL)return 0;
  if(last == NULL)return GetObjKey(data,key,keyval);
  if(*last < 0)i = 0;
  else i = *last + 1;
  while(i < MAXOBJKEYS)
  {
    if(strncmp(data[i],key,strlen(key))==0)
    {
      if(keyval != NULL)*keyval = &data[i][strlen(key) + 1];
      *last = i;
      return 1;
    }
    i++;
  }
  *last = 0;
  return 0;
}

int GetObjKey(char data[MAXOBJKEYS][80],char *key,char **keyval)
{
  int i;
  if(key == NULL)return 0;
  for(i = 0; i < MAXOBJKEYS;i++)
  {
    if(strncmp(data[i],key,strlen(key))==0)
    {
      if(keyval != NULL)*keyval = &data[i][strlen(key) + 1];
      return 1;
    }
  }
  return 0;
}

int CanSeeTarget(Entity *self, Entity *target)
{
  Trace trace;
  Coord v = {0,0,0};
  memset(&trace,0,sizeof(Trace));
  if(target == NULL)return 0;
  v.x = target->p.x - self->p.x;
  v.y = target->p.y - self->p.y;
  if(RelativeSize(v.x,v.y) > (self->sightdist*self->sightdist))return 0;
  if(GTracePtP(self->p,target->p, 1,F_SIGHT, &trace,self))
  /*if(GTrace(self->p,v, 2,F_SOLID|F_TDAMAGE, &trace, self))*/
  {
    if(trace.hittype != 1)return 0;
    if(trace.other != target)return 0;
    return 1;
  }
  return 1;
}

int CanSeeTarget2(Coord s, Entity *target, int targetrad, Entity *ignore)
{
  Coord v;
  Trace trace;
  int good;
  if(target == NULL)return -1;
  good = 0;
  v.x = target->p.x - s.x;
  v.y = target->p.y - s.y;
  v.z = 0;
  if(LevelTrace(s,v,0.5,0, -1, &trace))
  {
    if(PointInRect(trace.POC,target->p.x - targetrad, target->p.y - targetrad,2 * targetrad, 2 * targetrad ))
    {
      good = !EntTrace(s,v, 0.2, F_SIGHT, &trace, ignore);
      if(!good)
      {
        if(trace.other == target)
        {
          good = 1;
        }
      }
    }
  }
  else
  {
    good = !EntTrace(PlayerEnt->p,v, 0.2, F_SIGHT, &trace, ignore);
    if(!good)
    {
      if(trace.other == target)
      {
        good = 1;
      }
    }
  }
  return good;
}


Entity *AcquireTarget(Entity *self)
{
  Entity *target = NULL;
  do
  {
    target = GetClosestFoe(self,target);
    if(target == NULL)continue;
    
    if(CanSeePoint(self->p, target->p,target->radius + 1, self))
    /*if(CanSeeTarget(self, target) == 1)*/
    {
      return target;
    }
  }while(target != NULL);
  return NULL;
}

Entity *GetClosestFoe(Entity *self,Entity *from)
{
  /*needs to be alive on an enemy team and in sight*/
  float range;
  float r2;
  float orange = -1;
  Coord v;
  Entity *target = NULL;
  Entity *other = NULL;
  if(self == NULL)return NULL;
  range = self->sightdist * TILEW;
  r2 = range * range;
  other = GetNextEntity(from,self);
  while(other != NULL)
  {
    if((other->team != 0)&&(other->team != self->team)&&(other->state != MS_Dead)&&(other->state != MS_Deadbody))
    {
      v.x = other->p.x - self->p.x;
      v.y = other->p.y - self->p.y;
      orange = RelativeSize(v.x,v.y);
      if(orange <= r2)
      {
        AngleVector2D(&v,self->r.z);
        if(LookingTowardsPoint(self->p, v, other->p,self->sightrange))
        {
          /*       if(CanSeeTarget(self, other) == 1)*/
          if(CanSeeTarget2(self->p, other,other->radius + 1, self))
          {
            r2 = orange - 1;/*gotta do a lot better than this one!*/
            target = other;
          }
        }
      }
    }
    other = GetNextEntity(other,self);
  }
  return target;
}

void VectorToTarget(Entity *self,Coord *v)
{
  Entity *t;
  if(self == NULL)return;
  if(self->target == NULL)return;
  if(v == NULL)return;
  t = self->target;
  v->x = t->p.x - self->p.x;
  v->y = t->p.y - self->p.y;
  v->z = 0;
  VectorNormalize(v);
}

void Walk(Entity *self)
{
  float speedbonus = 1;
  if(CoordLength(self->v) < self->accel)
  {
    self->v.x = 0;
    self->v.y = 0;
  }
  if(!self->mv.x)
  {
    self->v.x *= WDAMP;
    if(self->v.x < 1)
    {
      self->v.x = 0;
    }
  }
  if(!self->mv.y)
  {
    self->v.y *= WDAMP;
    if(self->v.y < 1)
    {
      self->v.y = 0;
    }
  }
  if((self->mv.x)||(self->mv.y))
  {
    if(self->dashing)
    {
      speedbonus = 90;
    }
    else if(self->running)
    {
      speedbonus = 1.5;
    }
    VectorNormalize (&self->mv);
    VectorScale(self->mv,self->accel,self->mv);
    if(((self->mv.x < 0) && (self->v.x > 0))||((self->mv.x > 0) && (self->v.x < 0)))self->v.x = self->mv.x;
    else  self->v.x += (self->mv.x * speedbonus);
    if(((self->mv.y < 0) && (self->v.y > 0))||((self->mv.y > 0) && (self->v.y < 0)))self->v.y = self->mv.y;
    else self->v.y += (self->mv.y * speedbonus);
    if(CoordLength(self->v) > (self->speed * speedbonus))  /*player max movement speed*/
    {
      VectorNormalize (&self->v);
      VectorScale(self->v,(self->speed * speedbonus),self->v);
    }
  }
  self->mv.x = self->mv.y = 0;
}

void WalkTo(Entity *self,Point p)
{
  if(self == NULL)return;
  p.x = (p.x * TILEW)+TILEW/2;
  p.y = (p.y * TILEH)+TILEH/2;
  self->mv.x = p.x - self->p.x;
  self->mv.y = p.y - self->p.y;
  self->r.z = DegreeFromVect(-self->mv.x, self->mv.y);
}

int InRangeOfTile(Entity *self,int x, int y)
{
  float dx,dy;
  if(self == NULL)return -1;
  x = (x * TILEW)+TILEW/2;
  y = (y * TILEH)+TILEH/2;
  dx = x - self->p.x;
  dy = y - self->p.y;
  if(4 >= RelativeSize(dx, dy))return 1;
  return 0;
}

int EntityOnTile(Entity *ent,int x, int y)
{
  int tx = x * TILEW;
  int ty = y * TILEH;
  int tw,th;
  if(ent == NULL)return 0;
  tw = TILEW + (2 *ent->radius);
  th = TILEH + (2 *ent->radius);
  tx -= ent->radius;
  ty -= ent->radius;
  return PointInRect(ent->p,tx, ty,tw, th);
}

int TileWalkable(int x, int y,Entity *ignore)
{
  Entity *ent = NULL;
  if(GetLevelTileXY(x,y))
  {
    return 0;
  }
  do
  {
    ent = GetNextEntity(ent,NULL);
    if((ent != NULL)&&(ent != ignore))
    {
      if(ent->solid)
      {
        if(EntityOnTile(ent,x,y))
        {
          return 0;
        }
      }
    }
  }while(ent != NULL);
  return 1;
}

/*chipmunk integration!*/
void RemoveEntityFromSpace(Entity *ent)
{
  cpSpace *space = NULL;
  if(ent == NULL)return;
  if(ent->shape == NULL)return;
  space = GetLevelSpace();
  if(space == NULL)return;
  if(ent->shape->body != NULL)cpSpaceRemoveBody(space, ent->shape->body);
  /*cpSpaceRemoveShape(space, ent->shape);*/
}

void AddEntityToSpace(Entity *ent)
{
  cpBody *body;
  cpSpace *space;
  if(ent == NULL)return;
  space = GetLevelSpace();
  if(space == NULL)return;
  body = cpBodyNew(1, cpMomentForCircle(1, 0, ent->radius, cpv(0,0)));
  if(body == NULL)return;
  ent->shape = cpCircleShapeNew(body, ent->radius, cpv(0,0));
  /*cpSpaceAddShape(space, ent->shape);*/
}

/*eol@eof*/

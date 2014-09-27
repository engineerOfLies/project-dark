#include "monsters.h"
#include "combat.h"
#include "c_collide.h"
#include "items.h"
#include "player.h"
#include "path.h"
#include "projectiles.h"

extern Uint32 NOW;
extern PlayerSave PDat;
extern Level level;
extern int idleframes[2];
extern int deadframes[2];
extern int walkframes[2];
extern int sidestepframes[2];
extern int stabframes[2];
extern int slashframes[2];
extern int shootframes[3];/*the last frame is fire*/
extern int castframes[3];
extern int prayframes[3];
extern int twohandframes[2];
extern int crushframes[2];
extern int blockframes[2];
extern int deflectframes[2];
extern int parryframes[2];
extern Entity *PlayerEnt;

Monster MonsterList[MONSTERMAX];
int NumMonsters = 0;

void MonsterDie(Entity *self,Entity *other)
{
  int i;
  Monster *monster;
  int goldcount;
  if(self == NULL)return;
  self->state = MS_Dead;
  self->Acd = 100;
  if(other != NULL)
  {
    if(other->target == self)
    {
      /*you killed me, now look away*/
      other->target = NULL;
    }
  }
  self->think = NULL;
  monster = GetMonsterByName(self->name);
  if(monster == NULL)return;
  goldcount = monster->gold + (random()*monster->goldchance);
  if(goldcount > 0)SpawnItemDrop(self->p.x + crandom() * 2, self->p.y + crandom() * 2,"gold", goldcount);
  /*Item Drops*/
  for(i = 0;i < 4;i++)
  {
    if(GetItemByName(monster->drops[i]) != NULL)
    {
      if(random() < monster->droprate)
      {
        SpawnItemDrop(self->p.x + crandom() * 4, self->p.y + crandom() * 4,monster->drops[i], 1);
        break;
      }
    }
  }
  /*Earned Grace*/
  GivePlayerGrace(monster->grace);
  /*TODO:  Alignment shift*/
  /*TODO:  Run on-death triggers*/
}


int GetInRange(Entity *self)
{
  if((self == NULL)||(self->target == NULL))return -1;
  if(RelativeSize(self->p.x - self->target->p.x, self->p.y - self->target->p.y) < ((self->attackrange + self->target->radius) * (self->attackrange + self->target->radius)))
  {
    self->mv.x = self->mv.y = 0;
    self->v.x = self->v.y = 0;
    return 1;/*in range!*/
  }
  if(!CanWalkTo(self->p, self->target->p,self->radius,self))
  {
    MonsterPath(self, self->target);
  }
  else
  {
    VectorToTarget(self,&self->mv);
  }
  return 0;
}

void UpdateMonster(Entity *self)
{
  int *frames;
  if(self->state == MS_Deadbody)
  {
    return;
  }
  if(self->state == MS_Dead)
  {
    /*run death animation*/
    if((self->frame < deadframes[0])||(self->frame > deadframes[1]))
    {
      self->frame = deadframes[0] - 1;
    }
    self->frame++;
    if(self->frame != deadframes[1])return;
    if(GetObjKey(self->data,"persists",NULL))
    {
      SetPlayerProgress(level.name,self->name, self->objindex,"dead");
    }
    /*spawndeadbody, unless nocorpse*/
    self->state = MS_Deadbody;
    self->update = NULL;
    self->think = NULL;
    self->solid = 0;
    self->FloorDraw = 1;
    self->activate = NULL;
    return;
  }
  if(self->attacking)
  {
    /*this is going to become a lot more robust*/
    self->attackframe++;
      switch(self->attacktype)
      {
        case AT_Slash:
          frames = slashframes;
          if(self->attackframe >=4)
          {
            if((self->attackframe > (8 * self->attackspeed))||(Slash(self,self->p,self->targetpoint,self->attackrange,35,-35,2, self->attackframe/8.0, 1, 1, self->damagetype)))
            {
              self->attacking = 0;
              AddDelays(self,8);
              self->Rcd = 8  + (8 * self->attackspeed);
              self->Acd = 4 + (4 * self->attackspeed);
            }
          }
          break;
        case AT_Crush:
          frames = crushframes;
          if(self->attackframe >=4)
          {
            if((self->attackframe > (8 * self->attackspeed))||(Crush(self,self->p,self->targetpoint,self->attackrange,35,-35,2, self->attackframe/8.0, 1, 1, self->damagetype)))
            {
              self->attacking = 0;
              AddDelays(self,8);
              self->Rcd = 8  + (8 * self->attackspeed);
              self->Acd = 4 + (4 * self->attackspeed);
            }
          }
          break;
        case AT_Stab:
          frames = stabframes;
          if(self->attackframe >=4)
          {
            if((self->attackframe > (8 * self->attackspeed))||(Stab(self,self->p,self->targetpoint,self->attackrange,2, self->attackframe/8.0, 1, 1, self->damagetype)))
            {
              self->attacking = 0;
              AddDelays(self,8);
              self->Rcd = 8  + (8 * self->attackspeed);
              self->Acd = 4 + (4 * self->attackspeed);
            }
          }
          break;
        case AT_Shoot:
          frames = shootframes;
          if((int)self->attackframe == 6)
          {
            ShootArrow(self);
          }
          if(self->attackframe > 8)
          {
            self->attacking = 0;
            AddDelays(self,8);
            self->Rcd = 8  + (8 * self->attackspeed);
            self->Acd = 4 + (4 * self->attackspeed);
          }
          break;
      }

    self->frame = frames[0] + ((frames[1] - frames[0])*(self->attackframe/(8 * self->attackspeed)));
  }
  else if(self->guard)
  {
    /*using the shield, Set left arm sprite to guard*/
    self->frame = 1;
  }
  else
  {
    /*when idle*/
    if(self->frame < idleframes[0])self->frame = idleframes[0];
    else if (self->frame > idleframes[1])self->frame = idleframes[0];
    else
    {
      self->frame = self->frame + 0.2;
    }
  }
  if(RelativeSize(self->v.x, self->v.y) > 0)
  {
    UpdateEntityPosition(self,NULL);
  }
  CoolDowns(self);
}

int MonsterAcquireTarget(Entity *self)
{
  Entity *target;
  target = GetClosestFoe(self,NULL);
  if(target == NULL)return 0;
  self->target = target;
/*  MonsterPath(self,target);*/
  self->state = MS_Hostile;
  return 1;
}

void MonsterPath(Entity *self, Entity *target)
{
  int p;
  if((self == NULL)||(target == NULL))return;
  p = GetPath(self->p.x/TILEW,self->p.y/TILEH,target->p.x/TILEW,target->p.y/TILEH,self->team,self->path,PATHMAX,target);
  if(p != -1)
  {
    CleanPath(self->path,&p,self);
    self->drawtrail = 1;
    self->pathlen = p;
    self->pathstep = 0;
    self->target = target;
    self->state = MS_Pathing;
  }
  else
  {
    /*no path to target....*/
/*    self->target = NULL;*/
    self->state = MS_Hostile;
  }
}

/*
Can See Target -> Hostile
Can't See Target ->Pathing
Have no Path ->Alert
Default -> idle
*/
void MonsterThink(Entity *self)
{
  Coord v = {0,0,0};
  if((self->state == MS_Dead)||(self->state == MS_Deadbody))
  {
    /*no time to think, we're DEAD!*/
    self->think = NULL;
    return;
  }
  if(self->Acd > 0)return;
  switch(self->state)
  {
    case MS_Stunned:
      self->stun--;
      if(self->stun <= 0)self->state = MS_Hostile;
      break;
    case MS_Idle:
      MonsterAcquireTarget(self);
      break;
    case MS_Pathing:
      if(self->target == NULL)
      {
        self->state = MS_Alert;
        break;
      }
      if(CanSeeTarget(self,self->target))
      /*if(CanWalkTo(self->p, self->target->p,self->radius,self))*/
      {
        self->state = MS_Hostile;
        break;
      }
      if(self->pathstep >= self->pathlen)
      {
        /*must reacquire target*/
        if(self->target == NULL)
        {
          self->state = MS_Alert;
        }
        else
        {
          MonsterPath(self, self->target);
        }
      }
      else
      {
        /*continue to follow the path*/
        WalkTo(self,self->path[self->pathstep]);
        if(InRangeOfTile(self,self->path[self->pathstep].x, self->path[self->pathstep].y))
        {
          self->pathstep++;
        }
      }
      break;
    case MS_Hostile:
      if(self->target == NULL)
      {
        self->state = MS_Alert;
        break;
      }
      if(!CanSeeTarget(self,self->target))
      {
        MonsterPath(self,self->target);
        break;
      }
      EntLookAtOther(self,self->target);
      if(GetInRange(self))
      {
        if(self->Rcd <= 0)
        {
          self->state = MS_Attacking;
          self->attacking = 1;
          self->attackstart = NOW;
          self->attackframe = 0;
          if(self->target != NULL)
          {
            v.x =  self->target->p.x - self->p.x;
            v.y =  self->target->p.y - self->p.y;
            VectorNormalize(&v);
            VectorScale(v,32,v);    /*using 32 as the spear length, it will be based on weapon stats*/
            self->targetpoint.x = self->p.x + v.x;
            self->targetpoint.y = self->p.y + v.y;
          }
          else
          {
            AngleVector2D(&v,self->r.z);
            VectorScale(v,32,v);    /*using 32 as the spear length, it will be based on weapon stats*/
            self->targetpoint.x = self->p.x + v.x;
            self->targetpoint.y = self->p.y + v.y;
          }
        }
      }
      break;
    case MS_Alert:
      MonsterAcquireTarget(self);
      break;
  }
  Walk(self);
}

void OldMonsterThink(Entity *self)
{
  Coord v = {0,0,0};
  if(self->state == MS_Dead)return;/*no time to think, we're DEAD!*/
  if(self->Acd <= 0)
  {
    /*only think if I can act*/
    switch(self->state)
    {
      case MS_Pathing:
        if(!MonsterAcquireTarget(self))
        {
          if(self->pathstep >= self->pathlen)
          {
            /*I have tracked to the last known location!*/
            /*must reacquire target*/
            if(self->target == NULL)
            {
              self->state = MS_Alert;
            }
            else
            {
              MonsterPath(self, self->target);
            }
          }
          else
          {
            /*continue to follow the path*/
            WalkTo(self,self->path[self->pathstep]);
            if(InRangeOfTile(self,self->path[self->pathstep].x, self->path[self->pathstep].y))
            {
              self->pathstep++;
            }
          }
        }
        break;
      case MS_Idle:
        MonsterAcquireTarget(self);
        break;
      case MS_Alert:
        /*look for trouble if no targets*/
        MonsterAcquireTarget(self);
        break;
      case MS_Hostile:
        EntLookAtOther(self,self->target);
        if(self->Acd <= 0)
        {
          if(GetInRange(self))
          {
            if(self->Rcd <= 0)
            {
              ApplyStandardDelays(self,1,0);
              self->state = MS_Attacking;
              self->attacking = 1;
              self->attackstart = NOW;
              self->attackframe = 0;
              if(self->target != NULL)
              {
                v.x =  self->target->p.x - self->p.x;
                v.y =  self->target->p.y - self->p.y;
                VectorNormalize(&v);
                VectorScale(v,32,v);    /*using 32 as the spear length, it will be based on weapon stats*/
                self->targetpoint.x = self->p.x + v.x;
                self->targetpoint.y = self->p.y + v.y;
              }
              else
              {
                AngleVector2D(&v,self->r.z);
                VectorScale(v,32,v);    /*using 32 as the spear length, it will be based on weapon stats*/
                self->targetpoint.x = self->p.x + v.x;
                self->targetpoint.y = self->p.y + v.y;
              }
            }
          }
        }
        break;
    }
  }
  Walk(self);
}

void MonsterAlert(Entity *self, Entity *other)
{
  if((self == NULL)||(other == NULL))return;
  if(strcmp(other->name,"_Switch")==0)
  {
    MonsterPath(self, PlayerEnt);
    return;
  }
  MonsterPath(self, other);
/*  self->state = MS_Hostile;*/
  if(other->owner != NULL)self->target = other->owner;
  else self->target = other;
}

void SpawnMonsterGeneric(int x, int y,int r,Object *obj)
{
  Entity *self;
  char *buf;
  Progress *p = NULL;
  Monster *monster;
  self = NewEntity();
  if(self == NULL)return;
  if(obj != NULL)
  {
    strcpy(self->name,obj->name);
    self->objindex = obj->id;
    p = GetProgressByIndexMap(level.name,obj->id);
    if((p != NULL)&&(strcmp(p->value,"dead")==0))
    {
      FreeEntity(self);
      return;
    }
  }
  else strcpy(self->name,"monster");
  if(!GetObjKey(obj->data,"monster",&buf))
  {
    FreeEntity(self);
    return;
  }
  sscanf(buf,"%s",self->name);
  monster = GetMonsterByName(self->name);
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
  self->think = MonsterThink;
  self->ThinkRate = 50;
  self->update = UpdateMonster;
  self->UpdateRate = 30;
  self->activate = MonsterAlert;
  self->touch = MonsterAlert;
  self->hit = MonsterAlert;
  self->die = MonsterDie;
  self->texture = LoadSprite(monster->sprite,monster->sx,monster->sy);
  self->shaderProg = 0;
  if(!GetObjKey(obj->data,"neutral",&buf))
    self->team = 2;
  else self->team = 0;
  memcpy(self->data,obj->data,sizeof(obj->data));
  self->solid = 1;
  self->frame = 0;
  self->sightblock = 0;
  self->sightdist = monster->sightdistance;
  self->sightrange = monster->sightrange;
  self->takesdamage = 1;
  self->health = self->healthmax = monster->health;
  self->stamina = self->staminamax = monster->stamina;
  self->mana = self->manamax = monster->mana;
  self->accel = 0.25;         /*to be adjusted*/
  self->speed = 5 * monster->movespeed;            /*to be calculated*/
  self->attackspeed = monster->attackspeed;
  self->guardstrength = monster->guardstrength;
  self->guardrange = monster->guardrange;
  if(self->attackspeed == 0)self->attackspeed = 1;
  SetMonsterCombatStats(self,monster,0);
  self->state = MS_Idle;
}

void SetMonsterCombatStats(Entity *self,Monster *monster,int action)
{
  if((action < 0)||(action >= 2))return;
  self->damagetype = D_Physical;
  self->attackrange = monster->range[action];
  self->monsterdamage = monster->damage[action];
  if(strcmp(monster->actions[action],"slash")==0)
  {
    self->attacktype = AT_Slash;
    self->damagetype |= D_Slash;
  }
  else if(strcmp(monster->actions[action],"stab")==0)
  {
    self->attacktype = AT_Stab;
    self->damagetype |= D_Pierce;
  }
  else if(strcmp(monster->actions[action],"bash")==0)
  {
    self->attacktype = AT_Bash;
    self->damagetype |= D_Crush;
  }
  else if(strcmp(monster->actions[action],"shoot")==0)
  {
    self->attacktype = AT_Shoot;
    self->damagetype |= D_Pierce;
  }
}

Monster *GetMonsterByName(char name[80])
{
  int i;
  for(i = 0;i < NumMonsters;i++)
  {
    if(strcmp(name,MonsterList[i].name)==0)
      return &MonsterList[i];
  }
  return NULL;
}

void LoadMonsterList()
{
  int i = 0;
  int j = 0;
  int k = 0;
  FILE *file;
  char *c;
  char buf[512];
  file = fopen("system/monsterlist.def","r");
  if(file == NULL)
  {
    fprintf(stderr,"Unable to open race file\n");
    return;
  }
  memset(&MonsterList,0,sizeof(MonsterList));
  i = 0;
  j = 0;
  k = 0;
  NumMonsters = 0;
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
      NumMonsters++;
      j = k = 0;
      if(i >= MONSTERMAX)
      {
        fprintf(stderr,"maximum number of monsterss loaded from file!\n");
        return;
      }
      continue;
    }
    if(strcmp(buf,"<monster>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(MonsterList[i].name, 80, file);
      c = strchr(MonsterList[i].name, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<sprite>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(MonsterList[i].sprite, 80, file);
      c = strchr(MonsterList[i].sprite, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<spritesize>") ==0)
    {
      fscanf(file, "%i %i",&MonsterList[i].sx,&MonsterList[i].sy);
      continue;
    }
    if(strcmp(buf,"<nearactions>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(MonsterList[i].nearbehavior, 80, file);
      c = strchr(MonsterList[i].nearbehavior, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<faractions>") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(MonsterList[i].farbehavior, 80, file);
      c = strchr(MonsterList[i].farbehavior, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        continue;
    }
    if(strcmp(buf,"<scale>") ==0)
    {
      fscanf(file, "%f",&MonsterList[i].scale);
      continue;
    }
    if(strcmp(buf,"<radius>") ==0)
    {
      fscanf(file, "%f",&MonsterList[i].radius);
      continue;
    }
    if(strcmp(buf,"<grace>") ==0)
    {
      fscanf(file, "%i",&MonsterList[i].grace);
      continue;
    }
    if(strcmp(buf,"<gold>") ==0)
    {
      fscanf(file, "%i",&MonsterList[i].gold);
      continue;
    }
    if(strcmp(buf,"<goldchance>") ==0)
    {
      fscanf(file, "%i",&MonsterList[i].goldchance);
      continue;
    }
    if(strcmp(buf,"<tendency>") ==0)
    {
      fscanf(file, "%i",&MonsterList[i].tend);
      continue;
    }
    if(strcmp(buf,"<alignment>") ==0)
    {
      fscanf(file, "%i",&MonsterList[i].align);
      continue;
    }
    if(strcmp(buf,"<guardrange>") ==0)
    {
      fscanf(file, "%i",&MonsterList[i].guardrange);
      continue;
    }
    if(strcmp(buf,"<droprate>") ==0)
    {
      fscanf(file, "%f",&MonsterList[i].droprate);
      continue;
    }
    if(strcmp(buf,"<guardstrength>") ==0)
    {
      fscanf(file, "%f",&MonsterList[i].guardstrength);
      continue;
    }
    if(strcmp(buf,"<hsm>") ==0)
    {
      fscanf(file, "%i %i %i",&MonsterList[i].health,&MonsterList[i].stamina,&MonsterList[i].mana);
      continue;
    }
    if(strcmp(buf,"<sightrange>") ==0)
    {
      fscanf(file, "%i",&MonsterList[i].sightrange);
      continue;
    }
    if(strcmp(buf,"<sightdist>") ==0)
    {
      fscanf(file, "%i",&MonsterList[i].sightdistance);
      continue;
    }
    if(strcmp(buf,"<darksight>") ==0)
    {
      fscanf(file, "%f",&MonsterList[i].darkmod);
      continue;
    }
    if(strcmp(buf,"<attackspeed>") ==0)
    {
      fscanf(file, "%f",&MonsterList[i].attackspeed);
      continue;
    }
    if(strcmp(buf,"<movespeed>") ==0)
    {
      fscanf(file, "%f",&MonsterList[i].movespeed);
      continue;
    }
    if(strcmp(buf,"<actions>") ==0)
    {
      if(j < 2)
      {
        fscanf(file, "%s %i %i",MonsterList[i].actions[j], &MonsterList[i].damage[j], &MonsterList[i].range[j]);
        j++;
      }
      else
      {
        fgets(buf, sizeof(buf), file);
      }
      continue;
    }
    if(strcmp(buf,"<drop>") ==0)
    {
      if(k < 4)
      {
        fgetc(file);  /*clear the space before the word*/
        fgets(MonsterList[i].drops[k], 80, file);
        c = strchr(MonsterList[i].drops[k], '\n');
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

/*eof@eol*/

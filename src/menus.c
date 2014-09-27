#include "menus.h"
#include "graphics.h"
#include "glmodel.h"
#include "modelLoader.h"
#include "audio.h"
#include "particle.h"
#include "entity.h"
#include "levels.h"
#include "player.h"
#include "enemies.h"
#include "weapons.h"
#include "items.h"
#include "monsters.h"
#include "npcs.h"
#include "region.h"
#include "controls.h"

extern Mouse_T Mouse;
extern ScreenData  S_Data;
extern SDL_Rect Camera;
extern int FontSize[4];
extern Sprite *hudsprites[];
extern GLint viewport[4];
extern GLdouble modelview[16];
extern GLdouble projection[16];
extern Uint32 NOW;
extern glCamera_T glCamera;
extern int BUTTON_W;
extern int BUTTON_H;
extern float ScreenScaleFactor;
extern PlayerSave PDat;
extern Level level;
extern Entity *PlayerEnt;
extern Sprite *blackout;

Sprite *ingamemenu[10];
int Pausing = 0;
char text[80];
char newbg[80];
char newts[80];
char newname[80];
char ChosenRace[80];
char ChosenTraining[80];
char ChosenName[80] = "unknown";
int Rx = -1,Ry = -1;
int fill;
char LevelFilename[80];
int selectedtile = 0;     /*which tile to place*/
int selectedobj = 0;      /*which object to place*/
Object *selectedObject = NULL;  /*selected object for editing*/
int MarkedForDoom;
int WindowForDoom = -1;
void UpdateEditorCamera();
void DrawTilePanel(HUDInfo *self);


inline void Pause()
{
  Pausing++;
}

inline void UnPause()
{
  Pausing--;
}

inline int  IsPausing()
{
  return Pausing;
}


void LoadAssets()
{
/*  int i;
  char text[80];
  */
  if(blackout == NULL)blackout = LoadSprite("images/masks/unseen.png",48,48);
  LoadMonsterList();
  LoadSpellList();
  LoadSkillList();
  LoadTrainingList();
  LoadRaceList();
  LoadItemList();
  LoadNPCList();
  ingamemenu[0] = LoadSprite("images/UI/hud.png",-1,-1);
}
/*

  In Game Windows

*/
void SelectClassWindow();
void GameWindow(char level[80],int newgame);
void SelectRaceWindow();

void SetupTutorial()
{
  NewPlayer("Atlantean","Tutorial");
  DrawSplashScreen();
  GameWindow("levels/tutorial.lvl",1);
  ClearKeyboard();
}

void DrawPlayerStatsWindow(HUDInfo *self)
{
  char txt[80];
  DrawImageWindow(self->box.x,self->box.y,self->box.w,self->box.h);
  DrawTextCentered(ChosenName,self->box.x + Camera.w/2 + 2,16 + 2,IndexColor(DarkGrey),F_Large);
  DrawTextCentered(ChosenName,self->box.x + Camera.w/2,16,IndexColor(White),F_Large);
  sprintf(txt,"%s %s",PDat.race,PDat.train);
  DrawTextCentered(txt,self->box.x + Camera.w/2 + 2,16 + 2 + FontSize[F_Large], IndexColor(DarkGrey), F_Medium);
  DrawTextCentered(txt,self->box.x + Camera.w/2,16 + FontSize[F_Large],IndexColor(White),F_Medium);
  /*player attributes*/
  DrawPlayerState(self->box.x + (Camera.w * 0.02),self->box.y + (self->box.h * 0.1));
  /*player skills*/
  DrawPlayerSpells(self->box.x + (Camera.w * 0.45), self->box.y + (self->box.h * 0.15));
  /*player spells*/
  DrawPlayerSkills(self->box.x + (Camera.w * 0.65), self->box.y + (self->box.h * 0.15));
  /*Player Equipment*/
  DrawPlayerEquipment(self->box.x + (Camera.w * 0.22),self->box.y + (self->box.h * 0.1));
  DrawPlayerInventory(self->box.x + (Camera.w * 0.45),self->box.y + (self->box.h * 0.3), 0, 15);
  DrawPlayerResistance(self->box.x + (Camera.w * 0.65),self->box.y + (self->box.h * 0.3));
}

void DrawPlayerPreviewWindow(HUDInfo *self)
{
  DrawPlayerStatsWindow(self);
}

void ChangePlayerName()
{
  strcpy(ChosenName,text);
}

int UpdatePlayerPreviewWindow(HUDInfo *self,int pressID)
{
  switch(pressID)
  {
    case 0:
    PopWindow(self->Handle);
    DrawSplashScreen();
    GameWindow("levels/starttown.lvl",1);
    ClearKeyboard();
    return 1;
    case 1:
      strcpy(text,ChosenName);
      GetText("Enter New Name",text,80,ChangePlayerName,NULL);
      break;
    case 2:
      PopWindow(self->Handle);
      SelectRaceWindow();
      return 1;
  }
  return 0;
}

void PlayerPreviewWindow()
{
  HUDInfo *self;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stdout,"Big problem here, can't make starting game window.\n");
    return;
  }
  self->windowupdate = UpdatePlayerPreviewWindow;
  self->windowdraw = DrawPlayerPreviewWindow;
  self->box.x = 0;
  self->box.y = 0;
  self->box.w = Camera.w;
  self->box.h = Camera.h;
  PicButton(self,0,SDLK_RETURN,"Done",self->box.x + self->box.w  - (BUTTON_W + 10),self->box.y + BUTTON_H + 4,BUTTON_W,BUTTON_H);
  PicButton(self,2,SDLK_ESCAPE,"Back",self->box.x + self->box.w  - (BUTTON_W + 10),self->box.y + ((BUTTON_H + 4) * 2),BUTTON_W,BUTTON_H);
  PicButton(self,1,SDLK_n,"Change Name",self->box.x + 20,self->box.y + 10,BUTTON_W,BUTTON_H);
  NewPlayer(ChosenRace,ChosenTraining);
}


void DrawRaceWindow(HUDInfo *self)
{
  int i;
  char txt[80];
  P_Race *race;
  DrawImageWindow(self->box.x,self->box.y,self->box.w,self->box.h);
  DrawTextCentered("Choose Your Character's Race",self->box.x + Camera.w/2 + 2,16 + 2,IndexColor(DarkGrey),F_Large);
  DrawTextCentered("Choose Your Character's Race",self->box.x + Camera.w/2,16,IndexColor(White),F_Large);
  if(self->state != -1)
  {
    race = GetRaceByIndex(self->state);
    if(race != NULL)
    {
      sprintf(txt,"Name: %s",race->name);
      DrawTextCentered(txt,self->box.x + Camera.w/2 + 1,self->box.y + BUTTON_H * 2 + 1, IndexColor(DarkGrey), F_Medium);
      DrawTextCentered(txt,self->box.x + Camera.w/2,self->box.y + BUTTON_H * 2,IndexColor(White), F_Medium);
      for(i = 0;i < 6;i++)
      {
        sprintf(txt,"%s:  %i",GetAttributeName(i),race->attributes[i]);
        DrawTxt(txt,self->box.x + (Camera.w * 0.25),self->box.y + (BUTTON_H * 3) + ((FontSize[F_Small] + 4) * i), IndexColor(White), F_Small);
      }
    }
  }
}

int UpdateRaceWindow(HUDInfo *self,int pressID)
{
  P_Race *race;
  if(pressID == 0)
  {
    if(self->state == -1)
    {
      NewMessage("Pick a Race First!",IndexColor(White));
    }
    else
    {
      race = GetRaceByIndex(self->state);
      if(race != NULL)strcpy(ChosenRace,race->name);
      PopWindow(self->Handle);
      SelectClassWindow();
      return 1;
    }
  }
  if((pressID >= 1)&&(pressID <= GetRaceCount()))
  {
    self->state = pressID - 1;
  }
  return 0;
}

void SelectRaceWindow()
{
  HUDInfo *self;
  P_Race *race;
  int count,i;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stdout,"Big problem here, can't make starting game window.\n");
    return;
  }
  self->windowupdate = UpdateRaceWindow;
  self->windowdraw = DrawRaceWindow;
  self->box.x = 0;
  self->box.y = 0;
  self->box.w = Camera.w;
  self->box.h = Camera.h;
  count = GetRaceCount();
  PicButton(self,0,SDLK_RETURN,"Done",self->box.x + self->box.w/2  - BUTTON_W/2,self->box.y + BUTTON_H + 4,BUTTON_W,BUTTON_H);
  self->state = -1;
  for(i = 0;i < count;i++)
  {
    race = GetRaceByIndex(i);
    if(race != NULL)
    {
      PicButton(self,1 + i,SDLK_1 + i,race->name,self->box.x + (self->box.w * 0.05),self->box.y + (BUTTON_H *2) + ((BUTTON_H + 4) * i),BUTTON_W,BUTTON_H);
    }
  }
}

void DrawClassWindow(HUDInfo *self)
{
  int i;
  P_Train *train;
  DrawImageWindow(self->box.x,self->box.y,self->box.w,self->box.h);
  DrawTextCentered("Choose Your Character's Training Class",self->box.x + Camera.w/2 + 2,16 + 2,IndexColor(DarkGrey),F_Large);
  DrawTextCentered("Choose Your Character's Training Class",self->box.x + Camera.w/2,16,IndexColor(White),F_Large);
  if(self->state != -1)
  {
    train = GetTrainByIndex(self->state);
    if(train != NULL)
    {
      sprintf(text,"Name: %s",train->name);
      DrawTextCentered(text,self->box.x + Camera.w/2 + 1,self->box.y + BUTTON_H * 2 + 1, IndexColor(DarkGrey), F_Medium);
      DrawTextCentered(text,self->box.x + Camera.w/2,self->box.y + BUTTON_H * 2,IndexColor(White), F_Medium);
      for(i = 0;i < 6;i++)
      {
        sprintf(text,"%s:  %i",GetAttributeName(i),train->attributes[i]);
        DrawTxt(text,self->box.x + (Camera.w * 0.25),self->box.y + (BUTTON_H * 3) + ((FontSize[F_Small] + 4) * i), IndexColor(White), F_Small);
      }
    }
  }
}

int UpdateClassWindow(HUDInfo *self,int pressID)
{
  P_Train *train;
  if(pressID == 0)
  {
    if(self->state == -1)
    {
      NewMessage("Please Pick a Training Class!",IndexColor(White));
      return 0;
    }
    else
    {
      train = GetTrainByIndex(self->state);
      if(train != NULL)strcpy(ChosenTraining,train->name);
      PopWindow(self->Handle);
      PlayerPreviewWindow();
    }
  }
  if((pressID >= 1)&&(pressID <= GetTrainingCount()))
  {
    self->state = pressID - 1;
  }
  return 0;
}

void SelectClassWindow()
{
  HUDInfo *self;
  P_Train *train;
  int count,i;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stdout,"Big problem here, can't make starting game window.\n");
    return;
  }
  self->windowupdate = UpdateClassWindow;
  self->windowdraw = DrawClassWindow;
  self->box.x = 0;
  self->box.y = 0;
  self->box.w = Camera.w;
  self->box.h = Camera.h;
  count = GetTrainingCount();
  PicButton(self,0,SDLK_RETURN,"Done",self->box.x + self->box.w/2  - BUTTON_W/2,self->box.y + BUTTON_H + 4,BUTTON_W,BUTTON_H);
  self->state = -1;
  for(i = 0;i < count;i++)
  {
    train = GetTrainByIndex(i);
    if(train != NULL)
    {
      PicButton(self,1 + i,SDLK_1 + i,train->name,self->box.x + (self->box.w * 0.05),self->box.y + (BUTTON_H *2) + ((BUTTON_H + 4) * i),BUTTON_W,BUTTON_H);
    }
  }
}


/*

      Game Window

*/

void DrawGameWindow(HUDInfo *self)
{
  Sprite *sprite;
  Item *item;
  P_Skill *spell;
  char txt[80];
  int barx,barw;
  DrawLevel(0);
  DrawSprite(ingamemenu[0],0,0,0,1);
  barx = 85;
  barw = 140;
  DrawGradientRect(barx,2, barw, 10, IndexColor(Black), IndexColor(Black), IndexColor(DarkRed), IndexColor(DarkRed),1);
  DrawGradientRect(barx,15, barw, 10, IndexColor(Black), IndexColor(Black), IndexColor(DarkGreen), IndexColor(DarkGreen),1);
  DrawGradientRect(barx,28, barw, 10, IndexColor(Black), IndexColor(Black), IndexColor(DarkBlue), IndexColor(DarkBlue),1);
  if(PlayerEnt != NULL)
  {
    if(PlayerEnt->healthmax > 0)
    {
      DrawGradientRect(barx,2, barw * (PlayerEnt->health / (float)PlayerEnt->healthmax), 10, IndexColor(LightRed), IndexColor(LightRed), IndexColor(Red), IndexColor(Red),0.9);
    }
    if(PlayerEnt->staminamax > 0)
    {
      DrawGradientRect(barx,15, barw * (PlayerEnt->stamina / (float)PlayerEnt->staminamax), 10, IndexColor(LightGreen), IndexColor(LightGreen), IndexColor(Green), IndexColor(Green),0.9);
    }
    if(PlayerEnt->manamax > 0)
    {
      DrawGradientRect(barx,28, barw * (PlayerEnt->mana / (float)PlayerEnt->manamax), 10, IndexColor(LightBlue), IndexColor(LightBlue), IndexColor(Blue), IndexColor(Blue),0.9);
    }
  }
  sprintf(txt,"%i Gold",PDat.gold);
  DrawTxtShadow(txt,780,2,IndexColor(Gold),F_Small,1);
  sprintf(txt,"%i Grace",PDat.grace);
  DrawTxtShadow(txt,780,2 + GetFontHeight(F_Small),IndexColor(Silver),F_Small,1);
  if(PDat.chosenspell != -1)
  {
    spell = GetSpellByName(PDat.spells[PDat.chosenspell]);
    if(spell != NULL)
    {
      DrawTxtShadow(spell->name,80,50,IndexColor(Gold),F_Small,1);
      if(strlen(spell->icon) > 1)
      {
        sprite = LoadSprite(spell->icon,-1,-1);
        if(sprite)DrawSpriteStretchRot( sprite, 24, 52, 0,1,1, 0, 1);
/*        DrawSprite(sprite,12,8,0,1);*/
        FreeSprite(sprite);
        sprite = NULL;
      }
    }
  }
  if(PDat.equips[E_Tool] != -1)
  {
    sprintf(txt,"(%i) %ss",PDat.inventory[PDat.equips[E_Tool]].count, PDat.inventory[PDat.equips[E_Tool]].name);
    DrawTxtShadow(txt,780,50,IndexColor(Gold),F_Small,1);
    item = GetItemByName(PDat.inventory[PDat.equips[E_Tool]].name);
    if(item != NULL)
    {
      if(strlen(item->icon) > 1)
      {
        sprite = LoadSprite(item->icon,-1,-1);
        if(sprite)DrawSpriteStretchRot( sprite, 1000, 52, 0,1,1, 0, 1);
        FreeSprite(sprite);
        sprite = NULL;
      }
    }
  }
  if(PDat.equips[E_Primary1] != -1)
  {
    item = GetItemByName(PDat.inventory[PDat.equips[E_Primary1]].name);
    if(item != NULL)
    {
      if(strlen(item->icon) > 1)
      {
        sprite = LoadSprite(item->icon,-1,-1);
        if(sprite)DrawSpriteStretchRot( sprite, 976, 22, 0,1,1, 0, 1);
        FreeSprite(sprite);
        sprite = NULL;
      }
    }
  }
  if(PDat.equips[E_Secondary1] != -1)
  {
    item = GetItemByName(PDat.inventory[PDat.equips[E_Secondary1]].name);
    if(item != NULL)
    {
      if(strlen(item->icon) > 1)
      {
        sprite = LoadSprite(item->icon,-1,-1);
        if(sprite)DrawSpriteStretchRot( sprite, 47, 22, 0,1,1, 0, 1);
        FreeSprite(sprite);
        sprite = NULL;
      }
    }
  }
}

int UpdateGameWindow(HUDInfo *self,int pressID)
{
  updateAllEntities();
  UpdatePlayerCamera();
  return 0;
}

void GameWindow(char level[80],int newgame)
{
  HUDInfo *self;
  Object *playerstart;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stdout,"Big problem here, can't make starting game window.\n");
    return;
  }
  self->windowupdate = UpdateGameWindow;
  self->windowdraw = DrawGameWindow;
  self->box.x = 0;
  self->box.y = 0;
  self->box.w = Camera.w;
  self->box.h = Camera.h;
  HideMouse();
  LoadLevel(level);
  BuildLattice();
  if(newgame)
  {
    playerstart = GetObjectByName("playerstart");
    if(playerstart == NULL)
    {
      SetupPlayer(48,48);
    }
    else
    {
      SetupPlayer((playerstart->x * TILEW) + TILEW/2,(playerstart->y * TILEW) + TILEW/2);
    }
    NewMsg("Press F1 for Controls");
  }
  else
  {
    SetupPlayer(PDat.x,PDat.y);
  }
  SpawnLevelEnts();
  MoveMessages(244,0,4);
  return;
}

void DrawTextBlockWindow(HUDInfo *self)
{
  char *text;
  DrawImageWindow(self->box.x,self->box.y,self->box.w,self->box.h);
  text = (char *)self->ib;
  DrawTextBlock(text,self->box.x + (self->box.w *0.05) + 1, self->box.y + (self->box.w *0.05) + 1,IndexColor(DarkGrey),F_Small,self->box.w *0.9);
  DrawTextBlock(text,self->box.x + (self->box.w *0.05), self->box.y + (self->box.w *0.05),IndexColor(Silver),F_Small,self->box.w *0.9);
}

int UpdateTextBlockWindow(HUDInfo *self,int pressID)
{
  switch(pressID)
  {
    case 0:
      UnPause();
      HideMouse();
      PopWindow(self->Handle);
      return 1;
      break;
  }
  return 0;
}

void TextBlockWindow(char *text)
{
  HUDInfo *self;
  if(text == NULL)return;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stdout,"Big problem here, can't make text window window.\n");
    return;
  }
  self->windowupdate = UpdateTextBlockWindow;
  self->windowdraw = DrawTextBlockWindow;
  self->box.y = Camera.w *0.15;
  self->box.w = Camera.w * 0.5;
  self->box.x = Camera.w/2 - self->box.w/2;
  self->box.h = GetTextBlockHeight(text,F_Small,self->box.w*0.8) + (self->box.w * 0.1) + BUTTON_H + 8;
  self->ib = malloc(strlen(text) + 1);
  memcpy(self->ib,text,strlen(text) + 1);
  ShowMouse();
  Pause();
  PicButton(self,0,SDLK_RETURN,"Done",self->box.x + self->box.w/2  - (BUTTON_W/2),self->box.y + self->box.h - BUTTON_H - 4,BUTTON_W,BUTTON_H);
  return;
}


/*

      Map Editor Window

*/


/*
  sub windows of the editor
*/

/*
    Level Info Panel
*/
void NewHeight()
{
  sscanf(text,"%i",&Ry);
}

void NewWidth()
{
  sscanf(text,"%i",&Rx);
  sprintf(text,"%i",Ry);
  GetText("Enter New Height",text,4,NewHeight,NULL);
}

void NewBackground()
{
  strcpy(newbg,text);
}

void NewMapName()
{
  strcpy(newname,text);
}

void NewTileset()
{
  strcpy(newts,text);
}

void BuildNewLevel()
{
  NewLevel(newname,Rx, Ry, newbg,newts, fill, selectedtile);
}

void ApplyInfo()
{
  if(strcmp(level.name,newname) != 0)
  {
    strcpy(level.name,newname);
  }
  if(strcmp(level.background,newbg) != 0)
  {
    strcpy(level.background,newbg);
    LoadLevelBackground();
  }
  if(strcmp(level.tileset,newts) != 0)
  {
    strcpy(level.tileset,newts);
    LoadTileSet(level.tileset);
  }
}

int UpdateInfoPanel(HUDInfo *self,int pressID)
{
  switch(pressID)
  {
    case 1:
      YesNo("Create a New Level?",BuildNewLevel,NULL);
      break;
    case 2:
      strcpy(text,newbg);
      GetText("Enter New Background",text,80,NewBackground,NULL);
      break;
    case 3:
      strcpy(text,newts);
      GetText("Enter New Tile Set",text,80,NewTileset,NULL);
      break;
    case 4:
      sprintf(text,"%i",Rx);
      GetText("Enter New Width",text,4,NewWidth,NULL);
      break;
    case 5:
    case 6:
    case 7:
    case 8:
      fill = pressID - 5;
      break;
    case 9:
      strcpy(text,newname);
      GetText("Enter New Map Name",text,80,NewMapName,NULL);
      break;
    case 10:
      YesNo("Apply Level Info (not size)?",ApplyInfo,NULL);
      break;
  }
  return 0;
}

void DrawInfoPanel(HUDInfo *self)
{
  char txt[80];
  DrawTileToScreen(selectedtile,self->box.x + self->box.w - 150,self->box.y + self->box.h/2 - TILEH/2);
  sprintf(txt,"Level Size: %i, %i",Rx,Ry);
  DrawTxt(txt,self->box.x + self->box.w/2,40,IndexColor(White),F_Small);
  DrawTxt(newbg,self->box.x + self->box.w/2,14,IndexColor(White),F_Small);
  DrawTxt(newts,self->box.x + self->box.w/2,64,IndexColor(White),F_Small);
  DrawTxt(newname,self->box.x + 74 + BUTTON_W,24,IndexColor(White),F_Small);
  switch(fill)
  {
    case 0:
      DrawTxt("Bordered",self->box.x + self->box.w/2,88,IndexColor(White),F_Small);
      break;
    case 1:
      DrawTxt("Filled",self->box.x + self->box.w/2,88,IndexColor(White),F_Small);
      break;
    case 2:
      DrawTxt("Open",self->box.x + self->box.w/2,88,IndexColor(White),F_Small);
      break;
    case 3:
      DrawTxt("Random",self->box.x + self->box.w/2,88,IndexColor(White),F_Small);
      break;
  }
}

HUDInfo *InfoPanel()
{
  HUDInfo *self;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stdout,"Big problem here, can't make starting game window.\n");
    return NULL;
  }
  self->stealinput = 0;
  self->windowupdate = UpdateInfoPanel;
  self->windowdraw = DrawInfoPanel;
  self->box.x = 80;
  self->box.y = 0;
  self->box.w = Camera.w - 80;
  self->box.h = 100;
  TextButton(self,1,SDLK_F11,"Rebuild",self->box.x + self->box.w - 84,88,64,10);
  TextButton(self,2,SDLK_s,"Background",self->box.x + self->box.w - 84,14,64,10);
  TextButton(self,3,SDLK_t,"Tile Set",self->box.x + self->box.w - 84,64,64,10);
  TextButton(self,4,SDLK_F10,"Resize",self->box.x + self->box.w - 84,40,64,10);
  TextButton(self,5,SDLK_EQUALS,"Border",self->box.x + 74,40,64,10);
  TextButton(self,6,SDLK_PLUS,"Fill",self->box.x + 74,64,64,10);
  TextButton(self,7,SDLK_MINUS,"Empty",self->box.x + 74,88,64,10);
  TextButton(self,8,SDLK_r,"Random",self->box.x + 74,14,64,10);
  TextButton(self,9,SDLK_n,"Map Name",self->box.x + 74 + BUTTON_W,14,64,10);
  TextButton(self,10,SDLK_F12,"Apply Info",self->box.x + 74 + BUTTON_W,64,64,10);
    Rx = level.w;
    Ry = level.h;
    strcpy(newbg,level.background);
    strcpy(newts,level.tileset);
    strcpy(newname,level.name);
  return self;
}


/*  Object Keys  */

void DrawObjectKeys(HUDInfo *self)
{
  int i;
  DrawImageWindow(self->box.x, self->box.y, self->box.w, self->box.h);
  if(selectedObject != NULL)
  {
    for(i = 0; i < selectedObject->numkeys;i++)
    {
      DrawTxt(selectedObject->data[i],self->box.x + 32,self->box.y + ((FontSize[F_Small] + 2) * i),IndexColor(White),F_Small);
    }
    
  }
}

void AddKeyText()
{
  if(selectedObject != NULL)
    AddDataKeyToObject(selectedObject,text);
}

void KillKey()
{
  RemoveKeyFromObject(selectedObject,MarkedForDoom);
}

int UpdateObjectKeys(HUDInfo *self,int pressID)
{
  switch(pressID)
  {
    case 0:
      self->Parent->Child = NULL;
      PopWindow(self->Handle);
      return 1;
      break;
    case 1:
      memset(text,0,sizeof(text));
      GetText("Enter new Key",text,80,AddKeyText,NULL);
      break;
  }
  if(pressID >= 2)
  {
    if(selectedObject != NULL)
    {
      if(pressID - 2 < selectedObject->numkeys)
      {
        MarkedForDoom = pressID - 2;
        YesNo("Remove Key?",KillKey,NULL);
      }
    }
  }
  return 0;
}

HUDInfo *ObjectKeysWindow(HUDInfo *parent)
{
  int i;
  HUDInfo *self;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stdout,"Big problem here, can't make starting game window.\n");
    return NULL;
  }
  self->Parent = parent;
  self->windowupdate = UpdateObjectKeys;
  self->windowdraw = DrawObjectKeys;
  self->stealinput = 1;   /*don't let anything happen until this window is done.*/
  self->box.y = 128;
  self->box.w = FontSize[F_Small] * 50;
  self->box.x = Camera.w/2 - self->box.w/2;
  self->box.h = (FontSize[F_Small] + 4) * 16 + BUTTON_H + 8;
  PicButton(self,1,SDLK_n,"New Key",self->box.x + self->box.w/2 - BUTTON_W - 4,self->box.y + self->box.h - BUTTON_H - 4,BUTTON_W,BUTTON_H);
  PicButton(self,0,SDLK_RETURN,"Done",self->box.x + self->box.w/2 + 4,self->box.y + self->box.h - BUTTON_H - 4,BUTTON_W,BUTTON_H);
  for(i = 0;i < MAXOBJKEYS;i++)
  {
    TextButton(self,2 + i,SDLK_MINUS," ",self->box.x + 32,self->box.y + ((FontSize[F_Small] + 2) * i),self->box.w - 36,FontSize[F_Small]);
  }
  return self;
}

/*
  File Panel
*/

void DrawFilePanel(HUDInfo *self)
{
  DrawTextCentered(LevelFilename,Camera.w/2 + 1,16 + 1,IndexColor(DarkGrey),F_Medium);
  DrawTextCentered(LevelFilename,Camera.w/2,16,IndexColor(Silver),F_Medium);
}

void SaveFileAs()
{
  char filename[80];
  sprintf(filename,"levels/%s",LevelFilename);
  SaveLevel(filename);
}

void LoadFileByName()
{
  char filename[80];
  sprintf(filename,"levels/%s",LevelFilename);
  LoadLevel(filename);
}


int UpdateFilePanel(HUDInfo *self,int pressID)
{
  switch(pressID)
  {
    case 0:
      strcpy(LevelFilename,level.name);
      GetText("Save File As...",LevelFilename,80,SaveFileAs,NULL);
      break;
    case 1:
      GetText("File to Load...",LevelFilename,80,LoadFileByName,NULL);
      break;
  }
  return 0;
}

HUDInfo *FilePanel()
{
  HUDInfo *self;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stdout,"Big problem here, can't make starting game window.\n");
    return NULL;
  }
  self->stealinput = 0;
  self->windowupdate = UpdateFilePanel;
  self->windowdraw = DrawFilePanel;
  self->box.x = 80;
  self->box.y = 0;
  self->box.w = Camera.w - 80;
  self->box.h = 100;
  PicButton(self,0,SDLK_F2,"Save",Camera.w/2 - 170,64,165,58);
  PicButton(self,1,SDLK_F3,"Load",Camera.w/2 + 5,64,165,58);
  return self;
}


void DrawTilePanel(HUDInfo *self)
{
  DrawTileToScreen(selectedtile,self->box.x + 86 - TILEW/2,self->box.y + self->box.h/2 - TILEH/2);
}

int UpdateTilePanel(HUDInfo *self,int pressID)
{
  int numtiles;
  int x,y;
  numtiles = GetNumberOfTiles();
  switch(pressID)
  {
    case 0:
      selectedtile--;
      if(selectedtile < 0)selectedtile = numtiles - 1;
      break;
    case 1:
      selectedtile++;
      if(selectedtile >= numtiles)selectedtile = 0;
      break;
  }
  if(selectedtile >= numtiles)selectedtile = 0;
  if(MouseHeld(3))/*right click*/
  {
    if(Mouse.y > 116)
    {
      ConvertToTileXY(Mouse.x,Mouse.y,&x,&y);
      ClearLevelTile(x,y);
    }
  }
  if(MouseHeld(1))
  {
    if(Mouse.y > 116)
    {
      ConvertToTileXY(Mouse.x,Mouse.y,&x,&y);
      SetLevelTile(x,y,selectedtile);
    }
  }
  return 0;
}

HUDInfo *TilePanel()
{
  HUDInfo *self;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stdout,"Big problem here, can't make starting game window.\n");
    return NULL;
  }
  self->stealinput = 0;
  self->windowupdate = UpdateTilePanel;
  self->windowdraw = DrawTilePanel;
  self->box.x = 80;
  self->box.y = 0;
  self->box.w = Camera.w - 80;
  self->box.h = 100;
  TextButton(self,0,SDLK_p,"Prev",self->box.x + 24,88,64,10);
  TextButton(self,1,SDLK_n,"Next",self->box.x + 90,88,64,10);
  return self;
}

/*map objects*/
void DrawObjPanel(HUDInfo *self)
{
  int obj;
  char txt[80];
  DrawGradientRect(self->box.x + 86 - TILEW,self->box.y + self->box.h/2 - TILEH, TILEW*2, TILEH*2, IndexColor(DarkGrey), IndexColor(DarkGrey), IndexColor(LightGrey), IndexColor(LightGrey),1);
  DrawObjToScreen(selectedobj,self->box.x + 86 - TILEW/2,self->box.y + self->box.h/2 - TILEH/2,1);
  if(self->state == 1)
  {
    DrawTextCentered("Select Mode",self->box.x + self->box.w/2,self->box.y + self->box.h +16,IndexColor(DarkGrey),F_Small);
    DrawTextCentered("Select Mode",self->box.x + self->box.w/2 - 1,self->box.y + self->box.h + 16 - 1,IndexColor(LightBlue),F_Small);
  }
  if(self->state == 0)
  {
    DrawTextCentered("Add Object Mode",self->box.x + self->box.w/2,self->box.y + self->box.h +16,IndexColor(DarkGrey),F_Small);
    DrawTextCentered("Add Object Mode",self->box.x + self->box.w/2 - 1,self->box.y + self->box.h + 16 - 1,IndexColor(LightGreen),F_Small);
  }
  if(selectedObject != NULL)
  {
    obj = GetObjTypeIndexByName(selectedObject->name);
    if(obj != -1)
    {
      DrawGradientRect(self->box.x + 200 - TILEW,self->box.y + self->box.h/2 - TILEH, TILEW*2, TILEH*2, IndexColor(DarkGrey), IndexColor(DarkGrey), IndexColor(LightGrey), IndexColor(LightGrey),1);
      DrawObjToScreen(obj,self->box.x + 200 - TILEW/2,self->box.y + self->box.h/2 - TILEH/2,1);
      DrawTextCentered(selectedObject->name,self->box.x + self->box.w/2,self->box.y +10,IndexColor(DarkGrey),F_Small);
      DrawTextCentered(selectedObject->name,self->box.x + self->box.w/2 - 1,self->box.y + 10  - 1 ,IndexColor(Silver),F_Small);
      sprintf(txt,"Object ID: %i",selectedObject->id);
      DrawTextCentered(txt,self->box.x + self->box.w/2 - 1,self->box.y + 10  + FontSize[F_Small] ,IndexColor(Gold),F_Small);
    }
  }
}

int UpdateObjPanel(HUDInfo *self,int pressID)
{
  int numtiles;
  int x,y;
  numtiles = GetNumberOfObjTypes();
  switch(pressID)
  {
    case 0:
      selectedobj--;
      if(selectedobj < 0)selectedobj  = numtiles - 1;
      break;
    case 1:
      selectedobj++;
      if(selectedobj >= numtiles)selectedobj = 0;
      break;
    case 2:
      self->state = 0;
/*      selectedObject = NULL;*/
      break;
    case 3:
      self->state = 1;
      break;
    case 4:
      self->Child = ObjectKeysWindow(self);
      break;
  }
  if(selectedobj >= numtiles)selectedobj = 0;
  
  if(MouseReleased(3))
  {
    if(Mouse.y > 116)
    {
      if(self->state == 0)
      {
        ConvertToTileXY(Mouse.x,Mouse.y,&x,&y);
        RemoveObjectFromLevel(x,y);
      }
    }
  }
  if(MouseReleased(2))
  {
    if(Mouse.y > 116)
    {
      if(self->state == 0)
      {
        if(selectedObject != NULL)
        ConvertToTileXY(Mouse.x,Mouse.y,&x,&y);
        CloneObjectAt(selectedObject,x,y);
      }
    }
  }
  if(MouseReleased(1))
  {
    if(Mouse.y > 116)
    {
      if(self->state == 0)
      {
        ConvertToTileXY(Mouse.x,Mouse.y,&x,&y);
        AddObjectToLevel(selectedobj,x,y);
      }
      else if(self->state == 1)
      {
        ConvertToTileXY(Mouse.x,Mouse.y,&x,&y);
        selectedObject = GetObjectByXY(x,y);
      }
    }
  }
  return 0;
}

HUDInfo *ObjPanel()
{
  HUDInfo *self;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stdout,"Big problem here, can't make starting game window.\n");
    return NULL;
  }
  self->windowupdate = UpdateObjPanel;
  self->windowdraw = DrawObjPanel;
  self->stealinput = 0;
  self->box.x = 80;
  self->box.y = 0;
  self->box.w = Camera.w - 80;
  self->box.h = 100;
  self->state = 0;
  TextButton(self,0,SDLK_p,"Prev",self->box.x + 24,88,64,10);
  TextButton(self,1,SDLK_n,"Next",self->box.x + 90,88,64,10);
  TextButton(self,2,SDLK_a,"Add",self->box.x + self->box.w - 74,64,64,10);
  TextButton(self,3,SDLK_s,"Select",self->box.x + self->box.w - 74,88,64,10);
  TextButton(self,4,SDLK_v,"View Keys",self->box.x + self->box.w - 74,40,64,10);
  TextButton(self,5,SDLK_DELETE,"Delete",self->box.x + self->box.w - 74,16,64,10);
  return self;
}


enum EditorStates {E_NONE, E_TILE,E_OBJECT,E_INFO};

void DrawMapEditorWindow(HUDInfo *self)
{
  int mx,my;
  int lw,lh;
  Uint8 *keys = SDL_GetKeyState(NULL);
  if(keys[SDLK_LEFT])Camera.x-=5;
  if(keys[SDLK_RIGHT])Camera.x+=5;
  if(keys[SDLK_UP])Camera.y-=5;
  if(keys[SDLK_DOWN])Camera.y+=5;
  DrawLevel(1);
  if(GetLevelSize(&lw,&lh))
  {
    mx = Mouse.x + Camera.x;
    my = Mouse.y + Camera.y;
    mx /= TILEW;
    mx *= TILEW;
    my /= TILEH;
    my *= TILEH;
    lw *= TILEW;
    lh *= TILEH;
    if((mx >= 0)&&(mx < lw)&&(my >= 0)&&(my < lh))
    DrawRect(mx - Camera.x,my - Camera.y,TILEW,TILEH,IndexColor(LightGreen));
  }
  DrawSprite(hudsprites[0],0,0,0,1);
}

int UpdateMapEditorWindow(HUDInfo *self,int pressID)
{
  switch(self->state)
  {
    case E_TILE:
      break;
  }
  switch(pressID)
  {
    case 10:
      if(self->Child != NULL)
      {
        PopWindow(self->Child->Handle);
      }
      self->Child = FilePanel();
      self->state = E_NONE;
      break;
    case 11:
      if(self->Child != NULL)
      {
        PopWindow(self->Child->Handle);
      }
      self->Child = TilePanel();
      self->state = E_TILE;
      break;
    case 12:
      if(self->Child != NULL)
      {
        PopWindow(self->Child->Handle);
      }
      self->Child = ObjPanel();
      self->state = E_OBJECT;
      break;
    case 13:
      if(self->Child != NULL)
      {
        PopWindow(self->Child->Handle);
      }
      self->Child = InfoPanel();
      self->state = E_NONE;
      break;
    default:
      break;
  }
  return 0;
}

void MapEditorWindow(char level[80])
{
  HUDInfo *self;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stdout,"Big problem here, can't make map editor window.\n");
    return;
  }
  self->windowupdate = UpdateMapEditorWindow;
  self->windowdraw = DrawMapEditorWindow;
  self->box.x = 0;
  self->box.y = 0;
  self->box.w = Camera.w;
  self->box.h = Camera.h;
  self->state = 0;
  TextButton(self,10,SDLK_F1,"File",16,16,64,10);
  TextButton(self,11,SDLK_t,"Tiles",16,40,64,10);
  TextButton(self,12,SDLK_o,"Objects",16,64,64,10);
  TextButton(self,13,SDLK_i,"Info",16,88,64,10);
  ShowMouse();
  DrawSplashScreen();
  NewLevel("untitled.lvl",32, 24, "images/backgrounds/bg01.png","system/tilelist01.def", 0, 1);
  LoadObjTypes();
  sprintf(LevelFilename,"untitled.lvl");
  return;
}

/*

      Title Window

*/


void DrawTitleWindow(HUDInfo *self)
{
  DrawSprite(self->image,0,0,0,1);
  DrawImageWindow(self->box.x,self->box.y,self->box.w,self->box.h);

DrawGradientRect(self->box.w/2 - 120 + self->box.x,self->box.y + 115, 240, 200, IndexColor(DarkGrey), IndexColor(DarkGrey), IndexColor(Black), IndexColor(Black),1);
  DrawTextCenteredShadow("Empire",self->box.x + self->box.w/2 - 2,self->box.y + 20 - 2,IndexColor(Gold),F_Large,2);
  DrawTextCenteredShadow("of Ruin",self->box.x + self->box.w/2 - 2,self->box.y + 20 + FontSize[F_Large] + 8 - 2,IndexColor(Gold),F_Medium,2);
}

void ContinueGame()
{
  char mappath[80];
  if(LoadPlayerGame(ChosenName))
  {
    DrawSplashScreen();
    PopWindow(WindowForDoom);
    sprintf(mappath,"levels/%s",PDat.map);
    GameWindow(mappath,0);
    ClearKeyboard();
  }
  else
  {
    NewMsg("Failed to Load Player Save!");
  }
}

int UpdateTitleWindow(HUDInfo *self,int pressID)
{
  switch(pressID)
  {
    case 0:
      WindowForDoom = self->Handle;
      GetText("Enter Your Hero's Name",ChosenName,80,ContinueGame,NULL);
      break;
    case 1:
      PopWindow(self->Handle);
      SelectRaceWindow();
      /*GameWindow("levels/untitled.lvl");*/
      return 1;
      break;
    case 2:
      PopWindow(self->Handle);
      SetupTutorial();
      return 1;
    case 3:
      ControlsWindow();
      return 1;
      break;
    case 4:
      PopWindow(self->Handle);
      MapEditorWindow("levels/untitled.lvl");
      return 1;
      break;
    case 5:
      QuitCheck();
      break;
  }
  return 0;
}

void TitleWindow()
{
  HUDInfo *self;
  int top, left;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stdout,"Big problem here, can't make face tracking window.\n");
    return;
  }
  self->windowupdate = UpdateTitleWindow;
  self->windowdraw = DrawTitleWindow;
  self->box.x = 55;
  self->box.y = 65;
  self->box.w = 345;
  self->box.h = 350;
  top = self->box.y + 120;
  left = self->box.w/2 - 115 + self->box.x;
  self->image = LoadSprite("images/promo_image.png",1024,768);
  TextButton(self,0,SDLK_c,"Continue",left,top,230,(int)(FontSize[F_Small] + 8));
  TextButton(self,1,SDLK_n,"New Game",left,top + ((int)(FontSize[F_Small] + 10)),230,(int)(FontSize[F_Small] + 8));
  TextButton(self,2,SDLK_t,"Tutorial",left,top + ((int)(FontSize[F_Small] + 10) * 2),230,(int)(FontSize[F_Small] + 8));
  TextButton(self,3,SDLK_o,"Options",left,top + ((int)(FontSize[F_Small] + 10) * 3),230,(int)(FontSize[F_Small] + 8));
  TextButton(self,4,SDLK_e,"Editor",left,top + ((int)(FontSize[F_Small] + 10) * 4),230,(int)(FontSize[F_Small] + 8));
  TextButton(self,5,SDLK_q,"Quit",left,top + ((int)(FontSize[F_Small] + 10) * 5),230,(int)(FontSize[F_Small] + 8));
  ClearKeyboard();
  return;
}

/*

  Base Window

*/
HUDInfo *SetupBaseWindow()
{
  LoadAssets();
  TitleWindow();
  return NULL;
}


/*camera*/

void UpdateEditorCamera()
{
  if((Mouse.x < (Camera.w * 0.1)) && (Mouse.y > 116))Camera.x-=5;
  if((Mouse.x > (Camera.w * 0.9)) && (Mouse.y > 116))Camera.x+=5;
  if((Mouse.y > 116) && (Mouse.y < (Camera.h * 0.1) + 116))Camera.y-=5;
  if(Mouse.y > (Camera.h * 0.9))Camera.y+=5;
}
/*eol@eof*/

#include "hud.h"

enum Button_States {B_Normal,B_Press,B_Highlight};

extern int EventMode;
extern Uint32 rmask,gmask,bmask,amask;
extern ScreenData  S_Data;
extern GLint viewport[4];
extern GLdouble modelview[16];
extern GLdouble projection[16];
extern int FontSize[3];
int    BUTTON_W;
int    BUTTON_H;

int Quitting = 0;
int windowInUse;
#define keydelay 8

int keymoved = 0;
int DRAWBUTTONRECT = 0;

Mouse_T Mouse;
int KeyCount;
Uint8 *oldkeys; /*last frame's key presses*/
Uint8 *keys;    /*current frame's key presses  Points to the live SDL struct*/

/**/
#define WindowTop WindowStack[WindowCount - 1]

HUDInfo hud;
HUDInfo *WindowStack[NUMWINDOWS];   /*the stack for organizing windows.  Se we can just shuffle pointers instead of all that info*/
int     WindowCount = 0;                /*how many windows are in effect*/
Sint32     WindowTag = 0;                  /*which window tag is next to be used*/
HUDInfo WindowList[NUMWINDOWS];     /*the actual window information*/

Sprite *WindowSprites[16];
Sprite *buttonsprites[9];
Sprite *buttonarrows[3];
Sprite *windowsprite;
Sprite *mediumwindowsprite;
Sprite *elements;
Sprite *attributes;
Sprite *hudsprites[32];


/*

  Generic window utilities

*/

inline int GetButtonH()
{
  return BUTTON_H;
}

inline int GetButtonW()
{
  return BUTTON_W;
}

int ClickOut(HUDInfo *self)
{
  if(!MouseOverXYWH(self->box.x, self->box.y, self->box.w, self->box.h))
  {
    if((Mouse.buttons) && (Mouse.oldbuttons == 0))
    {
      PopWindow(self->Handle);
      return 1;
    }
  }
  return 0;
}


/*

  Window Stack Functions

*/

void InitWindowStack()
{
  int i;
  for(i = 0;i < NUMWINDOWS;i++)
  {
    WindowStack[i] = NULL;
    if(WindowList[i].image != NULL)FreeSprite(WindowList[i].image);
    if(WindowList[i].ib != NULL)free(WindowList[i].ib);
    memset(&WindowList[i],0,sizeof(HUDInfo));
    WindowList[i].Handle = -1;
  }
  WindowCount = 0;
  WindowTag = 0;
}

HUDInfo *GetWindowByHandle(int handle)
{
  int i;
  for(i = 0;i < NUMWINDOWS;i++)
  {
    if(WindowList[i].Handle == handle)
    {
      return &WindowList[i];
    }
  }
  return NULL;
}

/*
  does not remove any of the assets associated with the window.  If any extra were loaded for a unique window, then it must be removed prior to calling this function
*/
void PopWindow(int handle)
{
  HUDInfo *window = NULL;
  HUDInfo *thiswindow = NULL;
  if(handle == -1)return;        /*we don't kill the base window*/
  thiswindow = GetWindowByHandle(handle);
  if(thiswindow->Child != NULL)
  {
    PopWindow(thiswindow->Child->Handle);
    thiswindow->Child = NULL;
  }
  window = WindowTop;
  if(window->Handle != handle)
  {
    BubbleWindow(handle);
    window = WindowTop;
  }
  WindowTop = NULL;             /*sets the top of the stack to point at NULL*/
  window->Handle = 0;
  window->inuse = 0;
  memset(window,0,sizeof(HUDInfo));
  WindowCount--;
}

/*
  Creates a new empty window and places it on top of the stack.
*/
HUDInfo *Pushwindow()
{
  int w;
  if((WindowCount + 1) >= NUMWINDOWS)
  {
    fprintf(stderr,"We're out of window space!\n");
    return NULL;/*we're out of space*/
  }
  for(w = 0;w < NUMWINDOWS;w++)
  {
    if(WindowList[w].inuse == 0)break;
  }
  if(w == NUMWINDOWS)
  {
    fprintf(stderr,"We're out of window space, but we shouldn't be...!\n");
    return NULL;/*this should never happen...*/
  }
  WindowCount++;
  WindowTop = &WindowList[w];
  memset(&WindowList[w],0,sizeof(HUDInfo));
  WindowTop->inuse = 1;
  WindowTop->Handle = ++WindowTag;/*we should be good for a looong time*/
  return WindowTop;
}

/*brings target window to the top*/
int BubbleWindow(int Handle)
{
  HUDInfo *temp = NULL;
  int w;
  int foundit = 0;
  for(w = 0;w < WindowCount;w++)/*go through window list for the desired handle*/
  {
    if(WindowStack[w]->Handle == Handle)
    {
      foundit = 1;
      break;
    }
  }
  if(!foundit)return 0;
  temp = WindowStack[w];
  for(;w < (WindowCount - 1);w++)
  {
    WindowStack[w] = WindowStack[w + 1];
  }
  WindowTop = temp;
  return 1;
}

/*
  Used to set up the base window, the one that should never be popped.
*/

void DrawImageBorder(int x, int y, int w, int h)
{
  int thick;
  Coord old5,old4;
  thick = WindowSprites[0]->h;
  old4 = WindowSprites[4]->dimen;
  old5 = WindowSprites[5]->dimen;
  WindowSprites[4]->dimen = GetGlCoord((S_Data.xres>>1)+WindowSprites[4]->w,(S_Data.yres>>1)+ h,_2DPLANE_, modelview, projection, viewport);
  WindowSprites[5]->dimen = GetGlCoord((S_Data.xres>>1)+w,(S_Data.yres>>1)+ WindowSprites[5]->h,_2DPLANE_, modelview, projection, viewport);
  WindowSprites[5]->w2 = ((float)w/128);
  WindowSprites[4]->h2 = ((float)h/128);
  DrawSprite(WindowSprites[5],x,y - thick,0,1);
  DrawSprite(WindowSprites[5],x,y + h,0,1);
  DrawSprite(WindowSprites[4],x - thick,y,0,1);
  DrawSprite(WindowSprites[4],x + w,y,0,1);
  DrawSprite(WindowSprites[0],x - thick/2 - (WindowSprites[0]->w/2), y - thick/2 - (WindowSprites[0]->h/2),0,1);
  DrawSprite(WindowSprites[1],x + w + thick/2 - (WindowSprites[0]->w/2), y - thick/2 - (WindowSprites[0]->h/2),0,1);
  DrawSprite(WindowSprites[2],x - thick/2 - (WindowSprites[0]->w/2), y + h + thick/2 - (WindowSprites[0]->h/2),0,1);
  DrawSprite(WindowSprites[3],x + w + thick/2 - (WindowSprites[0]->w/2), y + h + thick/2 - (WindowSprites[0]->h/2),0,1);
  WindowSprites[5]->w2 = 1;
  WindowSprites[4]->h2 = 1;
  WindowSprites[4]->dimen = old4;
  WindowSprites[5]->dimen = old5;
}


void DrawImageWindow(int x, int y, int w, int h)
{
  int thick;
  float alpha = 0.8;
  Coord old5,old6,old4;
  glColor4f(1,1,1,alpha);
  thick = WindowSprites[0]->h;
  old4 = WindowSprites[4]->dimen;
  old5 = WindowSprites[5]->dimen;
  old6 = WindowSprites[6]->dimen;
  WindowSprites[4]->dimen = GetGlCoord((S_Data.xres>>1)+WindowSprites[4]->w,(S_Data.yres>>1)+ h,_2DPLANE_, modelview, projection, viewport);
  WindowSprites[5]->dimen = GetGlCoord((S_Data.xres>>1)+w,(S_Data.yres>>1)+ WindowSprites[5]->h,_2DPLANE_, modelview, projection, viewport);
  WindowSprites[6]->dimen = GetGlCoord((S_Data.xres>>1)+w,(S_Data.yres>>1)+ h,_2DPLANE_, modelview, projection, viewport);
  WindowSprites[5]->w2 = ((float)w/128);
  WindowSprites[6]->w2 = ((float)w/128);
  WindowSprites[6]->h2 = ((float)h/128);
  WindowSprites[4]->h2 = ((float)h/128);
  DrawSprite(WindowSprites[5],x,y - thick,0,1);
  DrawSprite(WindowSprites[5],x,y + h,0,1);
  DrawSprite(WindowSprites[6],x ,y,0,1);
  DrawSprite(WindowSprites[4],x - thick,y,0,1);
  DrawSprite(WindowSprites[4],x + w,y,0,1);
  DrawSprite(WindowSprites[0],x - thick/2 - (WindowSprites[0]->w/2), y - thick/2 - (WindowSprites[0]->h/2),0,1);
  DrawSprite(WindowSprites[1],x + w + thick/2 - (WindowSprites[0]->w/2), y - thick/2 - (WindowSprites[0]->h/2),0,1);
  DrawSprite(WindowSprites[2],x - thick/2 - (WindowSprites[0]->w/2), y + h + thick/2 - (WindowSprites[0]->h/2),0,1);
  DrawSprite(WindowSprites[3],x + w + thick/2 - (WindowSprites[0]->w/2), y + h + thick/2 - (WindowSprites[0]->h/2),0,1);
  WindowSprites[5]->w2 = 1;
  WindowSprites[6]->w2 = 1;
  WindowSprites[6]->h2 = 1;
  WindowSprites[4]->h2 = 1;
  WindowSprites[4]->dimen = old4;
  WindowSprites[5]->dimen = old5;
  WindowSprites[6]->dimen = old6;
  glColor4f(1,1,1,1);
}

void DrawButton(Button *button)
{
  int color;
  if((button->shown)&&(!button->off))
  {
    if(button->button != NULL) DrawSprite( button->button,button->box.x,button->box.y + button->state, button->frame,1);
    if(strlen(button->text) > 0)
    {
      if(button->hasfocus == 1)color = button->hcolor;
      else color = button->color;
      if(button->centered)
      {
        DrawTextCentered(button->text,1 + button->box.x + (button->box.w >> 1) - 5,button->box.y + (button->box.h >> 1) + 1 - 15 + button->state, IndexColor(DarkGrey), button->font);
        DrawTextCentered(button->text,button->box.x + (button->box.w >> 1) - 5,button->box.y + (button->box.h >> 1) - 15 + button->state, IndexColor(color), button->font);
      }
      else
      {
        DrawTxt(button->text,1 + button->box.x  + 5,button->box.y + (button->box.h >> 1) + 1 - 15 + button->state, IndexColor(DarkGrey), button->font);
        DrawTxt(button->text,button->box.x + 5,button->box.y + (button->box.h >> 1) - 15 + button->state, IndexColor(color), button->font);
      }
    }
    if(DRAWBUTTONRECT)
    {
      DrawRect(button->box.x,button->box.y,button->box.w,button->box.h, IndexColor(Yellow));
    }
  }
}

void DrawAllWindows()
{
  int i,b;
  glPushMatrix();
  glEnable(GL_BLEND);
  glColor4f(1,1,1,0.75);
  for(i = 0;i < WindowCount;i++)
  {
    if(WindowStack[i] != NULL)
    {
      if(WindowStack[i]->windowdraw != NULL)
      {
        WindowStack[i]->windowdraw(WindowStack[i]);
      }
      else /*draw defaul window*/
      {
        DrawImageWindow(WindowStack[i]->box.x,WindowStack[i]->box.y,WindowStack[i]->box.w,WindowStack[i]->box.h);
      }
      for(b = 0;b < WindowStack[i]->numbuttons;b++)
      {
        DrawButton(&WindowStack[i]->buttons[b]);
        if((WindowStack[i]->highlightrect)&&(WindowStack[i]->buttons[b].hasfocus))
        {
          DrawRect(WindowStack[i]->buttons[b].box.x,WindowStack[i]->buttons[b].box.y,WindowStack[i]->buttons[b].box.w,WindowStack[i]->buttons[b].box.h, IndexColor(LightBlue));
        }
      }
    }
  }
  glColor4f(1,1,1,1);
  glDisable(GL_BLEND);
  glPopMatrix();
}

void UpdateAllWindows()
{
  int i,w;
  int mousemoved = 0;
  Uint8 *keys;
  SDLMod mod;
  int hl;
  keys = SDL_GetKeyState(NULL);
  mod = SDL_GetModState();
  /*handle button focus*/
  /*  if(EventMode)return;*/
  windowInUse =0;
  if((Mouse.x != Mouse.ox)||(Mouse.y != Mouse.oy))mousemoved = 1;
  for(w = (WindowCount - 1);w >= 0;w--)
  {
    hl = 0;
    for(i = 0;i < WindowStack[w]->numbuttons;i++)
    {
      if((WindowStack[w]->buttons[i].shown)&&(!WindowStack[w]->buttons[i].off))
      {     /*only check mouse position if it has moved*/
        if(MouseOver(WindowStack[w]->buttons[i].box))
        {
          ResetFocus(WindowStack[w],i);
          hl = 1;
          if(Mouse.buttons)
          {
            WindowStack[w]->buttons[i].button = WindowStack[w]->buttons[i].buttons[1];
            WindowStack[w]->buttons[i].state = 1;
          }
          break;
        }
      }
    }
    if(!hl)ResetFocus(WindowStack[w],-1);
    /*TODO:need to add buttons that page through selections*/
    /*check if anything was pressed*/
    for(i = 0;i < WindowStack[w]->numbuttons;i++)
    {
      if((WindowStack[w]->buttons[i].shown)&&(!WindowStack[w]->buttons[i].off))
      {
        if((isKeyReleased(WindowStack[w]->buttons[i].hotkey)) || ((MouseOver(WindowStack[w]->buttons[i].box)) && (Mouse.oldbuttons) && (Mouse.buttons == 0)) || ((WindowStack[w]->buttonfocus == 1)&&(mod & KMOD_CTRL)&&(!keymoved)))
        {
          if(WindowStack[w]->windowupdate != NULL)
          {
            if(WindowStack[w]->windowupdate(WindowStack[w], WindowStack[w]->buttons[i].buttonID))return;
          }
        }
      }
    }
    ClearKeyboard();
    /*at this point, no standard button has been pressed, but there may be more input methods so we call the update, giving it -1 as the input button*/
    if(WindowStack[w]->windowupdate != NULL)
    {
      if(WindowStack[w]->windowupdate(WindowStack[w],-1))return;
    }
    if(WindowStack[w]->stealinput)return;/*do not pass input back along the stack*/
  }
}

/*
  Button Maintenance
*/

void SetButton(Button *button,int buttonID,int hotkey, char *text,Sprite *sprite,Sprite *sprite1,Sprite *sprite2,Sprite *sprite3,int x,int y,int w,int h,int shown,int frame,int c1, int c2, int font,int centered)
{
  strcpy(button->text,text);
  button->button = sprite;
  button->hotkey = hotkey;
  button->buttons[0] = sprite1;
  button->buttons[1] = sprite2;
  button->buttons[2] = sprite3;
  button->buttonID = buttonID;
  button->frame = frame;
  button->box.x = x;
  button->box.y = y;
  button->box.w = w;
  button->box.h = h;
  button->shown = shown;
  button->font = font;
  button->color = c1;
  button->hcolor = c2;
  button->centered = centered;
}

int  PicButton(HUDInfo *window,int buttonID,int hotkey,char *text,int x,int y,int w,int h)
{
  
  SetButton(&window->buttons[window->numbuttons],buttonID,hotkey, text,buttonsprites[0], buttonsprites[0], buttonsprites[1], buttonsprites[2],x,y,w,h,1,0,Silver,White,F_Small,1);
  return window->numbuttons++;
}

int  LPicButton(HUDInfo *window,int buttonID,int hotkey,char *text,int x,int y,int w,int h)
{
  
  SetButton(&window->buttons[window->numbuttons],buttonID,hotkey, text,buttonsprites[3], buttonsprites[3], buttonsprites[4], buttonsprites[5],x,y,w,h,1,0,Silver,White,F_Small,1);
  return window->numbuttons++;
}

int  FPicButton(HUDInfo *window,int buttonID,int hotkey,char *text,int x,int y,int w,int h)
{
  
  SetButton(&window->buttons[window->numbuttons],buttonID,hotkey, text,buttonsprites[6], buttonsprites[6], buttonsprites[7], buttonsprites[8],x,y,w,h,1,0,Silver,White,F_Small,1);
  return window->numbuttons++;
}

int ArrowButton(HUDInfo *window,int buttonID,int hotkey,char *text,int x,int y,int w,int h,int dir)
{
  SetButton(&window->buttons[window->numbuttons],buttonID,hotkey, text,buttonsprites[3], buttonsprites[4], buttonsprites[5], buttonsprites[3],x,y,w,h,1,0,Silver,White,F_Small,1);
  switch(dir)
  {
    case SDLK_LEFT:
    case 0:
      /*left*/
      window->buttons[window->numbuttons].frame = 1;
      break;
    case 1:
    case SDLK_RIGHT:
      /*right*/
      window->buttons[window->numbuttons].frame = 3;
      break;
    case 2:
    case SDLK_UP:
      /*up*/
      window->buttons[window->numbuttons].frame = 2;
      break;
    case 3:
    case SDLK_DOWN:
      /*down*/
      window->buttons[window->numbuttons].frame = 0;
      break;
    case 4:
    case SDLK_x:
      /*X*/
      window->buttons[window->numbuttons].frame = 5;
      break;
    case 5:
    case SDLK_s:
      /*Square*/
      window->buttons[window->numbuttons].frame = 4;
      break;
  }
  return window->numbuttons++;
}

int  TextButton(HUDInfo *window,int buttonID,int hotkey,char *text,int x,int y,int w,int h)
{
  SetButton(&window->buttons[window->numbuttons],buttonID,hotkey, text , NULL, NULL, NULL, NULL, x, y, w, h,1,0,Silver,White,F_Small,1);
  return window->numbuttons++;
}

int  LTextButton(HUDInfo *window,int buttonID,int hotkey,char *text,int x,int y,int w,int h)
{
  SetButton(&window->buttons[window->numbuttons],buttonID,hotkey, text , NULL, NULL, NULL, NULL, x, y, w, h,1,0,LightGrey,LightGreen,F_Medium,1);
  return window->numbuttons++;
}

void ModifyButton(Button *button,int state,int shown)
{
  button->button = buttonsprites[state];
  button->shown = shown;
}

void ResetButtons()
{
  int j;
  for(j = 0;j < HUDBUTTONS;j++)
  {
    ModifyButton(&hud.buttons[j],B_Normal,0);
  }
}

void ButtonOff(HUDInfo *window,int button)
{
  window->buttons[button].off = 1;
}

void ButtonOn(HUDInfo *window,int button)
{
  window->buttons[button].off = 0;
}


void ResetFocus(HUDInfo *window,int button)
{
  int i;
  window->buttonfocus = button;
  for(i = 0;i < window->numbuttons;i++)
  {
    window->buttons[i].state = 0;
    if(button == i)
    {
      window->buttons[i].button = window->buttons[i].buttons[2];
      window->buttons[i].hasfocus = 1;
    }
    else
    {           /*pressed buttons are not highlighted*/
      window->buttons[i].button = window->buttons[i].buttons[0];
      window->buttons[i].hasfocus = 0;
    }
  }
}

/*

  Window Maintenance

*/

void DrawStatusBarHoriz(int stat,int range,int FG,int BG,int x, int y, int w, int h)
{
  float percent;
  DrawFilledRect(x,y, w, h, IndexColor(BG));
  if((stat > 0)&&(range != 0))
  {
    percent = (stat * w) / range;
    DrawFilledRect(x,y, percent, h, IndexColor(FG));
  }

}

void DrawWindow(int size,int x, int y)
{
  switch(size)
  {
    case 1:
      DrawSprite(mediumwindowsprite,x,y,0,1);
      break;
    case 2:
      DrawSprite(windowsprite,x,y,0,1);
      break;
  }
}

/*
 *    Text Windows
 */
 

void CloseWin(HUDInfo *self,int pressID)
{
  if(pressID == BI_CloseWin)
  {
    keymoved = 2;
    return;
  }
}

void DrawInfoWin(HUDInfo *self)
{
  int height;
  DrawSprite(windowsprite,30,20,0,1);
  DrawTextCentered(hud.windowheader,120,32,IndexColor(DarkGrey),F_Large);
  DrawTextCentered(hud.windowheader,119,31,IndexColor(Green),F_Large);
  if(hud.image == NULL)height = 50;
  else
  {
    DrawSprite(hud.image,120 - (hud.image->w /2), 50,hud.subwindowinfo,1);
    height = 60 + hud.image->h;
  }
  DrawTextBlock(hud.description,38, height,IndexColor(LightBlue),F_Small,172);
}


void InfoWindow(char title[80],Sprite *image,int frame,char description[120])
{
  hud.numbuttons = 1;
  hud.buttonfocus = 0;
  hud.subwindow = 1;
  hud.subwindowinfo = frame;
  hud.image = image;
  strncpy(hud.windowheader,title,80);
  strncpy(hud.description,description,80);
  SetButton(&hud.buttons[0],BI_CloseWin, -1," ",buttonarrows[0],buttonarrows[0], buttonarrows[1], buttonarrows[2],182,18,16,16,1,5,Silver,White,F_Small,1);
  hud.windowdraw = DrawInfoWin;

}



/*
 *    Yes No Windows
*/

void GoQuit()
{
  exit(1);
}

void NoQuit()
{
  Quitting = 0;
  HideMouse();
}

void QuitCheck()
{
  if(Quitting)return;/*no need to do it more than once.*/
  ShowMouse();
  Quitting = 1;
  YesNo("Quit?",GoQuit,NoQuit);
}

void DrawYesNoWindow(HUDInfo *self)
{
  DrawImageWindow(self->box.x,self->box.y, self->box.w,self->box.h);
  DrawTextCentered(self->windowheader,self->box.x + (self->box.w/2) + 2,10 + self->box.y + 2,IndexColor(DarkGrey),F_Medium);
  DrawTextCentered(self->windowheader,self->box.x + (self->box.w/2),10 + self->box.y + 1,IndexColor(White),F_Medium);
}

int UpdateYesNoWindow(HUDInfo *self,int pressID)
{
  if(pressID == -1)return 0;
  switch(pressID)
  {
    case BI_Yes:
      self->readyreturn = 1;
      if(self->optionrun[0])self->optionrun[0]();
      PopWindow(self->Handle);
      return 1;
      break;
    case BI_No:
      self->readyreturn = 0;
      if(self->optionrun[1])self->optionrun[1]();
      PopWindow(self->Handle);
      return 1;
      break;
    case BI_CloseWin:
      self->readyreturn = 0;
      PopWindow(self->Handle);
      return 1;
  }
  return 0;
}

int *YesNo(char message[80],void (*option1)(),void (*option2)())
{
  return TwoOptions(message,"Yes","No",option1,option2);
}

int *TwoOptions(char message[80],char op1[80],char op2[80],void (*option1)(),void (*option2)())
{
  HUDInfo *self;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stdout,"Big problem here, can't make TwoOptions dialog window.\n");
    return NULL;
  }
  self->box.y = S_Data.yres * 0.3;
  self->box.w = (FontSize[F_Medium] * strlen(message));
  self->box.h = FontSize[F_Medium] + (buttonsprites[0]->h *1.5);
  if(self->box.w < 340)self->box.w = 340;
  self->box.x = (S_Data.xres / 2) - (self->box.w /2);
  self->stealinput = 1;
  self->readyreturn = -1;
  self->optionrun[0] = option1;
  self->optionrun[1] = option2;
  strncpy(self->windowheader,message,80);
  PicButton(self,BI_Yes,SDLK_y,op1,self->box.x + (self->box.w/4) - 75,self->box.y + self->box.h - 55,165,58);
  PicButton(self,BI_No,SDLK_n,op2,self->box.x + (self->box.w * 0.75) - 75,self->box.y + self->box.h - 55,165,58);
  self->windowdraw = DrawYesNoWindow;
  self->windowupdate = UpdateYesNoWindow;
  return &self->readyreturn;
}
/*ask message, put in str, call option on done*/

void DrawGetText(HUDInfo *self)
{
  int h = GetFontHeight(F_Medium);
  int w = self->box.w - 20;
  if((self->editstring != NULL) && (self->state > 0))
  {
    h = GetTextBlockHeight(self->editstring,F_Medium,w) - h;
    self->box.h = 128 - GetFontHeight(F_Medium) + h;
    self->buttons[0].box.y = self->box.y + self->box.h - 50;
    self->buttons[1].box.y = self->buttons[0].box.y ;
  }
  DrawImageWindow(self->box.x,self->box.y, self->box.w,self->box.h);
  DrawTextCentered(self->windowheader,self->box.x + (self->box.w/2) + 2,6 + self->box.y + 2,IndexColor(DarkGrey),F_Large);
  DrawTextCentered(self->windowheader,self->box.x + (self->box.w/2),6 + self->box.y + 1,IndexColor(White),F_Large);
  DrawFilledRect(self->box.x + 10,self->box.y + FontSize[F_Medium] + 18, self->box.w - 20, h, IndexColor(DarkGrey));
  DrawRect(self->box.x + 10,self->box.y + FontSize[F_Medium] + 18, w, h, IndexColor(Yellow));
  if((self->editstring != NULL) && (self->state > 0))
  {
    /*DrawTxt(self->editstring,self->box.x + 10,self->box.y + FontSize[F_Medium] + 16,IndexColor(White),F_Medium);*/
    DrawTextBlock(self->editstring,self->box.x + 10,self->box.y/* + FontSize[F_Medium]*/ + 12 ,IndexColor(White),F_Medium,w);
  }
}

int UpdateGetText(HUDInfo *self,int pressID)
{
  SDL_Event event;
  int shift = 0;
  if(pressID == BI_No)
  {
    self->readyreturn = 0;
    if(self->optionrun[1])self->optionrun[1]();
    PopWindow(self->Handle);
    return 1;
  }
  if(pressID == BI_OK)
  {
    self->readyreturn = 1;
    if(self->optionrun[0])self->optionrun[0]();
    PopWindow(self->Handle);
    return 1;
  }
  if(self->editstring == NULL)
  {
    return 0;
  }
  if(SDL_GetModState() & KMOD_SHIFT)
  {
    shift = 1;
  }
  while(SDL_PeepEvents(&event, 1, SDL_PEEKEVENT, SDL_EVENTMASK (SDL_KEYDOWN)))
  {
    if((event.type == SDL_KEYDOWN))
    {
      if((self->state + 1) < self->editstringlen)
      {
        if((event.key.keysym.sym >= SDLK_a)&&(event.key.keysym.sym <= SDLK_z))
        {
          if(shift)
          {
            self->editstring[self->state] = event.key.keysym.sym - SDLK_a + 'A';
          }
          else
          {
            self->editstring[self->state] = event.key.keysym.sym - SDLK_a + 'a';
          }
          self->state++;
          self->editstring[self->state] = '\0';
        }
        else if(!shift)
        {
          if((event.key.keysym.sym >= SDLK_0)&&(event.key.keysym.sym <= SDLK_9))
          {
            self->editstring[self->state] = event.key.keysym.sym - SDLK_0 + '0';
            self->editstring[++self->state] = '\0';
          }
          else if(event.key.keysym.sym == SDLK_SPACE)
          {
            self->editstring[self->state] = ' ';
            self->editstring[++self->state] = '\0';
          }
          else if(event.key.keysym.sym == SDLK_PERIOD)
          {
            self->editstring[self->state] = '.';
            self->editstring[++self->state] = '\0';
          }
          else if(event.key.keysym.sym == SDLK_MINUS)
          {
            self->editstring[self->state] = '-';
            self->editstring[++self->state] = '\0';
          }
          else if(event.key.keysym.sym == SDLK_COMMA)
          {
            self->editstring[self->state] = ',';
            self->editstring[++self->state] = '\0';
          }
          else if(event.key.keysym.sym == SDLK_SLASH)
          {
            self->editstring[self->state] = '/';
            self->editstring[++self->state] = '\0';
          }
          else if(event.key.keysym.sym == SDLK_COLON)
          {
            self->editstring[self->state] = ':';
            self->editstring[++self->state] = '\0';
          }
          else if(event.key.keysym.sym == SDLK_SEMICOLON)
          {
            self->editstring[self->state] = ';';
            self->editstring[++self->state] = '\0';
          }
          else if(event.key.keysym.sym == SDLK_QUOTE)
          {
            self->editstring[self->state] = '\'';
            self->editstring[++self->state] = '\0';
          }
          else if(event.key.keysym.sym == SDLK_QUOTEDBL)
          {
            self->editstring[self->state] = '\"';
            self->editstring[++self->state] = '\0';
          }
          
        }
      }
      if(event.key.keysym.sym == SDLK_BACKSPACE)
      {
        if(self->state > 0)
        {
          self->editstring[--self->state] = '\0';
        }
      }
      return 1;
    }
  }
  return 0;
}

int *GetText(char message[80],char *str,int stringlen,void (*option)(),void (*option2)())
{
  HUDInfo *self;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stdout,"Big problem here, can't make text dialog window.\n");
    return NULL;
  }
  strncpy(self->windowheader,message,80);
  self->box.x = (S_Data.xres / 2) - 300;
  self->box.y = S_Data.yres * 0.3;
  self->box.w = 600;
  self->box.h = 128;
  self->optionrun[0] = option;
  self->optionrun[1] = option2;
  self->readyreturn = -1;
  self->stealinput = 1;
  self->editstring = str;
  self->state = strlen(str);
  self->editstringlen = stringlen;
  self->windowdraw = DrawGetText;
  self->windowupdate = UpdateGetText;
  PicButton(self,BI_OK,SDLK_RETURN,"Done",self->box.x + (self->box.w/2) - 170,self->box.y + self->box.h - 50,165,58);
  PicButton(self,BI_No,SDLK_ESCAPE,"Cancel",self->box.x + (self->box.w/2) + 5,self->box.y + self->box.h - 50,165,58);
  return &self->readyreturn;
}


/*

  HUD Maintenance

*/

void LoadHUD()
{
  buttonsprites[0] = LoadSprite("images/UI/btn.png",165,58);
  buttonsprites[1] = LoadSprite("images/UI/btn_hit.png",165,58);
  buttonsprites[2] = LoadSprite("images/UI/btn_high.png",165,58);
  buttonsprites[3] = LoadSprite( "images/UI/arrows.png", 24,24);
  buttonsprites[4] = LoadSprite( "images/UI/arrows_hit.png", 24,24);
  buttonsprites[5] = LoadSprite( "images/UI/arrows_high.png", 24,24);
  BUTTON_W = 165;
  BUTTON_H = 58;
  WindowSprites[0] = LoadSprite("images/UI/windowborder_corner_UL.png",8,8);
  WindowSprites[1] = LoadSprite("images/UI/windowborder_corner_UR.png",8,8);
  WindowSprites[2] = LoadSprite("images/UI/windowborder_corner_BL.png",8,8);
  WindowSprites[3] = LoadSprite("images/UI/windowborder_corner_BR.png",8,8);
  WindowSprites[4] = LoadSprite("images/UI/windowborder_vert.png",8,128);
  WindowSprites[5] = LoadSprite("images/UI/windowborder_horiz.png",128,8);
  WindowSprites[6] = LoadSprite("images/UI/windowbackground1.png",128,128);
  /*hudsprites[0] = LoadSprite("images/UI/hudtop.png",1024,64);*/
  hudsprites[0] = LoadSprite("images/UI/topbar.png",1024,116);
  hudsprites[1] = LoadSprite("images/UI/percentbar.png",310,26);
  hudsprites[2] = LoadSprite("images/UI/equipped.png",88,86);
}




/*
  Mouse Maintenance functions
*/

int MouseOverXYWH(int x, int y, int w, int h)
{
  SDL_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;
  return MouseOver(rect);
}

int MouseOver(SDL_Rect rect)
{
  if((Mouse.x >= rect.x)&&(Mouse.x < rect.x + rect.w)&&(Mouse.y >= rect.y)&&(Mouse.y < rect.y + rect.h))
    return 1;
  return 0;
}

Uint32 MouseInXYWH(int x, int y, int w, int h)
{
  SDL_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;
  return MouseIn(rect);
}

Uint32 MouseIn(SDL_Rect rect)
{
  if((Mouse.buttons != 0)&&(Mouse.oldbuttons == 0))
  {
    if((Mouse.x >= rect.x)&&(Mouse.x < rect.x + rect.w)&&(Mouse.y >= rect.y)&&(Mouse.y < rect.y + rect.h))
      return Mouse.buttons;
  }
  return 0;
}

void InitMouse()
{
  Mouse.sprite=LoadSprite("images/UI/mouse_icon.png",25,43);
  Mouse.state = 0;
  Mouse.shown = 1;
  Mouse.frame = 0;
  Mouse.scale = 1;
}

void ShowMouse()
{
  Mouse.shown++;
}

void HideMouse()
{
  Mouse.shown--;
}

int MouseHeld(int button)
{
  if((Mouse.buttons & SDL_BUTTON(button))&&((Mouse.oldbuttons & SDL_BUTTON(button))))return 1;
  return 0;
}

int MousePressed(int button)
{
  if((Mouse.buttons & SDL_BUTTON(button))&&(!(Mouse.oldbuttons & SDL_BUTTON(button))))return 1;
  return 0;
}

int MouseReleased(int button)
{
  if((Mouse.oldbuttons & SDL_BUTTON(button))&&(!(Mouse.buttons & SDL_BUTTON(button))))return 1;
  return 0;
}

void DrawMouse()
{
  int mx,my;
  int dx,dy;
  mx = Mouse.x;
  my = Mouse.y;
  SDL_GetRelativeMouseState(&dx,&dy);
  Mouse.oldbuttons = Mouse.buttons;
  Mouse.buttons = SDL_GetMouseState(&mx,&my);
  glDisable(GL_DEPTH_TEST);
  glColor4f(1,1,1,0.7f);
  if(Mouse.shown > 0)DrawSprite(Mouse.sprite, mx,my,Mouse.frame,1.0f);
 /* Mouse.frame = (Mouse.frame +1)%16;*/
  glColor4f(1,1,1,1);
  glEnable(GL_DEPTH_TEST);
  Mouse.ox = Mouse.x;
  Mouse.oy = Mouse.y;
  Mouse.x = mx;
  Mouse.y = my;
}

int isMouseClicked()
{
  if(Mouse.clickdelay <= 0)
  {
    if(SDL_GetMouseState(NULL,NULL) && Mouse.shown == 0)
    {
      Mouse.shown = 1;
      return 1;
    }
  }
  return 0;
}


void KillMouse()
{
  FreeSprite(Mouse.sprite);
  Mouse.sprite = NULL;
}

/*

  Quick Keyboard access without going through SDL EVENTS

*/

void InitKeyboard()
{
  keys = SDL_GetKeyState(&KeyCount);
  oldkeys = (Uint8 *)malloc(sizeof(Uint8)*KeyCount);
  if(oldkeys == NULL)
  {
    fprintf(stderr,"unable to initialize keyboard structure!\n");
  }
}

void ClearKeyboard()
{
  if(oldkeys == NULL)return;
  memset(oldkeys,0,sizeof(Uint8)*KeyCount);
}

void updateKeyboard()
{
  int i;
  if((oldkeys == NULL)||(keys == NULL))
  {
    return;
  }
  for(i = 0; i < KeyCount;i++)
  {
    oldkeys[i] = keys[i];
  }
  keys = SDL_GetKeyState(NULL);
}

int isKeyPressed(int key)
{
  if((oldkeys == NULL)||(keys == NULL))
  {
    return 0;
  }
  if((keys[key]) && (!oldkeys[key]))
  {
    return 1;
  }
  return 0;
}

int isKeyReleased(int key)
{
  if((oldkeys == NULL)||(keys == NULL))
  {
    return 0;
  }
  if((!keys[key]) && (oldkeys[key]))
  {
    return 1;
  }
  return 0;
}

int isKeyHeld(int key)
{
  if((oldkeys == NULL)||(keys == NULL))
  {
    return 0;
  }
  if((keys[key]) && (oldkeys[key]))
  {
    return 1;
  }
  return 0;
}

/*eol@eof*/

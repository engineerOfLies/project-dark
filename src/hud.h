#ifndef __HUD__
#define __HUD__

/*
  Functions for handling the HUD and parsing commands
*/

#include "graphics.h"
#include "modelLoader.h"

enum Button_Pic {BP_Gem, BP_Larrow, BP_Rarrow, BP_Ex};
enum Button_ID  {BI_NewGame, BI_LoadGame, BI_Quit, BI_Care, BI_Status, BI_World, BI_Feed, BI_Train, BI_Rest, BI_Skills, BI_Stats, BI_Stage, BI_Search, BI_Move, BI_Link, BI_CloseWin, BI_Next,BI_Prev, BI_Select, BI_Yes, BI_No, BI_OK, BI_ListStart = 100,BI_Letter = 1000};
            /*BI_Letter must remain the last in the list*/

#define HUDBUTTONS 64
#define NUMWINDOWS 32

typedef struct
{
  Uint32 state2;
  Uint8 clickdelay;
  Sprite *sprite;
  Uint32 buttons;
  Uint32 oldbuttons;
  Sint32 state;
  Uint32 shown;
  Uint32 frame;
  Uint16  x, y;
  Uint16 ox,oy;
  float  scale;
}Mouse_T;

typedef struct
{
  Sprite *button;       /*pointer to active button*/
  Sprite *buttons[3];   /*up to 3 different states for button: idle, selected & active*/
  int off;              /*if button is off, it will not be checked against*/
  int color,hcolor;     /*font colors for normal and highlighted*/
  int font;             /*which font to use*/
  int centered;         /*weather to use left justify or centering*/
  int hasfocus;         /*buttons that have the focus are highlighted in some way*/
  int buttonID;         /*when button is pressed, this is the action sent*/
  SDL_Rect box;         /*area that is clickable*/
  char text[80];        /*text to be written on button*/
  int state;            /*current state : idle, selected or clicked*/
  int frame;            /*which frame of the sprite to use*/
  int hotkey;           /*if there is a hotkey for this button, otherwise its -1*/
  int shown;            /*the the button should bbe drawn or not*/
  int drawtype;         /*0 sprite 1 drawn 2 drawn translucent.  If drawn the box is used for dimensions*/
}Button;

typedef struct HUD_S
{
  int inuse;                    /*so as we don't reassign a functioning window*/
  int Handle;                   /*index of this window*/
  struct HUD_S *Parent;         /*the parent that spawned this window*/
  struct HUD_S *Child;         /*the parent that spawned this window*/
  SDL_Rect  box;                /*the position and size of the window.  When a sprite is is used, the position is used for the drawing.*/
  int *ref;                     /*in case this window is waiting on another window*/
  int stealinput;               /*If input is passed to windows below this one on the stack*/
  int ParentStealsFocus;        /*if the parent should bubble up to the top of the stack.*/
  Button buttons[HUDBUTTONS];   /*buttons to be used by thiss window*/
  int highlightrect;            /*if true will draw the button rect around the highlit button*/
  int numbuttons;               /*how many of them are in use*/
  int buttonfocus;              /*which button has the focus*/
  int state;                    /*state info for the window.  May be different for each window*/
  int subwindow;                /*same*/
  int subwindowinfo;            /*same*/
  float real;                   /*floating point info*/
  float real2;
  int readyreturn;              /*when this window is ready to return*/
  int submenu;
  Sprite *image;                /*background sprite for the window*/
  void *ib;                     /*pointer to unique data for this window*/
  char windowheader[80];        /*title for the window*/
  char description[512];        /*text for the window*/
  char *editstring;             /*for string gathering windows, this points to the string to be edited*/
  char editstringlen;           /*for string gatherers.  this is the max character length*/
  void (*windowdraw)(struct HUD_S *self);         /*custom draw function.  If it don't exist, it will fall back on deaults*/
  int (*windowupdate)(struct HUD_S *self,int pressID);/*updates the status for the window, receiving the button that has been pressed, returning successful input catch*/
  void (*optionrun[10])();      /*functions to run when the window is done, based on different conditions*/
}HUDInfo;

/*functions for windows using the stack system*/
HUDInfo *GetWindowByHandle(int handle);   /*returns a pointer to the window with the given handle*/
void PopWindow(int handle);               /*clears the window that is refered to by the handle*/
int BubbleWindow(int Handle);  /*rearranges the stack to put this window on the top of the stack.  returns 0 on failure, 1 on success*/
HUDInfo *Pushwindow();
void InitWindowStack();
void DrawAllWindows();
void UpdateAllWindows();

void DrawImageBorder(int x, int y, int w, int h);
void DrawImageWindow(int x, int y, int w, int h);
int ClickOut(HUDInfo *self);

void LoadHUD();
void KillHUD();
void DrawHUD();
void UpdateHUD();
void ResetFocus(HUDInfo *window,int button);
void SetButton(Button *button,int buttonID,int hotkey, char *text,Sprite *sprite,Sprite *sprite1,Sprite *sprite2,Sprite *sprite3,int x,int y,int w,int h,int shown,int frame,int c1, int c2, int font,int centered);
/*a simpler verions of SetButton, with some issues assumed, returns the index of the button for the window specified*/
inline int GetButtonH();
inline int GetButtonW();
void KillButton(HUDInfo *window,int button);
void ButtonOff(HUDInfo *window,int button);
void ButtonOn(HUDInfo *window,int button);
int PicButton(HUDInfo *window,int buttonID,int hotkey,char *text,int x,int y,int w,int h);
int TextButton(HUDInfo *window,int buttonID,int hotkey,char *text,int x,int y,int w,int h);
int LTextButton(HUDInfo *window,int buttonID,int hotkey,char *text,int x,int y,int w,int h);
int LPicButton(HUDInfo *window,int buttonID,int hotkey,char *text,int x,int y,int w,int h);
int FPicButton(HUDInfo *window,int buttonID,int hotkey,char *text,int x,int y,int w,int h);
int ArrowButton(HUDInfo *window,int buttonID,int hotkey,char *text,int x,int y,int w,int h,int dir);

void SetupMainMenu();
void SetupGameMenu();

void DrawStatusBarHoriz(int stat,int range,int FG,int BG,int x, int y, int w, int h);

/*
 returns a pointer to an integer that will be changed from -1 to either 1 or 0 when selected.
 Takes two function pointers that will be called for either the yes event or no event
*/
void QuitCheck();
int *YesNo(char message[80],void (*yes)(),void (*no)());
int *TwoOptions(char message[80],char op1[80],char op2[80],void (*option1)(),void (*option2)());
int *GetText(char message[80],char *str,int stringlen,void (*option)(),void (*option2)());

/*mouse handling stuff*/
void InitMouse();
void DrawMouse();
void KillMouse();
Uint32 MouseInXYWH(int x, int y, int w, int h);  /*calls the below, but doesn't need the rect struct*/
Uint32 MouseIn(SDL_Rect rect); /*returns the button mask IF the mouse is in the Rectangle*/
int MouseOverXYWH(int x, int y, int w, int h);
int MouseOver(SDL_Rect rect);
void ShowMouse();
void HideMouse();
int MouseHeld(int button);
int MousePressed(int button);
int MouseReleased(int button);
void InitKeyboard();
void ClearKeyboard();
void updateKeyboard();
int isKeyPressed(int key);
int isKeyReleased(int key);
int isKeyHeld(int key);


#endif

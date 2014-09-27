#include <physfs.h>
#include "controls.h"
#include "menus.h"


Input InputList[MAXINPUTS];   /*big list of control inputs*/
int NumInputs = 0;                /*how many are configured*/
extern SDL_Rect Camera;


/*

  Controls Menu

*/

void DrawControl(char *name, int x1, int x2, int x3, int y)
{
  Input *input;
  char text[80];
  if(name == NULL)return;
  DrawTxtShadow(name,x1,y, IndexColor(White), F_Small, 2);
  input = GetNthInput(name,0);
  if(input == NULL)return;
  if(GetInputText(input, text))
  {
    DrawTextCenteredShadow(text,x2,y, IndexColor(White), F_Small, 2);
  }
  input = GetNthInput(name,1);
  if(input == NULL)return;
  if(GetInputText(input, text))
  {
    DrawTextCenteredShadow(text,x3,y, IndexColor(White), F_Small, 2);
  }
}

void DrawControlConfig(int x,int x2, int  x3, int y)
{
  int i = 0;
  DrawControl("MoveUp", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
  DrawControl("MoveDown", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
  DrawControl("MoveLeft", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
  DrawControl("MoveRight", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
  DrawControl("LAction1", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
  DrawControl("LAction2", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
  DrawControl("RAction1", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
  DrawControl("RAction2", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
  DrawControl("LSwap", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
  DrawControl("RSwap", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
  DrawControl("NextSpell", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
  DrawControl("NextTool", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
  DrawControl("Target", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
  DrawControl("UseTool", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
  DrawControl("Run", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
  DrawControl("Interact", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
  DrawControl("SaveGame", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
  DrawControl("InventoryWindow", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
  DrawControl("StatusWindow", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
  DrawControl("SpellWindow", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
  DrawControl("SkillWindow", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
  DrawControl("ControlsWindow", x, x2, x3, y + (i++ * (GetFontHeight(F_Small) + 2)));
}

void DrawControlsWindow(HUDInfo *self)
{
  int x,x2,x3;
  x = self->box.x + (self->box.w*0.1);
  x2 = x + (GetFontHeight(F_Small) * 16);
  x3 = x  + (GetFontHeight(F_Small) * 24);
  DrawImageWindow(self->box.x,self->box.y,self->box.w,self->box.h);
  DrawTextCenteredShadow("Controls",self->box.x + self->box.w/2,self->box.y + 2,IndexColor(White), F_Medium, 2);
  DrawTextCenteredShadow("Primary",x2,self->box.y + 4 + GetFontHeight(F_Medium), IndexColor(Gold), F_Small, 2);
  DrawTextCenteredShadow("Alternate",x3,self->box.y + 4 + GetFontHeight(F_Medium), IndexColor(Gold), F_Small, 2);
  DrawControlConfig(x,x2,x3,self->box.y + 4 + GetFontHeight(F_Medium) + GetFontHeight(F_Medium));
}

int UpdateControlsWindow(HUDInfo *self,int pressID)
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

void ControlsWindow()
{
  HUDInfo *self;
  self = Pushwindow();
  if(self == NULL)
  {
    fprintf(stderr,"Big problem here, can't make Config window.\n");
    return;
  }
  Pause();
  self->stealinput = 1;
  self->windowupdate = UpdateControlsWindow;
  self->windowdraw = DrawControlsWindow;
  self->box.w = Camera.w*0.75;
  self->box.h = Camera.h*0.71;
  self->box.x = Camera.w/2 - self->box.w/2;
  self->box.y = Camera.h * 0.06;
  self->state = 0;
  ShowMouse();
  PicButton(self,0,SDLK_RETURN,"Done",self->box.x + self->box.w - (GetButtonW() + 4),self->box.y + self->box.h - GetButtonH() - 4,GetButtonW(),GetButtonH());
}

/*

  Control Functions

*/

Input *GetNthInput(char *name,int n)
{
  int i;
  int c = 0;
  for(i = 0; i < NumInputs;i++)
  {
    if(strcmp(name,InputList[i].name)==0)
    {
      if(c == n)
      {
        return &InputList[i];
      }
      else c++;
    }
  }
  return NULL;
}

int IsInputDoubleTapped(char *name)
{
  int i;
  for(i = 0;i < NumInputs;i++)
  {
    if(strcmp(name,InputList[i].name)==0)
    {
      if((InputList[i].oldvalue == 0)&&(InputList[i].value != 0))
      {
        if((InputList[i].TimePress - InputList[i].LastPress) < 130)
        {
          return 1;
        }
      }
    }
  }
  return 0;
}

int IsInputReleased(char *name)
{
  int i;
  for(i = 0;i < NumInputs;i++)
  {
    if(strcmp(name,InputList[i].name)==0)
    {
      if((InputList[i].oldvalue != 0)&&(InputList[i].value == 0))return 1;
    }
  }
  return 0;
}

int IsInputPressed(char *name)
{
  int i;
  for(i = 0;i < NumInputs;i++)
  {
    if(strcmp(name,InputList[i].name)==0)
    {
      if((InputList[i].oldvalue == 0)&&(InputList[i].value != 0))return 1;
    }
  }
  return 0;
}

int IsInputHeld(char *name)
{
  int i;
  for(i = 0;i < NumInputs;i++)
  {
    if(strcmp(name,InputList[i].name)==0)
    {
      if((InputList[i].oldvalue != 0)&&(InputList[i].value != 0))return 1;
    }
  }
  return 0;
}

int IsInputDown(char *name)
{
  int i;
  for(i = 0;i < NumInputs;i++)
  {
    if(strcmp(name,InputList[i].name)==0)
    {
      if(InputList[i].value != 0)return 1;
    }
  }
  return 0;
}

void UpdateControls()
{
  int i;
  Uint8 *keys = SDL_GetKeyState(NULL);
  SDLMod mod = SDL_GetModState();
  for(i = 0;i < NumInputs;i++)
  {
    InputList[i].oldvalue = InputList[i].value;
    if(strcmp(InputList[i].type,"key") == 0)
    {
      InputList[i].value = keys[InputList[i].id];
    }
    else if(strcmp(InputList[i].type,"mod") == 0)
    {
/*      fprintf(stdout,"%s: %i\n",InputList[i].name,InputList[i].id);
      fprintf(stdout,"mod: %i\n",mod);*/
      if(mod & InputList[i].id)
      {
        InputList[i].value = 1;
      }
      else InputList[i].value = 0;
    }
    /*joystick section*/
    if(InputList[i].value == 1)
    {
      InputList[i].LastPress = InputList[i].TimePress;
      InputList[i].TimePress = SDL_GetTicks();
    }
  }
}


/*

File Handles 

*/

int GetInputText(Input *input, char *text)
{
  if(input == NULL)return 0;
  if(text == NULL)return 0;
  if(strcmp(input->type,"mod")==0)
  {
    switch(input->id)
    {
      case  KMOD_LSHIFT:
        sprintf(text,"L Shift");
        return 1;
      case  KMOD_RSHIFT:
        sprintf(text,"R Shift");
        return 1;
      case  KMOD_LCTRL:
        sprintf(text,"L Ctrl");
        return 1;
      case  KMOD_RCTRL:
        sprintf(text,"R Ctrl");
        return 1;
      case  KMOD_LALT:
        sprintf(text,"L Alt");
        return 1;
      case  KMOD_RALT:
        sprintf(text,"R Alt");
        return 1;
    }
  }
  if(strcmp(input->type,"key")==0)
  {
    if((input->id >= SDLK_F1)&&(input->id <= SDLK_F12))
    {
      sprintf(text,"F%i",(input->id - SDLK_F1)+1);
      return 1;
    }
    if((input->id >= SDLK_a)&&(input->id <= SDLK_z))
    {
      sprintf(text,"%c",input->id - SDLK_a + 'a');
      return 1;
    }
    if((input->id >= SDLK_0)&&(input->id <= SDLK_9))
    {
      sprintf(text,"%i",input->id - SDLK_0);
      return 1;
    }
    if(input->id == SDLK_SLASH)
    {
      sprintf(text,"/");
      return 1;
    }
    if(input->id == SDLK_BACKSLASH)
    {
      sprintf(text,"\\");
      return 1;
    }
    if(input->id == SDLK_LEFTBRACKET)
    {
      sprintf(text,"[");
      return 1;
    }
    if(input->id == SDLK_RIGHTBRACKET)
    {
      sprintf(text,"]");
      return 1;
    }
    if(input->id == SDLK_MINUS)
    {
      sprintf(text,"-");
      return 1;
    }
    if(input->id == SDLK_EQUALS)
    {
      sprintf(text,"=");
      return 1;
    }
    if(input->id == SDLK_CARET)
    {
      sprintf(text,"`");
      return 1;
    }
    if(input->id == SDLK_COMMA)
    {
      sprintf(text,",");
      return 1;
    }
    if(input->id == SDLK_PERIOD)
    {
      sprintf(text,".");
      return 1;
    }
    if(input->id == SDLK_TAB)
    {
      sprintf(text,"TAB");
      return 1;
    }
    if(input->id == SDLK_ESCAPE)
    {
      sprintf(text,"ESC");
      return 1;
    }
    if(input->id == SDLK_SPACE)
    {
      sprintf(text,"SPACE");
      return 1;
    }
    if(input->id == SDLK_UP)
    {
      sprintf(text,"UP arrow");
      return 1;
    }
    if(input->id == SDLK_DOWN)
    {
      sprintf(text,"DOWN arrow");
      return 1;
    }
    if(input->id == SDLK_LEFT)
    {
      sprintf(text,"LEFT arrow");
      return 1;
    }
    if(input->id == SDLK_RIGHT)
    {
      sprintf(text,"RIGHT arrow");
      return 1;
    }
  }
  return 0;
}

int ParseInput(char *type, char *input)
{
  int i;
  if((type == NULL)||(input == NULL))return -1;
  if(strcmp(type,"key")==0)
  {
    if(input[0] == 'F')
    {
      sscanf(input,"F%i",&i);
      i--;
      return SDLK_F1 + i;
    }
    if((input[0] >= 'a')&&(input[0] <= 'z'))
    {
      return input[0]-'a' + SDLK_a;
    }
    if((input[0] >= '0')&&(input[0] <= '9'))
    {
      return input[0]-'0' + SDLK_0;
    }
    if(input[0] == '/')return SDLK_SLASH;
    if(input[0] == '\\')return SDLK_BACKSLASH;
    if(input[0] == '[')return SDLK_LEFTBRACKET;
    if(input[0] == ']')return SDLK_RIGHTBRACKET;
    if(input[0] == '-')return SDLK_MINUS;
    if(input[0] == '=')return SDLK_EQUALS;
    if(input[0] == '`')return SDLK_CARET;
    if(input[0] == ',')return SDLK_COMMA;
    if(input[0] == '.')return SDLK_PERIOD;
    if(strcmp(input,"TAB")==0)return SDLK_TAB;
    if(strcmp(input,"ESC")==0)return SDLK_ESCAPE;
    if(strcmp(input,"ENTER")==0)return SDLK_RETURN;
    if(strcmp(input,"SPACE")==0)return SDLK_SPACE;
    if(strcmp(input,"UPARROW")==0)return SDLK_UP;
    if(strcmp(input,"DOWNARROW")==0)return SDLK_DOWN;
    if(strcmp(input,"LEFTARROW")==0)return SDLK_LEFT;
    if(strcmp(input,"RIGHTARROW")==0)return SDLK_RIGHT;
  }
  if(strcmp(type,"mod")==0)
  {
    if(strcmp(input,"LShift")==0)
    {
      return KMOD_LSHIFT;
    }
    if(strcmp(input,"RShift")==0)
    {
      return KMOD_RSHIFT;
    }
    if(strcmp(input,"LCtrl")==0)
    {
      return KMOD_LCTRL;
    }
    if(strcmp(input,"RCtrl")==0)
    {
      return KMOD_RCTRL;
    }
    if(strcmp(input,"LAlt")==0)
    {
      return KMOD_LALT;
    }
    if(strcmp(input,"RAlt")==0)
    {
      return KMOD_RALT;
    }
  }
  if(strcmp(type,"button")==0)
  {
    sscanf(input,"%i",&i);
    return i;
  }
  if(strcmp(type,"+axis")==0)
  {
    sscanf(input,"%i",&i);
    return i;
  }
  if(strcmp(type,"-axis")==0)
  {
    sscanf(input,"%i",&i);
    return i;
  }
  return -1;
}

void LoadInputs()
{
  Sint64 size;
  char *buffer;
  char buf[512];
  char input[16];
  int s;
  PHYSFS_File *PSfile;
  FILE *file;
  PSfile = PHYSFS_openRead("system/controls.cfg");
  if(PSfile == NULL)
  {
    fprintf(stderr,"Unable to open control configuration file!\n");
    return;
  }
  size = PHYSFS_fileLength(PSfile);
  buffer = (char *)malloc(size);
  if(buffer == NULL)
  {
    fprintf(stderr,"Unable to allocate space for control configuration file\n");
    PHYSFS_close(PSfile);
    return;
  }
  NumInputs = 0;
  PHYSFS_read(PSfile, buffer, size, 1);
  file = fmemopen (buffer, size, "r");
  while(fscanf(file, "%s", buf) != EOF)
  {
    if(strcmp(buf,"#") ==0)
    {
      fgets(buf, sizeof(buf), file);
      continue;/*ignore the rest of the line.*/
    }
    if(strcmp(buf,"<input>") ==0)
    {
      fscanf(file, "%s %s %s",InputList[NumInputs].name,InputList[NumInputs].type,input);
      s = ParseInput(InputList[NumInputs].type,input);
      if(s != -1)
      {
        InputList[NumInputs].id = s;
        NumInputs++;
      }
      continue;
    }
    
  }
  fclose(file);
  PHYSFS_close(PSfile);
  free(buffer);
}

/*EOL@EOF*/

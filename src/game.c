#include <stdlib.h>
#include <chipmunk/chipmunk.h>
#include <physfs.h>
#include "graphics.h"
#include "modelLoader.h"
#include "menus.h"
#include "lighting.h"
#include "entity.h"
#include "audio.h"
#include "particle.h"
#include "controls.h"
#include "simple_logger.h"

Sprite *splash = NULL;
extern SDL_Rect Camera; /*x & y are the coordinates for the background map, w and h are of the screen*/

int windowx;
int windowy;
int windowedMode = 1;
float ScreenScaleFactor = 1;

extern Mouse_T Mouse;
extern ScreenData S_Data;
extern int FontSize[3];

void CameraUpdate3D();
void Init_All(const char *argv);

/*augmentation section*/
extern GLint viewport[4];
extern GLdouble modelview[16];
extern GLdouble projection[16];


int main(int argc, char *argv[])
{
  int done;
  int i;
  Uint8 *keys;
  SDL_Event event;
  for(i = 1;i < argc;i++)
  {
    if(strcmp(argv[i],"-fs")== 0)
    {
      windowedMode = 0;
    }
    if(strcmp(argv[i],"-model")==0)
    {
      saveBinaryModelFile(readOBJ(argv[i + 1]));
      exit(1);
    }else if(strcmp(argv[i],"-arm")==0)
    {
      transferArmatureFile(LoadArmatureObj(argv[i + 1]));
      exit(1);
    }
  }
  Init_All(argv[0]);
  done = 0;
  keys = SDL_GetKeyState(NULL);
  SetupBaseWindow();
  do
  {
    ResetBuffer();
    if(keys[SDLK_ESCAPE])
    {
      QuitCheck();
    }
    while(SDL_PeepEvents(&event, 1, SDL_PEEKEVENT, SDL_QUITMASK))
    {
      if(event.type == SDL_QUIT)
      {
        QuitCheck();
        break;
      }
    }
    glPushMatrix();
    updateLights();
    UpdateAllWindows();
    glPopMatrix();
    DrawAllWindows();
    DrawMessages();
    DrawMouse();
    NextFrame();
    updateKeyboard();
    while(SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_ALLEVENTS));
    SDL_PumpEvents();
    UpdateControls();
  }while(!done);
  exit(0);
  return 0;
}


void CleanUpAll()
{
  ClearEntities();
  CloseLights();
  CloseSprites();
  CloseModels();
  CloseZfmModels();
  CloseFonts();
  TTF_Quit();
  PHYSFS_deinit();
  /*any other cleanup functions can be added here*/ 
}

void Init_All(const char *argv)
{
  SDL_Surface *icon = NULL;
  init_logger();
  Init_Graphics(1024,768,windowedMode);
  PHYSFS_init(argv);
  PHYSFS_addToSearchPath("./", 1);
  PHYSFS_addToSearchPath("./system/pak0.pak", 1);
  ScreenScaleFactor = (float)Camera.w/1024.0;
  SDL_WM_SetCaption("Project Dark", NULL);
  InitGlFunctions();
  glEnable(GL_CULL_FACE);
  Init_Audio();
  cpInitChipmunk();
  InitSoundList();
  InitSpriteList();
  InitModelList();
  InitZfmModelList();
  InitKeyboard();
  InitEntList();
  InitLightList();
  icon = IMG_Load("images/UI/icon.png");
  if(icon != NULL)SDL_WM_SetIcon(icon, NULL);
  DrawSplashScreen();
  LoadFonts();
  LoadInputs();
  LoadHUD();
  InitMouse();
  InitWindowStack();
  InitMessages();
  MoveMessages(100,50,10);
  SetupDefaultLights();
  atexit(CleanUpAll);
}

void DrawSplashScreen()
{
  if(splash == NULL)splash = LoadSprite("images/UI/loading.png",1024,768);
  if(splash!=NULL)
  {
    DrawSprite(splash,0,0,0,1);
/*    DrawTextCentered("Loading...",Camera.w / 2 + 20,Camera.h * 0.8,IndexColor(DarkBlue),F_Large);*/
    NextFrame();
  }
}

/*eol @ eof*/

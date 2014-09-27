#ifndef __MENUS___
#define __MENUS___

#include "hud.h"
#include "audio.h"

/*Helper Functions*/

inline void Pause();
inline void UnPause();
inline int  IsPausing();

/*
  Specific menus for the project.  This file uses the window stack of hud.

*/

HUDInfo *SetupBaseWindow();

void TitleScreen();
void LoadGameConfig();
void TextBlockWindow(char *text);

#endif

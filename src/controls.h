#ifndef __CONTROLS__
#define __CONTROLS__
#include "graphics.h"

#define MAXINPUTS 32

typedef struct
{
  char name[80];  /*name of the input*/
  char type[16];  /*input type: key, button, axis*/
  Uint32 id;      /*which key or axis or button*/
  float value;    /*current state of the input*/
  float oldvalue; /*last state of the input*/
  Uint32 TimePress; /*this time index*/
  Uint32 LastPress; /*time index of the last time this input was pressed*/
}Input;

Input *GetNthInput(char *name,int n); /*gets the Nth occurance of the input in the input list*/
int GetInputText(Input *input, char *text);  /*returns 1 if it found the input text, 0 otherwise*/
int IsInputDown(char *name);
int IsInputHeld(char *name);      /*based on last time void UpdateControls() was called*/
int IsInputPressed(char *name);
int IsInputReleased(char *name);
int IsInputDoubleTapped(char *name);  /*if an input is pressed twice in close succession*/
void UpdateControls();        /*needs to be called after SDL Events are updated*/
int GetInput(char *input);    /*get current state of the input in question*/
void LoadInputs();            /*loads input config from file*/
void ControlsWindow();        /*menu to Show and change controls*/

#endif

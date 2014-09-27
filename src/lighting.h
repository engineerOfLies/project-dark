/*
    Copyright 2001, 2002, 2003, 2004, 2005, 2006 , 2007, 2008 Donald Kehoe, Anthony Rego
    This file is part of the Nanotech Game Engine.

    The Nanotech Game Engine is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The Nanotech Game Engine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with the Nanotech Game Engine.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __LIGHTING__
#define __LIGHTING__

#include "graphics.h" 

typedef struct Light_T
{
  float ambientLight[4];
  float diffuseLight[4];
  float specLight[4];
  float direction[3];
  float pos[4];
  float spotAngle;
  float spotEdge;
  int useRot;
  Coord rot;
  int used;

}GlLight;

void SetupDefaultLights();
void InitLightList();
void FreeLight(GlLight *light);
void CloseLights();
GlLight *NewLight();
void updateLights();
void drawLight(GlLight *light);

#endif

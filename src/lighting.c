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
#include "lighting.h"

GlLight LightList[GL_MAX_LIGHTS];
int NumLights;
GLuint ShaderProg = 0;

void SetupDefaultLights()
{
  GlLight *light;
  GlLight *light2;
  float fogcolor[4] = {0.75,.5,.4,1};
  ShaderProg = LoadShaderProgram("shaders/lighting1.vert","shaders/lighting1.frag");
  

   light=NewLight();
      light->ambientLight[0] =0.50f;
      light->ambientLight[1] =0.50f;
      light->ambientLight[2] =0.50f;
      light->ambientLight[3] =1.0f;
  
      light->diffuseLight[0] =1.0f;
      light->diffuseLight[1] =1.0f;
      light->diffuseLight[2] =1.0f;
      light->diffuseLight[3] =1.0f;
  
      light->specLight[0] =0.50f;
      light->specLight[1] =0.50f;
      light->specLight[2] =0.50f;
      light->specLight[3] =1.0f;
  
      light->direction[0] =1.0f;
      light->direction[1] =0.0f;
      light->direction[2] =0.0f;
      
      light->pos[0] =32.0f;
      light->pos[1] =15.0f;
      light->pos[2] =-32.0f;
      light->pos[3] =1.0f;
      light->spotEdge = 90.0f;
      light->spotAngle = 180.0f;
      light->useRot = 1;
      light->rot.x = 0;
      light->rot.y = 0;
      light->rot.z = 0;
      
      light2=NewLight();
      light2->ambientLight[0] =0.4f;
      light2->ambientLight[1] =0.4f;
      light2->ambientLight[2] =0.4f;
      light2->ambientLight[3] =1.0f;
  
      light2->diffuseLight[0] =1.0f;
      light2->diffuseLight[1] =1.0f;
      light2->diffuseLight[2] =1.0f;
      light2->diffuseLight[3] =1.0f;
  
      light2->specLight[0] =1.0f;
      light2->specLight[1] =1.0f;
      light2->specLight[2] =1.0f;
      light2->specLight[3] =1.0f;
  
      light2->direction[0] =1.0f;
      light2->direction[1] =0.0f;
      light2->direction[2] =0.0f;
      
      light2->pos[0] =32.0f;
      light2->pos[1] =590.0f;
      light2->pos[2] =-32.0f;
      light2->pos[3] =1.0f;
      light2->spotAngle =90.0f;
      light2->spotEdge = 180.0f;
      light2->useRot = 1;
      light2->rot.x = 0;
      light2->rot.y = 0;
      light2->rot.z = 180;


      
      glLightf(GL_LIGHT0,GL_CONSTANT_ATTENUATION, 0.0f);
      glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION, 0.02f);
      glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION, 0.000002f);
      glLightf(GL_LIGHT1,GL_CONSTANT_ATTENUATION, 0.001f);
      glLightf(GL_LIGHT1,GL_LINEAR_ATTENUATION, 0.02f);
      glLightf(GL_LIGHT1,GL_QUADRATIC_ATTENUATION, 0.000002f);
      glEnable(GL_LIGHT0);
      glEnable(GL_LIGHT1);
      glEnable(GL_FOG);
      glFogfv(GL_FOG_COLOR,fogcolor);
      glFogi(GL_FOG_MODE, GL_EXP);
      glFogf(GL_FOG_DENSITY, 0.001f);
      glFogf(GL_FOG_START, 1.0f);
      glFogf(GL_FOG_END, 2000.0f);
      /*
  light=NewLight();
  light->ambientLight[0] =0.7f;
  light->ambientLight[1] =0.7f;
  light->ambientLight[2] =0.7f;
  light->ambientLight[3] =1.0f;
  
  light->diffuseLight[0] =1.0f;
  light->diffuseLight[1] =1.0f;
  light->diffuseLight[2] =1.0f;
  light->diffuseLight[3] =1.0f;
  
  light->specLight[0] =1.0f;
  light->specLight[1] =1.0f;
  light->specLight[2] =1.0f;
  light->specLight[3] =1.0f;
  
  light->direction[0] =0.0f;
  light->direction[1] =0.0f;
  light->direction[2] =1.0f;
  
  light->pos[0] =32.0f;
  light->pos[1] =15.0f;
  light->pos[2] =32.0f;
  light->pos[3] =1.0f;
  light->spotEdge = 0.0f;
  light->spotAngle = 180.0f;
  light->useRot = 1;
  light->rot.x = 0;
  light->rot.y = 0;
  light->rot.z = 180;
  
  light2=NewLight();
  light2->ambientLight[0] =0.9f;
  light2->ambientLight[1] =0.9f;
  light2->ambientLight[2] =0.9f;
  light2->ambientLight[3] =1.0f;
  
  light2->diffuseLight[0] =1.0f;
  light2->diffuseLight[1] =1.0f;
  light2->diffuseLight[2] =1.0f;
  light2->diffuseLight[3] =1.0f;
  
  light2->specLight[0] =1.0f;
  light2->specLight[1] =1.0f;
  light2->specLight[2] =1.0f;
  light2->specLight[3] =1.0f;
  
  light2->direction[0] =0.0f;
  light2->direction[1] =0.0f;
  light2->direction[2] =1.0f;
  
  light2->pos[0] =0.0f;
  light2->pos[1] =0.0f;
  light2->pos[2] =-32.0f;
  light2->pos[3] =1.0f;
  light2->spotAngle =0.0f;
  light2->spotEdge = 90.0f;
  light2->useRot = 1;
  light2->rot.x = 0;
  light2->rot.y = 0;
  light2->rot.z = 0;*/
  /*
  light=NewLight();
  light->ambientLight[0] =1.0f;
  light->ambientLight[1] =1.0f;
  light->ambientLight[2] =1.0f;
  light->ambientLight[3] =1.0f;
  
  light->diffuseLight[0] =1.0f;
  light->diffuseLight[1] =1.0f;
  light->diffuseLight[2] =1.0f;
  light->diffuseLight[3] =1.0f;
  
  light->specLight[0] =1.0f;
  light->specLight[1] =1.0f;
  light->specLight[2] =1.0f;
  light->specLight[3] =1.0f;
  
  light->direction[0] =0.0f;
  light->direction[1] =0.0f;
  light->direction[2] =1.0f;
  
  light->pos[0] =0.0f;
  light->pos[1] =0.0f;
  light->pos[2] =-32.0f;
  light->pos[3] =0.0f;
  light->spotEdge = 1.0f;
  light->spotAngle = 45.0f;
  light->useRot = 1;
  light->rot.x = 0;
  light->rot.y = 0;
  light->rot.z = 0;
  
  */
/*  glLightf(GL_LIGHT0,GL_CONSTANT_ATTENUATION, 0.0f);
  glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION, 0.02f);
  glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION, 0.000002f);
  glLightf(GL_LIGHT1,GL_CONSTANT_ATTENUATION, 0.001f);
  glLightf(GL_LIGHT1,GL_LINEAR_ATTENUATION, 0.02f);
  glLightf(GL_LIGHT1,GL_QUADRATIC_ATTENUATION, 0.000002f);
  glLightf(GL_LIGHT2,GL_CONSTANT_ATTENUATION, 0.001f);
  glLightf(GL_LIGHT2,GL_LINEAR_ATTENUATION, 0.02f);
  glLightf(GL_LIGHT2,GL_QUADRATIC_ATTENUATION, 0.000002f);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
//  glEnable(GL_LIGHT2);
  glEnable(GL_FOG);
  glFogfv(GL_FOG_COLOR,fogcolor);
  glFogi(GL_FOG_MODE, GL_EXP);
  glFogf(GL_FOG_DENSITY, 0.001f);
  glFogf(GL_FOG_START, 0.8f);
  glFogf(GL_FOG_END, 2000.0f);*/

}

void InitLightList()
{
  NumLights = 0;
  memset(LightList,0,sizeof(GlLight) * GL_MAX_LIGHTS);
}

void FreeLight(GlLight *light)
{
  NumLights--;
  light->used--;
}

void CloseLights()
{
  int i;
  for(i = 0;i < GL_MAX_LIGHTS;i++)
  {
    FreeLight(&LightList[i]);
  }
}

GlLight *NewLight()
{
  int i;
  if(NumLights + 1 >= GL_MAX_LIGHTS)
  {
    return NULL;
  }
  NumLights++;
  for(i = 0;i < GL_MAX_LIGHTS;i++)
  {
    if(!LightList[i].used)break;
  }
  LightList[i].used = 1;
  
  LightList[i].ambientLight[0] =0.0f;
  LightList[i].ambientLight[1] =0.0f;
  LightList[i].ambientLight[2] =0.0f;
  LightList[i].ambientLight[3] =1.0f;
  
  LightList[i].diffuseLight[0] =0.0f;
  LightList[i].diffuseLight[1] =0.0f;
  LightList[i].diffuseLight[2] =0.0f;
  LightList[i].diffuseLight[3] =1.0f;
  
  LightList[i].specLight[0] =0.0f;
  LightList[i].specLight[1] =0.0f;
  LightList[i].specLight[2] =0.0f;
  LightList[i].specLight[3] =1.0f;
  
  LightList[i].direction[0] =0.0f;
  LightList[i].direction[1] =0.0f;
  LightList[i].direction[2] =-1.0f;

  LightList[i].pos[0] =0.0f;
  LightList[i].pos[1] =0.0f;
  LightList[i].pos[2] =1.0f;
  LightList[i].pos[3] =0.0f;
  LightList[i].spotEdge = 0;
  LightList[i].spotAngle= 180;
  
  return &LightList[i];
}

void updateLights()
{
  int i;
  int done;
  
  done =0;
  for (i = 0 ;i < GL_MAX_LIGHTS  ;i++)
  {
    if(done >= NumLights) break;
    if (LightList[i].used != 0)
    {
      done++;
      glPushMatrix();
      if(LightList[i].useRot==1)
      {
        glTranslatef(LightList[i].pos[0],LightList[i].pos[1],LightList[i].pos[2]);
        glRotatef(LightList[i].rot.x, 1.0f, 0.0f, 0.0f);
        glRotatef(LightList[i].rot.y, 0.0f, 1.0f, 0.0f);
        glRotatef(LightList[i].rot.z, 0.0f, 0.0f, 1.0f);
        glTranslatef(-LightList[i].pos[0],-LightList[i].pos[1],-LightList[i].pos[2]);

      }
      glLightfv(GL_LIGHT0+i,GL_SPECULAR,LightList[i].specLight);
      glLightfv(GL_LIGHT0+i,GL_AMBIENT,LightList[i].ambientLight);
      glLightfv(GL_LIGHT0+i,GL_DIFFUSE,LightList[i].diffuseLight);   
      glLightfv(GL_LIGHT0+i,GL_SPOT_DIRECTION,LightList[i].direction);
      glLightfv(GL_LIGHT0+i,GL_POSITION,LightList[i].pos);
      glLightf(GL_LIGHT0+i,GL_SPOT_EXPONENT,LightList[i].spotEdge);	
      glLightf(GL_LIGHT0+i,GL_SPOT_CUTOFF,LightList[i].spotAngle);
      glPopMatrix();
    }
  }
  
}
void drawLight(GlLight *light)
{
  drawBox(light->pos[0],light->pos[1],light->pos[2]);
  DrawLaser(light->pos[0],light->pos[1],light->pos[2],light->rot.x,light->rot.y,light->rot.z,1,0,0,5);
  DrawLaser(light->pos[0],light->pos[1],light->pos[2],light->rot.x,light->rot.y,light->rot.z+light->spotAngle,0,1,0,10);
  DrawLaser(light->pos[0],light->pos[1],light->pos[2],light->rot.x,light->rot.y,light->rot.z-light->spotAngle,0,1,0,10);
  DrawLaser(light->pos[0],light->pos[1],light->pos[2],light->rot.x,light->rot.y+light->spotAngle,light->rot.z,0,1,0,10);
  DrawLaser(light->pos[0],light->pos[1],light->pos[2],light->rot.x,light->rot.y-light->spotAngle,light->rot.z,0,1,0,10);
}


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
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "graphics.h"
#include <SDL_getenv.h>
#include <physfs.h>

#undef NO_STDIO_REDIRECT

#define MaxSprites    255

enum BoxSide {SB_Left,SB_Front,SB_Right,SB_Back,SB_Top,SB_Bottom};
Sprite *skyboxtextures[6];
SDL_Surface *videobuffer;
Sprite *mouse;
SDL_Rect Camera; /*x & y are the coordinates for the background map, w and h are of the videobuffer*/
glCamera_T glCamera;
Sprite SpriteList[MaxSprites];
SDL_PixelFormat *pixelFormat;
Uint32 NOW;  /*this represents the current time for the game loop.  Things move according to time*/
Uint32 DELAY; /*time between frames*/
GLint viewport[4];
GLdouble modelview[16];
GLdouble projection[16];
int OpenGLOn = 0;
int NumSprites;
int NumModels;
int capableResolutions[10][2];
int numcapable;
/*some data on the video settings that can be useful for a lot of functions*/
Uint32 rmask,gmask,bmask,amask;
ScreenData  S_Data;


void Init_Graphics(int x, int y, int windowed)
{
  Uint32 Vflags = SDL_ANYFORMAT | SDL_SRCALPHA;
    Uint32 HWflag = 0;

    S_Data.xres = x;
    S_Data.yres = y;
    if(!windowed)Vflags |= SDL_FULLSCREEN;
      Vflags |= SDL_OPENGL;
      OpenGLOn = 1;

    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
    #else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
    #endif

    #if defined(_WIN32)
    stream = freopen("stdout.txt", "w", stdout);
    stream = freopen("stderr.txt", "w", stderr);
    #endif


    if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0 )
      {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
      }

    else if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_DOUBLEBUF) < 0 )
    {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);
    numcapable=0;
    if(SDL_VideoModeOK(x,y, 32, Vflags | SDL_HWSURFACE))
    {
      capableResolutions[numcapable][0]=x;
      capableResolutions[numcapable][1]=y;
      numcapable++;
    }
    if(SDL_VideoModeOK(x, y, 32, Vflags | SDL_HWSURFACE))
    {
      S_Data.xres = x;
      S_Data.yres = y;
        S_Data.depth = 32;
        HWflag = SDL_HWSURFACE;

          SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
          SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
          SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
          SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
          SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    }
    else if(SDL_VideoModeOK(x + 6, y, 32, Vflags | SDL_HWSURFACE))
    {
      S_Data.xres = x + 6;
      S_Data.yres = y;
        S_Data.depth = 32;
        HWflag = SDL_HWSURFACE;

          SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
          SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
          SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
          SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
          SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    }
    else if(SDL_VideoModeOK(x, y, 32, Vflags))
    {
      S_Data.xres = x;
      S_Data.yres = y;
      S_Data.depth = 32;
      
      SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    }
    else if(SDL_VideoModeOK(x + 6, y, 32, Vflags))
    {
      S_Data.xres = x + 6;
      S_Data.yres = y;
      S_Data.depth = 32;
      
      SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    }
    else if(SDL_VideoModeOK(x + 6, y, 32, Vflags))
    {
      S_Data.xres = x + 6;
      S_Data.yres = y;
      S_Data.depth = 32;
      Vflags &= ~SDL_FULLSCREEN;
      SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    }
    else
    {
        fprintf(stderr, "Unable to use your screen: %s\n Deepest appologies \n", SDL_GetError());
        exit(1);
    }
    videobuffer = SDL_SetVideoMode(S_Data.xres, S_Data.yres,S_Data.depth, Vflags | HWflag);
    if ( videobuffer == NULL )
    {
        fprintf(stderr, "Unable to set video: %s\n", SDL_GetError());
        exit(1);
    }
    pixelFormat = videobuffer->format;
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY*2,SDL_DEFAULT_REPEAT_INTERVAL);
    SDL_ShowCursor(SDL_DISABLE);
    Camera.x = 0;
    Camera.y = 0;
    Camera.w = S_Data.xres;
    Camera.h = S_Data.yres;
      glCamera.position.x = 0.0f;
      glCamera.position.y = -10.0f;
      glCamera.position.z = -5.0f;
      glCamera.rotation.x = 0.0f;
      glCamera.rotation.y = 0.0f;
      glCamera.rotation.z = 0.0f;
      SDL_JoystickEventState(SDL_ENABLE);
      /*sets the viewing port to the size of the videobuffer*/
      glViewport(0,0,S_Data.xres, S_Data.yres);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      /*view angle, aspect ratio, near clip distance, far clip distance*/
      gluPerspective( 40, (float)x / (float)y, .01, 2000.0f);
      /**gluOrtho2D(0,S_Data.xres,S_Data.yres,0);*/
      glMatrixMode( GL_MODELVIEW );
      glLoadIdentity();
      /*Clear color for the buffer*/
      glClearColor(0,0,0,0);
      /*Enables drawing pixels according to thier depth*/
      glDepthFunc(GL_LESS);
      glEnable(GL_DEPTH_TEST);
      /*Enables alpha testing*/
      glAlphaFunc(GL_GREATER,0);
      glEnable(GL_ALPHA_TEST);
      glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
      glGetIntegerv(GL_VIEWPORT,viewport);
      glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
      glGetDoublev(GL_PROJECTION_MATRIX, projection);
      
      glLineWidth(2);

    srand(SDL_GetTicks());
}

void ResetBuffer()
{
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void NextFrame()
{
  char text[80];
  Uint32 Then;

  SDL_GL_SwapBuffers(); 
  FrameDelay(33);
  Then = NOW;
  NOW = SDL_GetTicks();
  DELAY = NOW- Then;
  sprintf(text,"Ticks passed this frame: %i", NOW - Then);
  #if 0
  NewMessage(text,IndexColor(White));
  fprintf(stdout,"Ticks passed this frame: %i\n", NOW - Then);
  fprintf(stdout,"FPS: %f\n", 1000.0 /(NOW - Then));
  #endif
}

/*
  InitSpriteList is called when the program is first started.
  It just sets everything to zero and sets all pointers to NULL.
  It should never be called again.
*/

void InitSpriteList()
{
  int x;
  NumSprites = 0;
  memset(SpriteList,0,sizeof(Sprite) * MaxSprites);
  for(x = 0;x < MaxSprites;x++)SpriteList[x].surface = NULL;
}

float CoordLength(Coord v)
{
  return sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

Coord M_Coord(float x, float y, float z)
{
  Coord m;
  m.x = x;
  m.y = y;
  m.z = z;
  return m;
}

/*Utility function that adds a new empty sprite entry and returns a pointer to it.  The sprite is empty and will not work yet.*/
Sprite *NewSprite()
{
  int i;
  /*makesure we have the room for a new sprite*/
  if(NumSprites + 1 >= MaxSprites)
  {
    fprintf(stderr, "Maximum Sprites Reached.\n");
    return NULL;
  }
  for(i = 0;i < MaxSprites;i++)
  {
    if(!SpriteList[i].used)break;
  }
  SpriteList[i].used = 1;
  /*if its not already in memory, then load it.*/
  NumSprites++;
  return &SpriteList[i];
}

Sprite *LoadSprite( char *filename, int sizex, int sizey )
{
  return LoadSwappedSprite( filename, sizex, sizey, -1, -1, -1, 0);
}

Sprite *LoadSwappedSprite( char *filename, int sizex, int sizey, int c1, int c2, int c3 , int mipmapping)
{
  int i;
  int k,j;
  SDL_Surface *temp;
  SDL_Surface *temp1;
  int w, h;
  int n;
  Uint32 clearColor;
  Uint8 r,g,b,a;
  /*first search to see if the requested sprite image is alreday loaded*/
  for(i = 0; i < MaxSprites; i++)
  {
    if((strncmp(filename,SpriteList[i].filename,80)==0)&&(SpriteList[i].loaded == 1)&&(sizex == SpriteList[i].w)&&(sizey == SpriteList[i].h))
    {
      SpriteList[i].used++;
      return &SpriteList[i];
    }
  }
  /*makesure we have the room for a new sprite*/
  /*if its not already in memory, then load it.*/
  n = -1;
  for(i = 0;i < MaxSprites;i++)
  {
    if(!SpriteList[i].loaded)
    {
      n = i;
      break;
    }
  }
  /*if  every slot has been loaded, find one that is no longer in use.*/
  if(n == -1)
  {
    for(i = 0;i < MaxSprites;i++)
    {
      if(SpriteList[i].used <= 0)
      {
        n = i;
        break;
      }
    }
    if(n == -1)
    {
      fprintf(stderr,"Ran out of places for sprites!\n");
      return NULL;
    }
    strcpy(SpriteList[n].filename,"\0");
    if(SpriteList[n].surface != NULL)SDL_FreeSurface(SpriteList[n].surface);
    SpriteList[n].surface = NULL;
    if(glIsTexture(SpriteList[n].image) == GL_TRUE)glDeleteTextures(1, &SpriteList[n].image);
  }
  temp = Load_Image(filename);
  /*temp = IMG_Load(filename);
  */
  if(temp == NULL)
  {
    fprintf(stderr,"unable to load a vital sprite: %s\n",SDL_GetError());
    return NULL;
  }
  temp1 = SDL_DisplayFormatAlpha(temp);
  strncpy(SpriteList[i].filename,filename,80);

  SpriteList[i].framesperline = 16;
  /*makes sure the width and height of each frame is opengl compatable*/
  if(sizex != -1)
  {
    SpriteList[i].w = sizex;
    SpriteList[i].h = sizey;
  }
  else
  {
    SpriteList[i].w = temp1->w;
    SpriteList[i].h = temp1->h;
  }
  SpriteList[i].used++;
  SpriteList[i].loaded = 1;
  /*makes sure the width and height of the entire surface is opengl compatable*/
  SpriteList[i].imageW = w = temp1->w;
  SpriteList[i].imageH = h = temp1->h;
  SpriteList[i].texHeight= (float)SpriteList[i].h/(float)h;
  SpriteList[i].texWidth = (float)SpriteList[i].w/(float)w;
  SpriteList[i].w2 = (float)SpriteList[i].w/(float)w;
  SpriteList[i].h2 = (float)SpriteList[i].h/(float)h;
  SpriteList[i].dimen = GetGlCoord((Camera.w>>1)+SpriteList[i].w,(Camera.h>>1)+SpriteList[i].h,_2DPLANE_,modelview,projection,viewport);
  /*Creates an opengl compatable RGBA surface*/
  SpriteList[i].surface = SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCALPHA,w, h,S_Data.depth,rmask,gmask,bmask,amask);
  /*Sets the clear color on the surface*/
  clearColor = SDL_MapRGBA(SpriteList[i].surface->format, 0, 0, 0,255);
  /*This goes through the pixel data copying it and checking each pixel for the clear color, when found sets the alpha to 0*/
  for(k=0;k < SpriteList[i].surface->h; k++)
  {
    for(j=0;j < SpriteList[i].surface->w; j++)
    {
        SDL_GetRGBA(getpixel(temp1, j, k), temp1->format, &r, &g, &b, &a);
        putpixel(SpriteList[i].surface, j, k, SDL_MapRGBA(SpriteList[i].surface->format, r, g, b, a));

    }
  }

  SpriteList[i].color1 = c1;
  SpriteList[i].color2 = c2;
  SpriteList[i].color3 = c3;
  SwapSprite(SpriteList[i].surface,c1,c2,c3);
  glGenTextures(1, &SpriteList[i].image);
  glBindTexture(GL_TEXTURE_2D,SpriteList[i].image);
  if(mipmapping == 1)
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, w, h, GL_RGBA, GL_UNSIGNED_BYTE, SpriteList[i].surface->pixels);
  }else
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, SpriteList[i].surface->pixels);
  }
  SDL_FreeSurface(SpriteList[i].surface);
  SpriteList[i].surface = NULL;
  SDL_FreeSurface(temp1);
  temp1 = NULL;
  SDL_FreeSurface(temp);
  temp = NULL;
  return &SpriteList[i];
}

void DrawSprite( Sprite *sprite, int sx, int sy, int frame, float scale )
{
  DrawSpriteStretch( sprite, sx, sy, frame, scale,scale );
}


void DrawSpriteStretchRot( Sprite *sprite, int sx, int sy, int frame, float scx,float scy, float rot, float alpha )
{
    Rectf src;
  Coord pos;

  if(sprite == NULL)
  {
    return;
  }
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_COLOR_MATERIAL);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glColor4f(1,1,1,alpha);

  src.x = frame%sprite->framesperline * sprite->w2;
  src.y = frame/sprite->framesperline * sprite->h2;
  src.w = sprite->w2+src.x;
  src.h = sprite->h2+src.y;

  pos = GetGlCoord(sx,sy,_2DPLANE_,modelview,projection,viewport);

  glBindTexture(GL_TEXTURE_2D,sprite->image);

  glPushMatrix();
  glTranslatef(pos.x,pos.y,pos.z);
  glRotatef(rot, 0.0f, 0.0f, 1.0f);
  glScalef(scx,scy,1);
  glTranslatef(sprite->dimen.x*0.5f,sprite->dimen.y*0.5f,0.0f);
  glTranslatef(sprite->dimen.x*-0.5f,sprite->dimen.y*-0.5f,0.0f);
  glBegin( GL_QUADS );

  glTexCoord2f(src.x,src.y);
  glVertex3f(-sprite->dimen.x/2,-sprite->dimen.y/2,0.0f);

  glTexCoord2f(src.x,src.h);
  glVertex3f(-sprite->dimen.x/2,sprite->dimen.y/2,0.0f);

  glTexCoord2f(src.w,src.h);
  glVertex3f(sprite->dimen.x/2,sprite->dimen.y/2,0.0f);

  glTexCoord2f(src.w,src.y);
  glVertex3f(sprite->dimen.x/2,-sprite->dimen.y/2,0.0f);

  glEnd( );
  glPopMatrix();
  glColor4f(1,1,1,1);
  glDisable(GL_BLEND);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);

}

void DrawSpriteAlpha( Sprite *sprite, int sx, int sy, int frame, float scx,float scy, float rot, float alpha )
{
  Rectf src;
  Coord pos;
  
  if(sprite == NULL)
  {
    return;
  }
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_COLOR_MATERIAL);
  glBlendFunc(GL_ZERO,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glColor4f(0,0,0,alpha);
  
  src.x = frame%sprite->framesperline * sprite->w2;
  src.y = frame/sprite->framesperline * sprite->h2;
  src.w = sprite->w2+src.x;
  src.h = sprite->h2+src.y;
  
  pos = GetGlCoord(sx,sy,_2DPLANE_,modelview,projection,viewport);
  
  glBindTexture(GL_TEXTURE_2D,sprite->image);
  
  glPushMatrix();
  glTranslatef(pos.x,pos.y,pos.z);
  glRotatef(rot, 0.0f, 0.0f, 1.0f);
  glScalef(scx,scy,1);
  glTranslatef(sprite->dimen.x*0.5f,sprite->dimen.y*0.5f,0.0f);
  glTranslatef(sprite->dimen.x*-0.5f,sprite->dimen.y*-0.5f,0.0f);
  glBegin( GL_QUADS );
  
  glTexCoord2f(src.x,src.y);
  glVertex3f(-sprite->dimen.x/2,-sprite->dimen.y/2,0.0f);
  
  glTexCoord2f(src.x,src.h);
  glVertex3f(-sprite->dimen.x/2,sprite->dimen.y/2,0.0f);
  
  glTexCoord2f(src.w,src.h);
  glVertex3f(sprite->dimen.x/2,sprite->dimen.y/2,0.0f);
  
  glTexCoord2f(src.w,src.y);
  glVertex3f(sprite->dimen.x/2,-sprite->dimen.y/2,0.0f);
  
  glEnd( );
  glPopMatrix();
  glColor4f(1,1,1,1);
  glDisable(GL_BLEND);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  
}


void DrawSpriteStretch( Sprite *sprite, int sx, int sy, int frame, float scx,float scy )
{
  Rectf src;
  Coord pos;
  
  if(sprite == NULL)
  {
    return;
  }
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_COLOR_MATERIAL);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  
  src.x = frame%sprite->framesperline * sprite->w2;
  src.y = frame/sprite->framesperline * sprite->h2;
  src.w = sprite->w2+src.x;
  src.h = sprite->h2+src.y;
  
  pos = GetGlCoord(sx,sy,_2DPLANE_,modelview,projection,viewport);
  
  glBindTexture(GL_TEXTURE_2D,sprite->image);
  
  glPushMatrix();
  
  glTranslatef(pos.x,pos.y,pos.z);
  glScalef(scx,scy,1);
  glTranslatef(sprite->dimen.x*0.5f,sprite->dimen.y*0.5f,0.0f);
  glTranslatef(sprite->dimen.x*-0.5f,sprite->dimen.y*-0.5f,0.0f);
  glBegin( GL_QUADS );
  
  glTexCoord2f(src.x,src.y);
  glVertex3f(0.0f,0.0f,0.0f);
  
  glTexCoord2f(src.x,src.h);
  glVertex3f(0.0f,sprite->dimen.y,0.0f);
  
  glTexCoord2f(src.w,src.h);
  glVertex3f(sprite->dimen.x,sprite->dimen.y,0.0f);
  
  glTexCoord2f(src.w,src.y);
  glVertex3f(sprite->dimen.x,0.0f,0.0f);
  
  glEnd( );
  glPopMatrix();
  glDisable(GL_BLEND);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  
}

void DrawSpriteCropped( Sprite *sprite, int sx, int sy,int cx,int cy,int cw,int ch, int frame, float scx,float scy )
{
  Rectf src;
  Coord pos;

  if(sprite == NULL)
  {
    return;
  }
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_COLOR_MATERIAL);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  src.x = (frame%sprite->framesperline * sprite->w2) + (cx / (float)sprite->imageW);
  src.y = (frame/sprite->framesperline * sprite->h2) + (cy / (float)sprite->imageH);
  src.w = sprite->w2+src.x - (cw / (float)sprite->imageW);
  src.h = sprite->h2+src.y - (ch / (float)sprite->imageH);

  pos = GetGlCoord(sx,sy,_2DPLANE_,modelview,projection,viewport);

  glBindTexture(GL_TEXTURE_2D,sprite->image);

  glPushMatrix();

  sprite->dimen = GetGlCoord((Camera.w>>1)+sprite->w - (cx + cw),(Camera.h>>1)+sprite->h - (cy + ch),_2DPLANE_,modelview,projection,viewport);

  glTranslatef(pos.x,pos.y,pos.z);
  glScalef(scx,scy,1);
  glTranslatef(sprite->dimen.x*0.5f,sprite->dimen.y*0.5f,0.0f);
  glTranslatef(sprite->dimen.x*-0.5f,sprite->dimen.y*-0.5f,0.0f);
  glBegin( GL_QUADS );

  glTexCoord2f(src.x,src.y);
  glVertex3f(0.0f,0.0f,0.0f);

  glTexCoord2f(src.x,src.h);
  glVertex3f(0.0f,sprite->dimen.y,0.0f);

  glTexCoord2f(src.w,src.h);
  glVertex3f(sprite->dimen.x,sprite->dimen.y,0.0f);

  glTexCoord2f(src.w,src.y);
  glVertex3f(sprite->dimen.x,0.0f,0.0f);

  glEnd( );
  glPopMatrix();
  sprite->w2 = sprite->texWidth;
  sprite->h2 = sprite->texHeight;
  sprite->dimen = GetGlCoord((Camera.w>>1)+sprite->w,(Camera.h>>1)+sprite->h,_2DPLANE_,modelview,projection,viewport);

  glDisable(GL_BLEND);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
}


void DrawSprite2( Sprite *sprite, int sx, int sy, int frame, float scale )
{
  Rectf src;
  Coord pos;

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_COLOR_MATERIAL);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  src.x = frame%sprite->framesperline * sprite->w2;
  src.y = frame/sprite->framesperline * sprite->h2;
  src.w = sprite->w2+src.x;
  src.h = sprite->h2+src.y;

  pos = GetGlCoord(sx,sy,_2DPLANE_,modelview,projection,viewport);

  glBindTexture(GL_TEXTURE_2D,sprite->image);

  glPushMatrix();


  glTranslatef(pos.x,pos.y,pos.z);
  glScalef(scale,scale,scale);
  glBegin( GL_QUADS );

  glTexCoord2f(src.x,src.y);
  glVertex3f(0.0f,0.0f,0.0f);

  glTexCoord2f(src.x,src.h);
  glVertex3f(0.0f,sprite->dimen.y,0.0f);

  glTexCoord2f(src.w,src.h);
  glVertex3f(sprite->dimen.x,sprite->dimen.y,0.0f);

  glTexCoord2f(src.w,src.y);
  glVertex3f(sprite->dimen.x,0.0f,0.0f);

  glEnd( );
  glPopMatrix();

  glDisable(GL_BLEND);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_TEXTURE_2D);
}

void DrawSprite3D(Sprite *sprite, Coord pos, Coord scale, int frame,float alpha)
{
  Rectf src;

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_COLOR_MATERIAL);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glColor4f(1,1,1,alpha);
  src.x = frame%sprite->framesperline * sprite->texWidth;
  src.y = frame/sprite->framesperline * sprite->texHeight;
  src.w = sprite->texWidth+src.x;
  src.h = sprite->texHeight+src.y;

  glBindTexture(GL_TEXTURE_2D,sprite->image);

  glPushMatrix();

  glTranslatef(pos.x,pos.y,pos.z);
  glTranslatef(-sprite->dimen.x*0.5f*scale.x,-sprite->dimen.y*0.5f*scale.y,0.0f);
/*  glRotatef(-glCamera.rotation.z, 0.0f, 0.0f, 1.0f);
  glRotatef(-glCamera.rotation.x, 1.0f, 0.0f, 0.0f);
  glRotatef(-glCamera.rotation.y, 0.0f, 1.0f, 0.0f);*/
  glScalef(scale.x,scale.y,scale.z);
/* glTranslatef(sprite->dimen.x*-0.5f,sprite->dimen.y*-0.5f,0.0f);
*/
  glBegin( GL_QUADS );

  glTexCoord2f(src.x,src.y);
  glVertex3f(0.0f,0.0f,0.0f);

  glTexCoord2f(src.x,src.h);
  glVertex3f(0.0f,sprite->dimen.y,0.0f);

  glTexCoord2f(src.w,src.h);
  glVertex3f(sprite->dimen.x,sprite->dimen.y,0.0f);

  glTexCoord2f(src.w,src.y);
  glVertex3f(sprite->dimen.x,0.0f,0.0f);

  glEnd( );
  glPopMatrix();
  glColor4f(1,1,1,1);
  glDisable(GL_BLEND);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_TEXTURE_2D);
}

void DrawSpriteBG(Sprite *sprite, int sx, int sy, int frame)
{
  Rectf src;
  Coord pos;

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_COLOR_MATERIAL);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  src.x = frame%sprite->framesperline * sprite->texWidth;
  src.y = frame/sprite->framesperline * sprite->texHeight;
  src.w = sprite->texWidth+src.x;
  src.h = sprite->texHeight+src.y;

  pos = GetGlCoord(sx,sy,_2DPLANE_,modelview,projection,viewport);

  glBindTexture(GL_TEXTURE_2D,sprite->image);

  glPushMatrix();


  glTranslatef(pos.x,pos.y,pos.z);

  glBegin( GL_QUADS );

  glTexCoord2f(src.x,src.y);
  glVertex3f(0.0f,0.0f,0.0f);

  glTexCoord2f(src.x,src.h);
  glVertex3f(0.0f,sprite->dimen.y,0.0f);

  glTexCoord2f(src.w,src.h);
  glVertex3f(sprite->dimen.x,sprite->dimen.y,0.0f);

  glTexCoord2f(src.w,src.y);
  glVertex3f(sprite->dimen.x,0.0f,0.0f);

  glEnd( );
  glPopMatrix();

  glDisable(GL_BLEND);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_TEXTURE_2D);

}

void DrawSpritScaledRotated(Sprite *sprite,int sx,int sy, int frame, float scale, float rotx, float roty, float rotz)
{

  Rectf src;
  Coord pos;

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_COLOR_MATERIAL);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  src.x = frame%sprite->framesperline * sprite->texWidth;
  src.y = frame/sprite->framesperline * sprite->texHeight;
  src.w = sprite->texWidth+src.x;
  src.h = sprite->texHeight+src.y;

  pos = GetGlCoord(sx,sy,_2DPLANE_,modelview,projection,viewport);

  glBindTexture(GL_TEXTURE_2D,sprite->image);

  glPushMatrix();
  glTranslatef(pos.x,pos.y,pos.z); 
  glRotatef(rotx, 1.0f, 0.0f, 0.0f);
  glRotatef(roty, 0.0f, 1.0f, 0.0f);
  glRotatef(rotz, 0.0f, 0.0f, 1.0f);
  glScalef(scale,scale,1.0f);

  glBegin( GL_QUADS );

  glTexCoord2f(src.x,src.y);
  glVertex3f(0.0f,0.0f,0.0f);

  glTexCoord2f(src.x,src.h);
  glVertex3f(0.0f,sprite->dimen.y,0.0f);

  glTexCoord2f(src.w,src.h);
  glVertex3f(sprite->dimen.x,sprite->dimen.y,0.0f);

  glTexCoord2f(src.w,src.y);
  glVertex3f(sprite->dimen.x,0.0f,0.0f);

  glEnd( );

  glPopMatrix();
  glDisable(GL_BLEND);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_TEXTURE_2D);
}

/*
 * When we are done with a sprite, lets give the resources back to the system...
 * so we can get them again later.
 */

void OldFreeSprite(Sprite *sprite)
{
  /*first lets check to see if the sprite is still being used.*/
  sprite->used--;
  if(sprite->used == 0)
  {
  strcpy(sprite->filename,"\0");
     /*just to be anal retentive, check to see if the image is already freed*/
  if(sprite->surface != NULL)SDL_FreeSurface(sprite->surface);
  sprite->surface = NULL;
  }
 /*and then lets make sure we don't leave any potential seg faults 
  lying around*/
}

void FreeSprite(Sprite *sprite)
{
  /*first lets check to see if the sprite is still being used.*/
  sprite->used--;
 /* if(sprite->used <= 0)
  {
    strcpy(sprite->filename,"\0");
    NumSprites--;
    if(sprite->surface != NULL)SDL_FreeSurface(sprite->surface);
    sprite->surface = NULL;
    if(glIsTexture(sprite->image) == GL_TRUE)glDeleteTextures(1, &sprite->image);
  }*/
}

void CloseSprites()
{
  int i;
   for(i = 0;i < MaxSprites;i++)
   {
     /*it shouldn't matter if the sprite is already freed, 
     FreeSprite checks for that*/
      if(SpriteList[i].loaded)
      {
        if(SpriteList[i].surface != NULL)SDL_FreeSurface(SpriteList[i].surface);
        SpriteList[i].surface = NULL;
        if(glIsTexture(SpriteList[i].image) == GL_TRUE)glDeleteTextures(1, &SpriteList[i].image);
      }
   }
   memset(SpriteList,0,sizeof(SpriteList));
}

void DrawGreySprite(Sprite *sprite,SDL_Surface *surface,int sx,int sy, int frame)
{
  int i,j;
  int offx,offy;
  Uint8 r,g,b;
  Uint32 pixel;
  Uint32 Key = sprite->surface->format->colorkey;
  offx = frame%sprite->framesperline * sprite->w;
  offy = frame/sprite->framesperline * sprite->h;
  if ( SDL_LockSurface(sprite->surface) < 0 )
  {
      fprintf(stderr, "Can't lock videobuffer: %s\n", SDL_GetError());
      exit(1);
  }
  for(j = 0;j < sprite->h;j++)
  {
    for(i = 0;i < sprite->w;i++)
    {
      pixel = getpixel(sprite->surface, i + offx ,j + offy);
      if(Key != pixel)
      {
        SDL_GetRGB(pixel, sprite->surface->format, &r, &g, &b);
        r = (r + g + b)/3;
        putpixel(surface, sx + i, sy + j, SDL_MapRGB(sprite->surface->format, r, r, r));
      }
    }
  }
  SDL_UnlockSurface(sprite->surface);
}

void OldDrawSprite(Sprite *sprite,SDL_Surface *surface,int sx,int sy, int frame)
{
    SDL_Rect src,dest;
    src.x = frame%sprite->framesperline * sprite->w;
    src.y = frame/sprite->framesperline * sprite->h;
    src.w = sprite->w;
    src.h = sprite->h;
    dest.x = sx;
    dest.y = sy;
    dest.w = sprite->w;
    dest.h = sprite->h;
    SDL_BlitSurface(sprite->surface, &src, surface, &dest);
}

void DrawSpritePixel(Sprite *sprite,SDL_Surface *surface,int sx,int sy, int frame)
{
    SDL_Rect src,dest;
    src.x = frame%sprite->framesperline * sprite->w + sprite->w/2;
    src.y = frame/sprite->framesperline * sprite->h + sprite->h/2;
    src.w = 1;
    src.h = 1;
    dest.x = sx;
    dest.y = sy;
    dest.w = 1;
    dest.h = 1;
    SDL_BlitSurface(sprite->surface, &src, surface, &dest);
}

Uint32 SetColor(Uint32 color, int newcolor1,int newcolor2, int newcolor3)
{
    Uint8 r,g,b,a;
    Uint8 intensity;
    int newcolor;

    SDL_GetRGBA(color, videobuffer->format, &r, &g, &b,&a);
    if((r == 0) && (g == 0)&&(b !=0))
    {
        intensity = b;
        newcolor = newcolor3;
    }
    else if((r ==0)&&(b == 0)&&(g != 0))
    {
        intensity = g;
        newcolor = newcolor2;
    }
    else if((g == 0)&&(b == 0)&&(r != 0))
    {
        intensity = r;
        newcolor = newcolor1;
    }
    else return color;
    switch(newcolor)
    {
        case Red:
            r = intensity;
            g = 0;
            b = 0;
            break;
        case Green:
            r = 0;
            g = intensity;
            b = 0;
            break;
        case Blue:
            r = 0;
            g = 0;
            b = intensity;
            break;
        case Yellow:
            r = (Uint8)(intensity * 0.7);
            g = (Uint8)(intensity * 0.7);
            b = 0;
            break;
        case Orange:
            r = (Uint8)(intensity * 0.9);
            g = (Uint8)(intensity * 0.4);
            b = (Uint8)(intensity * 0.1);
            break;
        case Violet:
            r = (Uint8)(intensity * 0.7);
            g = 0;
            b = (Uint8)(intensity * 0.7);
            break;
        case Brown:
            r = (Uint8)(intensity * 0.6);
            g = (Uint8)(intensity * 0.3);
            b = (Uint8)(intensity * 0.15);
            break;
        case Grey:
            r = (Uint8)(intensity * 0.5);
            g = (Uint8)(intensity * 0.5);
            b = (Uint8)(intensity * 0.5);
            break;
        case DarkRed:
            r = (Uint8)(intensity * 0.5);
            g = 0;
            b = 0;
            break;
        case DarkGreen:
            r = 0;
            g = (Uint8)(intensity * 0.5);
            b = 0;
            break;
        case DarkBlue:
            r = 0;
            g = 0;
            b = (Uint8)(intensity * 0.5);
            break;
        case DarkYellow:
            r = (Uint8)(intensity * 0.4);
            g = (Uint8)(intensity * 0.4);
            b = 0;
            break;
        case DarkOrange:
            r = (Uint8)(intensity * 0.6);
            g = (Uint8)(intensity * 0.2);
            b = (Uint8)(intensity * 0.1);
            break;
        case DarkViolet:
            r = (Uint8)(intensity * 0.4);
            g = 0;
            b = (Uint8)(intensity * 0.4);
            break;
        case DarkBrown:
            r = (Uint8)(intensity * 0.2);
            g = (Uint8)(intensity * 0.1);
            b = (Uint8)(intensity * 0.05);
            break;
        case DarkGrey:
            r = (Uint8)(intensity * 0.3);
            g = (Uint8)(intensity * 0.3);
            b = (Uint8)(intensity * 0.3);
            break;
        case LightRed:
            r = intensity;
            g = (Uint8)(intensity * 0.45);
            b = (Uint8)(intensity * 0.45);
            break;
        case LightGreen:
            r = (Uint8)(intensity * 0.45);
            g = intensity;
            b = (Uint8)(intensity * 0.45);
            break;
        case LightBlue:
            r = (Uint8)(intensity * 0.45);
            b = intensity;
            g = (Uint8)(intensity * 0.45);
            break;
        case LightYellow:
            r = intensity;
            g = intensity;
            b = (Uint8)(intensity * 0.45);
            break;
        case LightOrange:
            r = intensity;
            g = (Uint8)(intensity * 0.75);
            b = (Uint8)(intensity * 0.35);
            break;
        case LightViolet:
            r = intensity;
            g = (Uint8)(intensity * 0.45);
            b = intensity;
            break;
        case LightBrown:
            r = intensity;
            g = (Uint8)(intensity * 0.85);
            b = (Uint8)(intensity * 0.45);
            break;
        case LightGrey:
            r = (Uint8)(intensity * 0.85);
            g = (Uint8)(intensity * 0.85);
            b = (Uint8)(intensity * 0.85);
            break;
        case Black:
            r = (Uint8)(intensity * 0.15);
            g = (Uint8)(intensity * 0.15);
            b = (Uint8)(intensity * 0.15);
            break;
        case White:
            r = intensity;
            g = intensity;
            b = intensity;
            break;
        case Tan:
            r = intensity;
            g = (Uint8)(intensity * 0.9);
            b = (Uint8)(intensity * 0.6);
            break;
        case Gold:
            r = (Uint8)(intensity * 0.8);
            g = (Uint8)(intensity * 0.7);
            b = (Uint8)(intensity * 0.2);
            break;
        case Silver:
            r = (Uint8)(intensity * 0.95);
            g = (Uint8)(intensity * 0.95);
            b = intensity;
            break;
        case YellowGreen:
            r = (Uint8)(intensity * 0.45);
            g = (Uint8)(intensity * 0.75);
            b = (Uint8)(intensity * 0.2);
            break;
        case Cyan:
            r = 0;
            g = (Uint8)(intensity * 0.85);
            b = (Uint8)(intensity * 0.85);
            break;
        case Magenta:
            r = (Uint8)(intensity * 0.7);
            g = 0;
            b = (Uint8)(intensity * 0.7);
            break;
		default:
            r = 0;
            g = (Uint8)(intensity * 0.85);
            b = (Uint8)(intensity * 0.85);

			break;
    }
	color = SDL_MapRGBA(videobuffer->format,r,g,b,a);

    return color;
}

void DrawLaser(float x, float y,float z,float rx, float ry,float rz, float r, float g, float b,float length)
{

  glPushMatrix();
  
  glTranslatef(x,y,z);
  glRotatef(rx, 1.0f, 0.0f, 0.0f);
  glRotatef(ry, 0.0f, 1.0f, 0.0f);
  glRotatef(rz, 0.0f, 0.0f, 1.0f);

  glLineWidth(1);
  glBegin( GL_LINES);
  
  glColor4f(r,g,b,1);
  glVertex3f(length,0,0);
  glVertex3f(0,0,0);
  
  glEnd();
  glLineWidth(2);
  glColor4f(1,1,1,1);
  glPopMatrix();

}

void DrawLine2D(int sx, int sy, int gx,int gy,float thick, float r, float g, float b, float a)
{

  Coord pos,pos2;

  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  pos = GetGlCoord(sx,sy,_2DPLANE_,modelview,projection,viewport);
  pos2 = GetGlCoord(gx,gy,_2DPLANE_,modelview,projection,viewport);

  glPushMatrix();
  glColor4f(r,g,b,a);
  glLineWidth(thick);
  glBegin( GL_LINES );

  glVertex3f(pos.x,pos.y,pos.z);
  glVertex3f(pos2.x,pos2.y,pos2.z);

  glEnd( );
  glColor4f(1,1,1,1);
  glPopMatrix();
  glDisable(GL_BLEND);
}

void DrawLine(float x, float y,float z,float rx, float ry,float rz, float r, float g, float b)
{
  
  glPushMatrix();
  
  glLineWidth(1);
  glBegin( GL_LINES);
  
  glColor4f(r,g,b,1);
  glVertex3f(x,y,z);
  glVertex3f(rx,ry,rz);
  
  glEnd();
  glColor4f(1,1,1,1);
  glPopMatrix();
}


void drawBox(float x, float y, float z)
{


  glPushMatrix();

  glTranslatef(x,y,z);
  glScalef(0.5f,0.5f,0.5f);
  glBegin(GL_QUADS);

  glVertex3f(1,1,1);

  glVertex3f(-1,1,1);

  glVertex3f(-1,-1,1);

  glVertex3f(1,-1,1);

  glVertex3f(1,1,-1);

  glVertex3f(-1,1,-1);

  glVertex3f(-1,-1,-1);

  glVertex3f(1,-1,-1);


  glVertex3f(1,1,1);

  glVertex3f(-1,1,1);

  glVertex3f(-1,1,-1);

  glVertex3f(1,1,-1);

  
  glVertex3f(1,-1,1);

  glVertex3f(-1,-1,1);

  glVertex3f(-1,-1,-1);

  glVertex3f(1,-1,-1);


  glVertex3f(1,1,1);

  glVertex3f(1,1,-1);

  glVertex3f(1,-1,-1);

  glVertex3f(1,-1,1);



  glVertex3f(-1,1,1);

  glVertex3f(-1,1,-1);

  glVertex3f(-1,-1,-1);

  glVertex3f(-1,-1,1);

  

  glEnd();
  glPopMatrix();

}


/*
 * and now bringing it all together, we swap the pure colors in the sprite out
 * and put the new colors in.  This maintains any of the artist's shading and
 * detail, but still lets us have that old school palette swapping.
 */
void SwapSprite(SDL_Surface *sprite,int color1,int color2,int color3)
{
    int x, y;
  SDL_Surface *temp;
    Uint32 pixel,pixel2;

  
   /*First the precautions, that are tedious, but necessary*/
    if(color1 == -1)return;
    if(sprite == NULL)return;
    temp = SDL_DisplayFormatAlpha(sprite);
    if ( SDL_LockSurface(temp) < 0 )
    {
        fprintf(stderr, "Can't lock surface: %s\n", SDL_GetError());
        exit(1);
    }
   /*now step through our sprite, pixel by pixel*/
    for(y = 0;y < sprite->h ;y++)
    {
      for(x = 0;x < sprite->w ;x++)
        {
            pixel = getpixel(temp,x,y);/*and swap it*/
	    pixel2 = SetColor(pixel,color1,color2,color3);
            putpixel(sprite,x,y,pixel2);

        }
    }
    SDL_UnlockSurface(temp);
    SDL_FreeSurface(temp);
}

Uint32 IndexColor(int color)
{
  switch(color)
  {
    case Red:
      return SDL_MapRGB(videobuffer->format,138,0,0);;
    case Green:
      return SDL_MapRGB(videobuffer->format,0,138,0);;
    case Blue:
      return SDL_MapRGB(videobuffer->format,0,0,138);;
    case Yellow:
      return SDL_MapRGB(videobuffer->format,196,196,0);;
    case Orange:
      return SDL_MapRGB(videobuffer->format,148,118,0);;
    case Violet:
      return SDL_MapRGB(videobuffer->format,128,0,128);
    case Brown:
      return SDL_MapRGB(videobuffer->format,100,64,4);
    case Grey:
      return SDL_MapRGB(videobuffer->format,128,128,128);
    case DarkRed:
      return SDL_MapRGB(videobuffer->format,64,0,0);
    case DarkGreen:
      return SDL_MapRGB(videobuffer->format,0,64,0);
    case DarkBlue:
      return SDL_MapRGB(videobuffer->format,0,0,64);
    case DarkYellow:
      return SDL_MapRGB(videobuffer->format,60,60,0);
    case DarkOrange:
      return SDL_MapRGB(videobuffer->format,64,56,0);
    case DarkViolet:
      return SDL_MapRGB(videobuffer->format,60,0,60);
    case DarkBrown:
      return SDL_MapRGB(videobuffer->format,56,32,2);
    case DarkGrey:
      return SDL_MapRGB(videobuffer->format,64,64,64);
    case LightRed:
      return SDL_MapRGB(videobuffer->format,255,32,32);
    case LightGreen:
      return SDL_MapRGB(videobuffer->format,32,255,32);
    case LightBlue:
      return SDL_MapRGB(videobuffer->format,32,32,255);
    case LightYellow:
      return SDL_MapRGB(videobuffer->format,250,250,60);
    case LightOrange:
      return SDL_MapRGB(videobuffer->format,255,234,30);
    case LightViolet:
      return SDL_MapRGB(videobuffer->format,250,30,250);
    case LightBrown:
      return SDL_MapRGB(videobuffer->format,200,100,32);
    case LightGrey:
      return SDL_MapRGB(videobuffer->format,196,196,196);
    case Black:
      return SDL_MapRGB(videobuffer->format,0,0,0);
    case White:
      return SDL_MapRGB(videobuffer->format,255,255,255);
    case Tan:
      return SDL_MapRGB(videobuffer->format,255,128,64);
    case Gold:
      return SDL_MapRGB(videobuffer->format,255,245,30);
    case Silver:
      return SDL_MapRGB(videobuffer->format,206,206,226);
    case YellowGreen:
      return SDL_MapRGB(videobuffer->format,196,255,30);
    case Cyan:
      return SDL_MapRGB(videobuffer->format,0,255,255);;
    case Magenta:
      return SDL_MapRGB(videobuffer->format,255,0,255);
  }
  return SDL_MapRGB(videobuffer->format,0,0,0);
}


Sprite *LoadModelTexture(char *texturefile)
{
  return LoadSwappedSprite(texturefile,-1,-1,-1,-1,-1,1);
}

Sprite *LoadModelSwappedTexture(char *texturefile, int c1, int c2, int c3)
{
  return LoadSwappedSprite(texturefile,-1,-1,c1,c2,c3,1);
}


/*
Opengl support functions
*/

Coord Point2DFrom3D(Coord p)
{
  return GetGlCoord(p.x,p.y,_2DPLANE_,modelview,projection,viewport);
}

void glvfToCoord(float *v, Coord *c)
{
  c->x = v[0];
  c->y = v[1];
  c->z = v[2];
}

void CoordToglvf(Coord c, float *v)
{
  v[0] = c.x;
  v[1] = c.y;
  v[2] = c.z;
}

float DegreeFromVect(float a, float b)
{
  int negx = 0;
  int muly = 1;
  float rot;
  if((b != 0) || (a != 0))
  {
    if((b < 0) && (a < 0))negx = -180;
    else
    {
      if(a < 0)
      {
        muly = -1;
        negx = -180;
      }
      if(b < 0)muly = -1;
    }
    if(a == 0)
    {
      if(b > 0)
        rot = 180;
      else rot = 0;
    }
    else rot = (muly * (atan(fabs(b) / fabs(a)) * RadToDeg)) + 90 + negx;
  }
  return rot;
}

Coord GetGlCoord(int x, int y, float z, const GLdouble *model, const GLdouble *proj, const GLint *view)
{

  Coord temp;
  GLdouble dstx, dsty, dstz;  
  GLfloat scrx, scry, scrz;

  scrx= (float)x;
  scry=(float)view[3] - (float)y;
  scrz= z;

  gluUnProject(scrx,scry,scrz,model,proj,view,&dstx,&dsty,&dstz);

  temp.x=dstx;
  temp.y=dsty;
  temp.z=dstz;

  return temp;
}



float GetGlz(int z, float plane, const GLdouble *model, const GLdouble *proj, const GLint *view)
{

  Coord temp;
  GLdouble dstx, dsty, dstz;  
  GLfloat scrx, scry, scrz;

  scrx= z;
  scry=(float)view[3] - 0;
  scrz= plane;

  gluUnProject(scrx,scry,scrz,model,proj,view,&dstx,&dsty,&dstz);

  temp.x=dstx;
  temp.y=dsty;
  temp.z=dstz;

  return temp.x;
}

Coord GetScreenCoord(float x, float y, float z)
{
  GLint viewport[4];
  GLdouble modelview[16];
  GLdouble projection[16];
  Coord temp;
  GLdouble dstx, dsty, dstz;

  glGetIntegerv(GL_VIEWPORT,viewport);
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);

  gluProject(x,y,z,modelview,projection,viewport,&dstx,&dsty,&dstz);
  temp.y=(float)viewport[3] - dsty;
  temp.x=dstx;
  temp.z=dstz;
  return temp;
}

void ScreenCapture(const char *file)
{
  SDL_Surface *temp;
  SDL_Surface *image;
  int idx;

  temp = SDL_CreateRGBSurface(SDL_SWSURFACE,S_Data.xres, S_Data.yres,24,rmask,gmask,bmask,0x000000);
  image = SDL_CreateRGBSurface(SDL_SWSURFACE,S_Data.xres, S_Data.yres,24,rmask,gmask,bmask,0x000000);
  glReadBuffer(GL_FRONT);
  glReadPixels(0,0,S_Data.xres, S_Data.yres,GL_RGB, GL_UNSIGNED_BYTE,image->pixels);

  for(idx =0; idx <S_Data.yres;idx++)
  {
    memcpy((unsigned char *)(temp->pixels) + 3*S_Data.xres*idx,(unsigned char *)(image->pixels) + 3*S_Data.xres*(S_Data.yres-idx),3*S_Data.xres);
  }
  memcpy(image->pixels,temp->pixels,S_Data.xres*S_Data.yres*3);
  SDL_SaveBMP(image,file);
  SDL_FreeSurface(temp);
  SDL_FreeSurface(image);
}



/*
  Copied from SDL's website.  I use it for palette swapping
  Its not plagerism if you document it!
*/
void DrawSquareLine(SDL_Surface *videobuffer,Uint32 color,int sx,int sy,int gx,int gy)
{ 
  SDL_Rect box;
  if(sx < gx)box.x = sx;
  else box.x = gx;
  if(sy < gy)box.y = sy;
  else box.y = gy;
  if(sy == gy)
  {
    box.w = fabs(sx - gx);
    box.h = 1;                                        
    SDL_FillRect(videobuffer,&box,color);    
    return;
  }
  box.h = fabs(sy - gy);
  box.w = 1;                                        
  SDL_FillRect(videobuffer,&box,color);    
}

void DrawPixel(SDL_Surface *videobuffer, Uint8 R, Uint8 G, Uint8 B, int x, int y)
{
    Uint32 color = SDL_MapRGB(videobuffer->format, R, G, B);

    if ( SDL_LockSurface(videobuffer) < 0 )
    {
      return;
    }
    switch (videobuffer->format->BytesPerPixel)
    {
        case 1:
        { /* Assuming 8-bpp */
            Uint8 *bufp;

            bufp = (Uint8 *)videobuffer->pixels + y*videobuffer->pitch + x;
            *bufp = color;
        }
        break;

        case 2:
        { /* Probably 15-bpp or 16-bpp */
            Uint16 *bufp;

            bufp = (Uint16 *)videobuffer->pixels + y*videobuffer->pitch/2 + x;
            *bufp = color;
        }
        break;

        case 3:
        { /* Slow 24-bpp mode, usually not used */
            Uint8 *bufp;

            bufp = (Uint8 *)videobuffer->pixels + y*videobuffer->pitch + x;
            *(bufp+videobuffer->format->Rshift/8) = R;
            *(bufp+videobuffer->format->Gshift/8) = G;
            *(bufp+videobuffer->format->Bshift/8) = B;
        }
        break;

        case 4:
        { /* Probably 32-bpp */
            Uint32 *bufp;

            bufp = (Uint32 *)videobuffer->pixels + y*videobuffer->pitch/4 + x;
            *bufp = color;
        }
        break;
    }
    SDL_UnlockSurface(videobuffer);
    SDL_UpdateRect(videobuffer, x, y, 1, 1);
}

Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
  /* Here p is the address to the pixel we want to retrieve*/
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;
  if((x < 0)||(x >= surface->w)||(y < 0)||(y >= surface->h))return -1;
  switch(surface->format->BytesPerPixel)
  {
    case 1:
      return *p;

    case 2:
      return *(Uint16 *)p;

    case 3:
      if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
        return p[0] << 16 | p[1] << 8 | p[2];
      else
        return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
      return *(Uint32 *)p;

    default:
      return 0;       /*shouldn't happen, but avoids warnings*/
  }
}



/*
 * the putpixel function ont he SDL website doesn't always wrk right.  Here is a REAL simple alternative.
 */
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
  SDL_Rect point = {0,0,1,1};
  point.x = x;
  point.y = y;
  SDL_FillRect(surface,&point,pixel);
}



/*
  copied and pasted and then significantly modified from the sdl website.  
  I kept ShowBMP to test my program as I wrote it, and I rewrote it to use any file type supported by SDL_image
*/

void ShowBMP(SDL_Surface *image, SDL_Surface *videobuffer, int x, int y)
{
    SDL_Rect dest;

    /* Blit onto the videobuffer surface.
       The surfaces should not be locked at this point.
     */
    dest.x = x;
    dest.y = y;
    dest.w = image->w;
    dest.h = image->h;
    SDL_BlitSurface(image, NULL, videobuffer, &dest);

    /* Update the changed portion of the videobuffer */
    SDL_UpdateRects(videobuffer, 1, &dest);
}


/*needed for opengl compatibility*/
int powerOfTwo(int input) 
{
  int value = 1;

  while (value < input) 
  {
    value <<= 1;
  }
  return value;
}


/*
  makes sure a minimum number of ticks is waited between frames
  this is to ensure that on faster machines the game won't move so fast that
  it will look terrible.
  This is a very handy function in game programming.
*/

void FrameDelay(Uint32 delay)
{
    static Uint32 pass = 100;
    Uint32 dif;
    dif = SDL_GetTicks() - pass;
    if(dif < delay)SDL_Delay( delay - dif);
    pass = SDL_GetTicks();
}
/*draws an elipse at the location specified*/
void DrawElipse(int ox,int oy, int radius, Uint32 Color, SDL_Surface *surface)
{
  int r2 = radius * radius;
  int x,y;
  for(x = radius * -1;x <= radius;x++)
  {
    y = (int) (sqrt(r2 - x*x) * 0.6);
    putpixel(surface, x + ox, oy + y, Color);
    putpixel(surface, x + ox, oy - y, Color);
  }
}

/*draws a circle*/
void DrawCircle3D(float x,float y,float z,float radius, Coord color, float alpha)
{
  glPushMatrix();
  glPointSize(radius);
  glEnable(GL_POINT_SMOOTH);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND); 
  glColor4f(color.x,color.y,color.z,alpha);
  glBegin(GL_POINTS);
  glVertex3f(x,y,z);
  glEnd();
  glColor4f(1,1,1,1);
  glDisable(GL_BLEND);
  glPopMatrix();
}

/*draws an rectangle outline at the coordinates of the width and height*/
void DrawRect(int sx,int sy, int sw, int sh, Uint32 Color)
{    
  Uint8 r,g,b,a;
  Coord pos, pos2;
  pos = GetGlCoord(sx,sy,_2DPLANE_,modelview,projection,viewport);
  pos2 = GetGlCoord(sx + sw,sy + sh,_2DPLANE_,modelview,projection,viewport);
  SDL_GetRGBA(Color, videobuffer->format, &r, &g, &b, &a);

  glDisable(GL_DEPTH_TEST);
  glBegin( GL_LINES );
  glColor4f(r/256.0f,g/256.0f,b/256.0f,1);
    
  glVertex3f( pos2.x,pos.y,pos.z);
  glVertex3f( pos.x,pos.y,pos.z);
    
  glVertex3f( pos.x,pos2.y,pos.z);
  glVertex3f( pos.x,pos.y,pos.z);
    
  glVertex3f( pos2.x,pos2.y,pos.z);
  glVertex3f( pos2.x,pos.y,pos.z);
    
  glVertex3f( pos2.x,pos2.y,pos.z);
  glVertex3f( pos.x,pos2.y,pos.z);
    
  glEnd( );
  glColor4f(1,1,1,1);
  glEnable(GL_DEPTH_TEST);
    
    
}

/*draws a filled rect at the coordinates, in the color, on the surface specified*/
void DrawFilledRect(int sx,int sy, int sw, int sh, Uint32 Color)
{
  DrawFilledRectAlpha( sx, sy, sw, sh,Color,1);
}

void DrawFilledRectAlpha(int sx,int sy, int sw, int sh, Uint32 Color,float alpha)
{
  Uint8 r,g,b,a;
  float r2,g2,b2;
  Coord pos, pos2;
  pos = GetGlCoord(sx,sy,_2DPLANE_,modelview,projection,viewport);
  pos2 = GetGlCoord(sx + sw,sy + sh,_2DPLANE_,modelview,projection,viewport);
  SDL_GetRGBA(Color, videobuffer->format, &r, &g, &b, &a);
  r2 = ((float)r / 255.0);
  g2 = ((float)g / 255.0);
  b2 = ((float)b / 255.0);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBegin( GL_QUADS );
  glColor4f(r2,g2,b2,alpha);
  
  glVertex3f( pos2.x,pos2.y,pos2.z);
  
  glVertex3f( pos2.x,pos.y,pos.z);
  
  glVertex3f( pos.x,pos.y,pos.z);
  
  glVertex3f( pos.x,pos2.y,pos.z);
  
  glEnd( );
  glEnable(GL_DEPTH_TEST);
  glColor4f(1,1,1,1);
}

void DrawGradientRect(int sx,int sy, int sw, int sh, Uint32 ul, Uint32 ur, Uint32 bl, Uint32 br,float alpha)
{
  Uint8 r,g,b,a;
  float r2,g2,b2;
  Coord pos, pos2;
  pos = GetGlCoord(sx,sy,_2DPLANE_,modelview,projection,viewport);
  pos2 = GetGlCoord(sx + sw,sy + sh,_2DPLANE_,modelview,projection,viewport);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBegin( GL_QUADS );
  SDL_GetRGBA(br, videobuffer->format, &r, &g, &b, &a);
  r2 = ((float)r / 255.0);
  g2 = ((float)g / 255.0);
  b2 = ((float)b / 255.0);
  glColor4f(r2,g2,b2,alpha);
  
  glVertex3f( pos2.x,pos2.y,pos2.z);
  
  SDL_GetRGBA(ur, videobuffer->format, &r, &g, &b, &a);
  r2 = ((float)r / 255.0);
  g2 = ((float)g / 255.0);
  b2 = ((float)b / 255.0);
  glColor4f(r2,g2,b2,alpha);

  glVertex3f( pos2.x,pos.y,pos.z);
  
  SDL_GetRGBA(ul, videobuffer->format, &r, &g, &b, &a);
  r2 = ((float)r / 255.0);
  g2 = ((float)g / 255.0);
  b2 = ((float)b / 255.0);
  glColor4f(r2,g2,b2,alpha);
  
  glVertex3f( pos.x,pos.y,pos.z);
  
  SDL_GetRGBA(bl, videobuffer->format, &r, &g, &b, &a);
  r2 = ((float)r / 255.0);
  g2 = ((float)g / 255.0);
  b2 = ((float)b / 255.0);
  glColor4f(r2,g2,b2,alpha);
  
  glVertex3f( pos.x,pos2.y,pos.z);
  
  glEnd( );
  glEnable(GL_DEPTH_TEST);
  glColor4f(1,1,1,1);
}

/*sets an sdl surface to all color.*/

void BlankScreen(SDL_Surface *buf,Uint32 color)
{
    SDL_LockSurface(buf);
    memset(buf->pixels, (Uint8)color,buf->format->BytesPerPixel * buf->w *buf->h);
    SDL_UnlockSurface(buf);
}

int SDLPointInRect(SDL_Point p,SDL_Rect r)
{
  if(p.x < r.x)return 0;
  if(p.y < r.y)return 0;
  if(p.x >= r.x + r.w)return 0;
  if(p.y >= r.y + r.h)return 0;
  return 1;
}

int BoxCollide(BBox box1,BBox box2)
{
  if((box1.x + box1.w >= box2.x) && (box1.x <= box2.x + box2.w) && (box1.y + box1.h >= box2.y) && (box1.y <= box2.y + box2.h) && (box1.z + box1.d >= box2.z) && (box1.z <= box2.z + box2.d))
  { 
    return 1;
  }else 
  {
    return 0;
  }
}


int BoxCollide2(BBox box1,BBox box2)
{
  if((box1.x + box1.w >= box2.x) && (box1.x <= box2.x + box2.w) && (box1.y + box1.h >= box2.y) && (box1.y <= box2.y + box2.h))
  { 
    return 1;
  }else 
  {
    return 0;
  }
}


void IncreaseMagnitude(Coord *v,float m)
{
   Coord vb;
   if(v == NULL)
   {
     fprintf(stderr,"unable to increase the magnitude of a NULL Pointer vector\n");
     return;
   }
   VectorSet(vb,v->x,v->y,v->z);
   Normalize(&vb);
   VectorScale(vb,m,vb);
   v->x += vb.x;
   v->y += vb.y;
   v->z += vb.z;
}


void RotateVector (const GLfloat *model, Coord *V, Coord *D)
{
  D->x = (model[0] * V->x) + (model[4] * V->y) + (model[8]  * V->z);
  D->y = (model[1] * V->x) + (model[5] * V->y) + (model[9]  * V->z);
  D->z = (model[2] * V->x) + (model[6] * V->y) + (model[10] * V->z);
}

/* Calculate The Length Of The Vector*/
float Magnitude (Coord V)
{
  return sqrt (V.x * V.x + V.y * V.y + V.z * V.z);
}

void Normalize (Coord *V)
{
  float M = Magnitude (*V);

  if (M != 0.0f)
  {
    V->x /= M;
    V->y /= M;
    V->z /= M;
  }
}

/* Calculate The Angle Between The 2 Vectors*/
float dotproduct (Coord *V1, Coord *V2)
{
  return V1->x * V2->x + V1->y * V2->y + V1->z * V2->z;
}

Sprite *loadShader(char *filename)
{
  int i,j;
  float shaderData[32][3];
  FILE *file = NULL;
  
  for(i = 0; i < MaxSprites; i++)
  {
    if((strncmp(filename,SpriteList[i].filename,80)==0))
    {
      SpriteList[i].used++;
      return &SpriteList[i];
    }
  }
  NumSprites++;
  for(i = 0;i <= MaxSprites;i++)
  {
    if(!SpriteList[i].used)break;
  }
  
  strncpy(SpriteList[i].filename,filename,20);
  SpriteList[i].used++;
  
  file = fopen (filename, "r");

  if (file)
  {
    for (j = 0; j < 32; j++)
    {
      if(feof (file))break;
      
      fscanf(file, "%f", &shaderData[j][0]);
      shaderData[j][1]=shaderData[j][0];
      shaderData[j][2]=shaderData[j][0];
    }

    fclose (file);
  }
  
  glGenTextures (1, &SpriteList[i].image);

  glBindTexture (GL_TEXTURE_1D, SpriteList[i].image);

  glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
  glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glTexImage1D (GL_TEXTURE_1D, 0, GL_RGB, 32, 0, GL_RGB , GL_FLOAT, shaderData);
  
  return &SpriteList[i];
}




GLuint LoadShaderProgram(char *filenameVertex, char *filenameFragment)
{

  GLuint program = 0;
  char *buffer1;
  char *buffer2;
  const GLchar *pSource = 0;
  long length = 0;
  GLuint vertShader = 0;
  GLuint fragShader = 0;
  FILE* file;



  if (filenameVertex != NULL)
  {

    file = fopen(filenameVertex, "r");
    if(!file)
    {
      fprintf(stderr, "failed to open vert shader file %s\n", filenameVertex);
      exit(1);
    }
    fseek (file , 0 , SEEK_END);
    length = ftell (file);
    rewind (file);
    buffer1= (char*) malloc(sizeof(char)*length);
    fread(buffer1,1,length,file);
        
    fclose (file);
    pSource = (const GLchar *)(buffer1);

    vertShader = CompileShader(GL_VERTEX_SHADER, pSource, length);
  }

  if (filenameFragment != NULL)
  {
    file = fopen(filenameFragment, "r");
    if(!file)
    {
      fprintf(stderr, "failed to open frag shader file %s\n", filenameFragment);
      exit(1);
    }
    fseek (file , 0 , SEEK_END);
    length = ftell(file);
    rewind(file);
    buffer2= (char*) malloc(sizeof(char)*length);
    fread(buffer2,1,length,file);
        
    fclose (file);
    pSource = (const GLchar *)(buffer2);
    fragShader = CompileShader(GL_FRAGMENT_SHADER, pSource, length);
  }


  program = linkShaders(vertShader, fragShader);



  return program;
}

GLuint linkShaders(GLuint vertShader, GLuint fragShader)
{

  GLuint program;
  GLint linked = 0;
  
  program = glCreateProgram();

  if (program)
  {

    if (vertShader)
      glAttachShader(program, vertShader);

    if (fragShader)
      glAttachShader(program, fragShader);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked)
    {
      fprintf(stderr, "failed to link shader\n");
      return 0;
    }
    if (vertShader)
      glDeleteShader(vertShader);

    if (fragShader)
      glDeleteShader(fragShader);
  }

  return program;
}

GLuint CompileShader(GLenum type, const GLchar *pszSource, GLint length)
{

  GLuint shader;
  GLint compiled = 0;
  
  shader = glCreateShader(type);
  
  if (shader)
  {
    glShaderSource(shader, 1, &pszSource, &length);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
      fprintf(stderr, "failed to compile shader\n");
      return 0;
    }
  }

  return shader;
}

Coord crossProd(Coord *V1, Coord *V2 )
{
  Coord rt;
  rt.x = V1->y*V2->z-V1->z*V2->y;
  rt.y = V1->z*V2->x-V1->x*V2->z;
  rt.z = V1->x*V2->y-V1->y*V2->x;	
  return rt;
}

void CalcTangentVector(const float pos1[3], const float pos2[3],
                       const float pos3[3], const float texCoord1[2],
                       const float texCoord2[2], const float texCoord3[2],
                       const float normal[3], float tangent[4])
{

  Coord edge1;
  Coord edge2;
  Coord texEdge1;
  Coord texEdge2;
  Coord t;
  Coord b;
  Coord n;
  float det;
  Coord bitangent;
  float handedness;
  
  edge1.x=pos2[0] - pos1[0];
  edge1.y=pos2[1] - pos1[1];
  edge1.z=pos2[2] - pos1[2];
  
  edge2.x=pos3[0] - pos1[0];
  edge2.y=pos3[1] - pos1[1];
  edge2.z=pos3[2] - pos1[2];
  
  Normalize(&edge1);
  Normalize(&edge2);
  
  texEdge1.x=texCoord2[0] - texCoord1[0];
  texEdge1.y=texCoord2[1] - texCoord1[1];
  texEdge1.z = 0;
  
  texEdge2.x=texCoord3[0] - texCoord1[0];
  texEdge2.y=texCoord3[1] - texCoord1[1];
  texEdge2.z = 0;
  
  Normalize(&texEdge1);
  Normalize(&texEdge2);

  n.x=normal[0];
  n.y=normal[1];
  n.z=normal[2];
  
  det = (texEdge1.x * texEdge2.y) - (texEdge1.y * texEdge2.x);

  if (closeEnough(det, 0.0f))
  {
    t.x= 1.0f;
    t.y= 0.0f;
    t.z= 0.0f;
    
    b.x= 0.0f;
    b.y= 1.0f;
    b.z= 0.0f;
  }
  else
  {
    det = 1.0f / det;

    t.x = (texEdge2.y * edge1.x - texEdge1.y * edge2.x) * det;
    t.y = (texEdge2.y * edge1.y - texEdge1.y * edge2.y) * det;
    t.z = (texEdge2.y * edge1.z - texEdge1.y * edge2.z) * det;

    b.x = (-texEdge2.x * edge1.x + texEdge1.x * edge2.x) * det;
    b.y = (-texEdge2.x * edge1.y + texEdge1.x * edge2.y) * det;
    b.z = (-texEdge2.x * edge1.z + texEdge1.x * edge2.z) * det;

    Normalize(&t);
    Normalize(&b);
  }
  
  bitangent = crossProd(&n, &t);
  handedness = (dotproduct(&bitangent, &b) < 0.0f) ? -1.0f : 1.0f;
  
  tangent[0] = t.x;
  tangent[1] = t.y;
  tangent[2] = t.z;
  tangent[3] = handedness;
}

int closeEnough(float f1, float f2)
{
        /* Determines whether the two floating-point values f1 and f2 are
           close enough together that they can be considered equal.*/

  return fabs((f1 - f2) / ((f2 == 0.0f) ? 1.0f : f2)) < EPSILON;
}




void LoadSkyBox(char *texturename)
{
  char loadname[80];
  sprintf(loadname,"%s_ft.tga",texturename);
  skyboxtextures[0] = LoadModelTexture(loadname);
  sprintf(loadname,"%s_lf.tga",texturename);
  skyboxtextures[1] = LoadModelTexture(loadname);
  sprintf(loadname,"%s_bk.tga",texturename);
  skyboxtextures[2] = LoadModelTexture(loadname);
  sprintf(loadname,"%s_rt.tga",texturename);
  skyboxtextures[3] = LoadModelTexture(loadname);
  sprintf(loadname,"%s_up.tga",texturename);
  skyboxtextures[4] = LoadModelTexture(loadname);
  sprintf(loadname,"%s_dn.tga",texturename);
  skyboxtextures[5] = LoadModelTexture(loadname);
}

void LoadSkyBox2(char *texturename)
{
  char loadname[80];
  sprintf(loadname,"%s",texturename);
  skyboxtextures[0] = LoadModelTexture(loadname);
  sprintf(loadname,"%s",texturename);
  skyboxtextures[1] = LoadModelTexture(loadname);
  sprintf(loadname,"%s",texturename);
  skyboxtextures[2] = LoadModelTexture(loadname);
  sprintf(loadname,"%s",texturename);
  skyboxtextures[3] = LoadModelTexture(loadname);
  sprintf(loadname,"%s",texturename);
  skyboxtextures[4] = LoadModelTexture(loadname);
  sprintf(loadname,"%s",texturename);
  skyboxtextures[5] = LoadModelTexture(loadname);
}

void DrawSkyBox()
{
  int tileNum = 4;

  glEnable(GL_TEXTURE_2D);

  
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
    
  glPushMatrix();
  glTranslatef(0.0f,0.0f,0.0f);
  if(skyboxtextures[SB_Top] != NULL)  glBindTexture(GL_TEXTURE_2D,skyboxtextures[SB_Top]->image); 
  glBegin(GL_QUADS);

  glNormal3f(0,0,-1);
  glTexCoord2f(tileNum,0);
  glVertex3f(1,1,1);
  glTexCoord2f(0,0);
  glVertex3f(-1,1,1);
  glTexCoord2f(0,tileNum);
  glVertex3f(-1,-1,1);
  glTexCoord2f(tileNum,tileNum);
  glVertex3f(1,-1,1);
  glEnd();
  
  if(skyboxtextures[SB_Bottom] != NULL)  glBindTexture(GL_TEXTURE_2D,skyboxtextures[SB_Bottom]->image); 
  glBegin(GL_QUADS);
  glNormal3f(0,0,1);
  glTexCoord2f(tileNum,0);
  glVertex3f(1,1,-1);
  glTexCoord2f(0,0);
  glVertex3f(-1,1,-1);
  glTexCoord2f(0,tileNum);
  glVertex3f(-1,-1,-1);
  glTexCoord2f(tileNum,tileNum);
  glVertex3f(1,-1,-1);
  
  glEnd();
  if(skyboxtextures[SB_Front] != NULL)  glBindTexture(GL_TEXTURE_2D,skyboxtextures[SB_Front]->image); 
  glBegin(GL_QUADS);
  
  glNormal3f(0,-1,0);
  glTexCoord2f(tileNum,0);
  glVertex3f(1,1,1);
  glTexCoord2f(0,0);
  glVertex3f(-1,1,1);
  glTexCoord2f(0,tileNum);
  glVertex3f(-1,1,-1);
  glTexCoord2f(tileNum,tileNum);
  glVertex3f(1,1,-1);

  glEnd();
  if(skyboxtextures[SB_Back] != NULL)  glBindTexture(GL_TEXTURE_2D,skyboxtextures[SB_Back]->image); 
  glBegin(GL_QUADS);
  
  glNormal3f(0,1,0);
  glTexCoord2f(0,0);
  glVertex3f(1,-1,1);
  glTexCoord2f(tileNum,0);
  glVertex3f(-1,-1,1);
  glTexCoord2f(tileNum,tileNum);
  glVertex3f(-1,-1,-1);
  glTexCoord2f(0,tileNum);
  glVertex3f(1,-1,-1);

  glEnd();
  if(skyboxtextures[SB_Right] != NULL)  glBindTexture(GL_TEXTURE_2D,skyboxtextures[SB_Right]->image); 
  glBegin(GL_QUADS);

  glNormal3f(-1,0,0);
  glTexCoord2f(0,0);
  glVertex3f(1,1,1);
  glTexCoord2f(0,tileNum);
  glVertex3f(1,1,-1);
  glTexCoord2f(tileNum,tileNum);
  glVertex3f(1,-1,-1);
  glTexCoord2f(tileNum,0);
  glVertex3f(1,-1,1);
  
  glEnd();
  if(skyboxtextures[SB_Left] != NULL)  glBindTexture(GL_TEXTURE_2D,skyboxtextures[SB_Left]->image); 
  glBegin(GL_QUADS);
  
  glNormal3f(1,0,0);
  glTexCoord2f(tileNum,0);
  glVertex3f(-1,1,1);
  glTexCoord2f(tileNum,tileNum);
  glVertex3f(-1,1,-1);
  glTexCoord2f(0,tileNum);
  glVertex3f(-1,-1,-1);
  glTexCoord2f(0,0);
  glVertex3f(-1,-1,1);
  
  glEnd();

  
  glPopMatrix();
  glEnable(GL_CULL_FACE);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
}

void Updateskybox()
{
  static Coord rotation;
  static float colorShift = 1;
  static int flag=0;
  glPushMatrix();
  glRotatef(rotation.x,1.0f, 0.0f, 0.0f );
  glRotatef(rotation.y,0.0f, 1.0f, 0.0f );
  glRotatef(rotation.z,0.0f, 0.0f, 1.0f );
  
  rotation.x+=0.5f;
  rotation.y+=0.5f;
  if(flag == 0)
  {
    colorShift-= 0.01f;
    glColor4f(1,1,colorShift,1);
    if(colorShift<=0)
    {
      flag = 1;
    }
  }
  if(flag == 1)
  {
    colorShift+= 0.01f;
    glColor4f(1,1,colorShift,1);
    if(colorShift >= 1)
    {
      flag = 2;
      colorShift = 1;
    }
  }
  if(flag == 2)
  {
    colorShift-= 0.01f;
    glColor4f(colorShift,1,1,1);
    if(colorShift<=0)
    {
      flag = 3;
    }
  }
  if(flag == 3)
  {
    colorShift+= 0.01f;
    glColor4f(colorShift,1,1,1);
    if(colorShift>=1)
    {
      flag = 0;
      colorShift = 1;
    }
  }
  DrawSkyBox();
  glColor4f(1,1,1,1);
  glPopMatrix();
}
/**

  Load an Image through PhysFS and SDL_Image

**/
SDL_Surface *Load_Image(char *filename)
{
  SDL_Surface *image = NULL;
  SDL_RWops *rw;
  PHYSFS_sint64 size;
  PHYSFS_File *file;
  char *buffer = NULL;
  file = PHYSFS_openRead(filename);
  if(file == NULL)
  {
    fprintf(stderr,"unable to load file %s\n",filename);
    return NULL;
  }
  size = PHYSFS_fileLength(file);
  buffer = (char *)malloc(size);
  if(buffer == NULL)
  {
    fprintf(stderr,"Unable to allocate space for loading image: %s\n",filename);
    PHYSFS_close(file);
    return NULL;
  }
  PHYSFS_read(file,buffer,size,1);
  rw = SDL_RWFromMem(buffer, size);
  if(rw == NULL)
  {
    fprintf(stderr,"Unable to create RW reference to data loaded for %s\n",filename);
    free(buffer);
    PHYSFS_close(file);
    return NULL;
  }
  image = IMG_Load_RW(rw,0);
  SDL_FreeRW(rw);
  free(buffer);
  PHYSFS_close(file);
  return image;
}

/*end of file*/

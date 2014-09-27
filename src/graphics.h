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

#ifndef _GRAPHICS_
#define _GRAPHICS_

#include "initGL.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

/*
The Z value of the 2D plane, a range of 0 to 1
0 = the near Z
1 = the far Z
*/
#define _2DPLANE_    0.99f
#define MAX_FRAMES    255
#define MAX_GROUPS    8
#define EPSILON   1e-6f
#define DegToRad  0.017453277
#define RadToDeg  57.295779513082
#define DEGTORAD  0.017453292519943295769236907684886

#ifndef MIN
#define MIN(a,b)          (a<=b?a:b)
#endif

#ifndef MAX
#define MAX(a,b)          (a>=b?a:b)
#endif

#define DotProduct(a,b)     (a.x*b.x+a.y*b.y+a.z*b.z)
#define DotProduct2(x,y)     ((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define VectorSubtract(a,b,c) (c.x=a.x-b.x,c.y=a.y-b.y,c.z=a.z-b.z)
#define VectorAdd(a,b,c)    (c.x=a.x+b.x,c.y=a.y+b.y,c.z=a.z+b.z)
#define VectorAddP(a,b,c)    (c->x=a->x+b->x,c->y=a->y+b->y,c->z=a->z+b->z)
#define VectorScale(a,b,c)    (c.x=a.x*b,c.y=a.y*b,c.z=a.z*b)
#define VectorCopy(a,b)     (b.x=a.x,b.y=a.y,b.z=a.z)
#define VectorClear(a)      (a.x=a.y=a.z=0)
#define VectorNegate(a,b)   (b.x=-a.x,b.y=-a.y,b.z=-a.z)
#define VectorSet(v, a, b, c) (v.x=(a), v.y=(b), v.z=(c))

/*uint32*/
#define Red_         0xDD0000
#define Green_       0x00DD00
#define Blue_        0x0000DD
#define Yellow_      0xDDDD00
#define Orange_      0xEE5522
#define Violet_      0xDD00DD
#define Brown_       0x663311
#define Grey_        0x888888
#define DarkRed_     0x880000
#define DarkGreen_   0x008800
#define DarkBlue_    0x000088
#define DarkYellow_  0x888800
#define DarkOrange_  0x774411
#define DarkViolet_  0x660066
#define DarkBrown_   0x442205
#define DarkGrey_    0x444444
#define LightRed_    0xFF2222
#define LightGreen_  0x33FF33
#define LightBlue_   0x3333FF
#define LightYellow_ 0xFFFF11
#define LightOrange_ 0xFFDD11
#define LightViolet_ 0xFF55FF
#define LightBrown_  0x886622
#define LightGrey_   0xBBBBBB
#define Black_       0x000000
#define White_       0xFEFEFE
#define Tan_         0xFFDD22
#define Gold_        0xBB9900
#define Silver_      0xAAAABB
#define YellowGreen_ 0x44EE01
#define Cyan_        0x00FFFF
#define Magenta_     0xFF00FF


#define random()  ((rand () & 0x7fff) / ((float)0x7fff))
#define crandom() (2.0 * (random() - 0.5))

/*color constants:*/
enum colors {Red = 1,Green = 2,Blue = 3,Yellow = 4,Orange = 5,Violet = 6,Brown = 7,Grey = 8,DarkRed = 9,DarkGreen = 10,
               DarkBlue = 11,DarkYellow = 12,DarkOrange = 13,DarkViolet = 14,DarkBrown = 15,DarkGrey = 16,LightRed =17,
               LightGreen = 18,LightBlue = 19,LightYellow = 20,LightOrange = 21,LightViolet = 22,LightBrown = 23,LightGrey = 24,
               Black = 25,White = 26,Tan = 27,Gold = 28,Silver = 29,YellowGreen = 30,Cyan = 31,Magenta = 32};

enum FONTS {F_Small, F_Medium, F_Large, F_Digit,F_Count};

typedef struct
{
  int x,y;
}SDL_Point;

typedef struct POINT_T
{
  int x,y,z;
}Point;

typedef struct COORD_T
{
  float x,y,z;
}Coord;

typedef struct AABB_T
{
  float x,y,z;
  float w,h,d;
}BBox;

typedef struct MATERIAL_T
{
  float ambient[4];
  float modelAmbient[4];
  float diffuse[4];
  float specular[4];
  float emission[4];
  float shininess;
}Material;

/*
dater structure to handle sprites with both 
*/

typedef struct Sprite_T
{
  GLuint image;
  int loaded;
  char filename[80];
  float texHeight;
  float texWidth;
  int imageW;
  int imageH;
  SDL_Surface *surface;
  int color1,color2,color3;
  int w, h;         /*the width and height of the frames of the sprites, not the file itself*/
  float w2, h2; 
  Coord dimen;        /*the width and height with gl coords*/
  int framesperline;        /*default is 16*/
  int used;         /*used by the maintanenc functions to keep track of how many times a single sprite is being used*/
}Sprite;


/*non -axis aligned rectangle*/
typedef struct
{
  Coord center;
  Coord p[4];
}FreeRect;

typedef struct RECTF_T
{
  GLfloat x,y,w,h;
}Rectf;


typedef struct
{
    int xres,yres,depth;
}ScreenData;

typedef struct
{
  Coord  rotation;
  Coord  position;
  Coord  target;
  float  distance;
  int    freeCam;
}glCamera_T;



/*the basics*/
void DrawSplashScreen();
void Init_Graphics(int x, int y, int windowed);
/*basic drawing*/
void DrawPixel(SDL_Surface *screen, Uint8 R, Uint8 G, Uint8 B, int x, int y);
void DrawSquareLine(SDL_Surface *screen,Uint32 color,int sx,int sy,int gx,int gy);
void ShowImage(SDL_Surface *image, SDL_Surface *screen, int x, int y);
Uint32 getpixel(SDL_Surface *surface, int x, int y);
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
void DrawFilledRect(int sx,int sy, int sw, int sh, Uint32 Color);
void DrawFilledRectAlpha(int sx,int sy, int sw, int sh, Uint32 Color,float alpha);
void DrawGradientRect(int sx,int sy, int sw, int sh, Uint32 ul, Uint32 ur, Uint32 bl, Uint32 br,float alpha);
void DrawCircle3D(float x,float y,float z,float radius, Coord color, float alpha);
void DrawRect(int sx,int sy, int sw, int sh, Uint32 Color);
void DrawElipse(int ox,int oy, int radius, Uint32 Color, SDL_Surface *surface);
void DrawAnyLine(int sx,int sy,int dx, int dy,Uint32 Color,SDL_Surface *surface);
void DrawThickLine(int sx,int sy,int dx, int dy,int width,Uint32 Color,SDL_Surface *surface);
void Draw3DThickLine(Coord a, Coord b, float c1, float c2, float c3);
void DrawLine(float x, float y,float z,float x2, float y2,float z2, float r, float g, float b);
void DrawLine2D(int sx, int sy, int gx,int gy,float thick, float r, float g, float b, float a);

/*math*/
Coord M_Coord(float x, float y, float z);
int powerOfTwo(int input);
void glvfToCoord(float *v, Coord *c);
void CoordToglvf(Coord c, float *v);
float DegreeFromVect(float a, float b);
Coord GetGlCoord(int x, int y, float z, const GLdouble *model, const GLdouble *proj, const GLint *view);
Coord GetScreenCoord(float x, float y, float z);
float GetGlz(int z, float plane, const GLdouble *model, const GLdouble *proj, const GLint *view);
void ScreenCapture(const char *file);
float CoordLength(Coord v);
Coord Point2DFrom3D(Coord p);


/*sprite functions*/
SDL_Surface *Load_Image(char *filename);
void InitSpriteList();
void FreeSprite(Sprite *img);
Sprite *LoadSprite(char *filename,int sizex, int sizey);
void DrawSprite(Sprite *sprite, int sx, int sy, int frame, float scale);
void DrawSpriteStretch( Sprite *sprite, int sx, int sy, int frame, float scx,float scy );
void DrawSpriteAlpha( Sprite *sprite, int sx, int sy, int frame, float scx,float scy, float rot, float alpha );/*only draws the alpha to the destination buffer*/
void DrawGreySprite(Sprite *sprite,SDL_Surface *surface,int sx,int sy, int frame);
void DrawSpritePixel(Sprite *sprite,SDL_Surface *surface,int sx,int sy, int frame);
void DrawSpriteStretchRot( Sprite *sprite, int sx, int sy, int frame, float scx,float scy, float rot, float alpha );
void CloseSprites();
void DrawSpriteBG(Sprite *sprite, int sx, int sy, int frame);
void DrawSpriteScaledRotated(Sprite *sprite,int sx,int sy, int frame, float scale, float rotx, float roty, float rotz);
void DrawSpriteCropped( Sprite *sprite, int sx, int sy,int cx,int cy,int cw,int ch, int frame, float scx,float scy );

void DrawSprite2( Sprite *sprite, int sx, int sy, int frame, float scale );

/*'palette swapping functions*/
Uint32 SetColor(Uint32 color, int newcolor1,int newcolor2, int newcolor3);
void SwapSprite(SDL_Surface *sprite,int color1,int color2,int color3);
Uint32 IndexColor(int color);
Sprite *loadShader(char *filename);

Sprite *NewSprite();    /*Function creates a new empty sprite.  Not to be drawn until given data*/
Sprite *LoadModelTexture(char *texturefile);
Sprite *LoadSwappedSprite( char *filename, int sizex, int sizey, int c1, int c2, int c3 , int mipmapping);

/*frame handling functions*/
void BlankScreen(SDL_Surface *buf,Uint32 color);
void FrameDelay(Uint32 delay);
void ResetBuffer();
void NextFrame();

/*font stuff*/
inline float GetFontHeight(int fsize); /*gets the height of the font fsize*/
void LoadFonts();
void CloseFonts();
void LoadFont(char filename[40],int ptsize,int type);
BBox DrawTxtShadow(char *text,int sx,int sy,Uint32 color,int size,int offset);  /*this one has built  in drop shadow*/
BBox DrawTxt(char *text,int sx,int sy,Uint32 color,int size);
void DrawTextCenteredShadow(char *text,int sx,int sy,Uint32 color,int size,int offset);
void DrawTextCentered(char *text,int sx,int sy,Uint32 color,int size);
void DrawTextBlock(char *thetext,int sx, int sy,Uint32 color,int size,int width);
void DrawMessages();
void InitMessages();
void NewMsg(char *text);
void NewMessage(char *text,Uint32 color);
void MoveMessages(int x,int y,int drawcount);
void DrawTextZ(char *text,int sx,int sy,float z,Uint32 color,int size);
int GetTextBlockHeight(char *thetext,int size,int width);
void GetTextDimensions(char *text,int size, int *w, int *h);
    
void drawBox(float x, float y, float z);
void drawBoundBox(float x,float y,float w,float h);

void captureScene();
void drawScene();
void InitVideoTestBuffer();
void DeleteVideoTestBuffer();
void DrawLaser(float x, float y,float z,float rx, float ry,float rz, float r, float g, float b,float length);

int SDLPointInRect(SDL_Point p,SDL_Rect r);
int BoxCollide(BBox box1,BBox box2);
int BoxCollide2(BBox box1,BBox box2);
void DrawSprite3D(Sprite *sprite, Coord pos, Coord scale, int frame,float alpha);

void IncreaseMagnitude(Coord *v,float m); /*increases the magnitude of v by m, without changing direction*/
void RotateVector (const GLfloat *model, Coord *V, Coord *D);
float Magnitude (Coord V);
void Normalize (Coord *V);
float dotproduct (Coord *V1, Coord *V2);
Coord crossProd(Coord *V1, Coord *V2 );
GLuint linkShaders(GLuint vertShader, GLuint fragShader);
GLuint LoadShaderProgram(char *filenameVertex, char *filenameFragment);
GLuint CompileShader(GLenum type, const GLchar *pszSource, GLint length);
int closeEnough(float f1, float f2);
void CalcTangentVector(const float pos1[3], const float pos2[3],
                       const float pos3[3], const float texCoord1[2],
                       const float texCoord2[2], const float texCoord3[2],
                       const float normal[3], float tangent[4]);
                       
void DrawSkyBox();
void LoadSkyBox(char *texturename);
void LoadSkyBox2(char *texturename);
void Updateskybox();
int isMouseClicked();
void writeFPS();
void CropStr(char *text,int length,int strl);
#endif

#include <stdlib.h>
#include <string.h>
#include <SDL_ttf.h>
#include <physfs.h>
#include "graphics.h"

extern GLint viewport[4];
extern GLdouble modelview[16];
extern GLdouble projection[16];
extern ScreenData  S_Data;
extern Uint32 rmask,gmask,bmask,amask;
TTF_Font *SSFont = NULL;
TTF_Font *SFont = NULL;
TTF_Font *LFont = NULL;
TTF_Font *DFont = NULL;
extern SDL_PixelFormat *pixelFormat;
int   FontSize[4];
#define MAXMES    16

TTF_Font *Load_Font(char *filename,int ptsize);


typedef struct MESSAGE_T
{
  char message[MAXMES][80];
  Uint32 fadeout[MAXMES];     /*this is when the test will fade out.*/
  Uint32 color[MAXMES];
  Uint32 defcolor;            /*this is the default color for messages*/
  int count;
  int drawnum;
  int last;
  int sx,sy;
}Message;

Message messages;

inline float GetFontHeight(int fsize)
{
  if((fsize < 0)||(fsize >= F_Count))return 0;
  return FontSize[fsize];
}

void LoadFonts()
{
  FontSize[F_Small] = S_Data.yres*0.025f;
  LoadFont("fonts/font1.ttf",FontSize[F_Small],F_Small);
  FontSize[F_Medium] = S_Data.yres*0.040f;
  LoadFont("fonts/font1.ttf",FontSize[F_Medium],F_Medium);
  FontSize[F_Large] = S_Data.yres*0.055f;
  LoadFont("fonts/font1.ttf",FontSize[F_Large],F_Large);
  FontSize[F_Digit] = S_Data.yres*0.035f;
  LoadFont("fonts/font1.ttf",FontSize[F_Digit],F_Digit);
}

void CloseFonts()
{
  TTF_CloseFont(SSFont);
  TTF_CloseFont(SFont);
  TTF_CloseFont(LFont);
  TTF_CloseFont(DFont);
}

void LoadFont(char filename[80],int ptsize,int type)
{
  if(TTF_Init()==0)
  {
    atexit(TTF_Quit);
  }
  else
  {
    fprintf(stderr,"Couldn't initialize Font System: %s\n", SDL_GetError());
    return;
  }
  switch(type)
  {
    case F_Small:
      SFont = TTF_OpenFont(filename,ptsize);
      if(SFont == NULL)
      {
        fprintf(stderr,"Couldn't initialize Font: %s\n", SDL_GetError());
        return;
      }
    break;
    case F_Medium:
      SSFont = TTF_OpenFont(filename,ptsize);
      if(SSFont == NULL)
      {
        fprintf(stderr,"Couldn't initialize Font: %s\n", SDL_GetError());
        return;
      }
    case F_Large:
      LFont = TTF_OpenFont(filename,ptsize);
      if(LFont == NULL)
      {
        fprintf(stderr,"Couldn't initialize Font: %s\n", SDL_GetError());
        return;
      }
    case F_Digit:
      DFont = TTF_OpenFont(filename,ptsize);
      if(DFont == NULL)
      {
        fprintf(stderr,"Couldn't initialize Font: %s\n", SDL_GetError());
        return;
      }
    break;
  }
}

void InitMessages()
{
  int i;
  for(i = 0; i < MAXMES; i++)
  {
    strncpy(messages.message[i]," \0",80);
    messages.fadeout[i] = 0;
  }
  messages.defcolor = IndexColor(White);
  messages.sx = 10;
  messages.sy = 10;
}

void MoveMessages(int x,int y,int drawcount)
{
  messages.sx = x;
  messages.sy = y;
  messages.drawnum = drawcount;
}

void NewMsg(char *text)
{
  NewMessage(text,messages.defcolor);
}

void NewMessage(char *text,Uint32 color)
{
  int i,x = -1;
  Uint32 Now = SDL_GetTicks();
  Uint32 best = Now * 2;
  for(i = 0; i < MAXMES;i ++)
  {
    if(messages.fadeout[i] < Now)
    {
      x = i;
      if(messages.count < messages.drawnum)
        messages.count++;
      break;
    }
  }
  if(x == -1) /*everything is in use, so lets hurry along the oldest one, shall we.*/
  {
    for(i = 0; i < MAXMES;i ++)
    {
      if(messages.fadeout[i] < best)
      {
        best = messages.fadeout[i];
        x = i;
      }
    }
  }
  if(x < 0)x = 0;
  strncpy(messages.message[x],text,80);
  messages.fadeout[x] = Now + 3500;
  messages.color[x] = color;
  messages.last = x;
}

void DrawMessages()
{
  int i,j;
  int t; /*target message*/
  int current = 0;
  Uint32  Now = SDL_GetTicks();
  j = messages.drawnum - 1;
  for(i = MAXMES;i > 0;--i)
  {
    t = (messages.last + i)%MAXMES;
    if(messages.fadeout[t] > Now)
    {
      current++;
      DrawTxt(messages.message[t],messages.sx + 1,messages.sy + 1 + (14 * j),IndexColor(DarkGrey),F_Small);
      DrawTxt(messages.message[t],messages.sx,messages.sy + (14 * j--),messages.color[t],F_Small);
      if(current >= messages.drawnum)break;
    }
  }
  if(current <= messages.drawnum)
    messages.count = current;
  else messages.count = messages.drawnum;
}

void DrawTextCenteredShadow(char *text,int sx,int sy,Uint32 color,int size,int offset)
{
  DrawTextCentered(text,sx + offset,sy + offset,IndexColor(DarkGrey),size);
  DrawTextCentered(text,sx,sy,color,size);
}

void DrawTextCentered(char *text,int sx,int sy,Uint32 color,int size)
{
  Coord pos,pos2;
  int w,h;
  Coord center;
  GLuint image;
  SDL_Surface *temp1 = NULL;
  SDL_Surface *temp = NULL;
  SDL_Surface *fontpic = NULL;
  SDL_Color colortype,bgcolor;
  if(strlen(text) <= 0)return;
  SDL_GetRGBA(color, pixelFormat, &colortype.r, &colortype.g, &colortype.b, &colortype.unused);
  bgcolor.r = 0;
  bgcolor.g = 0;
  bgcolor.b = 0;
  bgcolor.unused = SDL_ALPHA_TRANSPARENT;
  switch(size)
  {
    case F_Small:
      if(SFont == NULL)return;
      temp = TTF_RenderText_Blended(SFont, text,colortype);
      break;
    case F_Medium:
      if(SSFont == NULL)return;
      temp = TTF_RenderText_Blended(SSFont, text,colortype);
      break;
    case F_Large:
      if(LFont == NULL)return;
      temp = TTF_RenderText_Blended(LFont, text,colortype);
      break;
    case F_Digit:
      if(DFont == NULL)return;
      temp = TTF_RenderText_Blended(DFont, text,colortype);
      break;
    default:
      return;
  }
 
  w = powerOfTwo(temp->w);
  h = powerOfTwo(temp->h);
  center.x = temp->w>>1;
  center.y =temp->h>>1; 
  /*Creates an opengl compatable RGBA surface*/
  fontpic = SDL_CreateRGBSurface(SDL_HWSURFACE,w, h,S_Data.depth,rmask,gmask,bmask,amask);	
  /*Copies pixel data from the image to surface*/
  temp1 = SDL_DisplayFormatAlpha(temp);
  SDL_SetAlpha(temp1, 0, 0 );
  SDL_BlitSurface(temp1, NULL, fontpic, NULL);
  SDL_FreeSurface(temp1);
  SDL_FreeSurface(temp);

  glGenTextures(1, &image);
  glBindTexture(GL_TEXTURE_2D,image);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, fontpic->pixels);
  SDL_FreeSurface(fontpic);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  sx=sx+w;
  sy=sy+h;
  sx-=center.x;
/*  sy-=center.y;*/
  pos = GetGlCoord(sx,sy,_2DPLANE_,modelview,projection,viewport);
  pos2 = GetGlCoord(sx-w,sy-h,_2DPLANE_,modelview,projection,viewport);
 
  glDisable(GL_DEPTH_TEST);
  

  glBegin( GL_QUADS );

  glTexCoord2f(0.0,0.0);
  glVertex3f(pos2.x,pos2.y,pos.z);

  glTexCoord2f(0.0,1.0);
  glVertex3f( pos2.x,pos.y,pos.z);
	
  glTexCoord2f(1.0,1.0);
  glVertex3f( pos.x,pos.y,pos.z);
	
  glTexCoord2f(1.0,0.0);
  glVertex3f( pos.x,pos2.y,pos.z);
	
  glEnd( );
  glEnable(GL_DEPTH_TEST);

  glDisable(GL_TEXTURE_2D);
  glDeleteTextures(1, &image);
  glDisable(GL_BLEND);
}

void GetTextDimensions(char *text,int size, int *w, int *h)
{
  TTF_Font *font = NULL;
  if((text == NULL)||(text[0] == '\0'))
  {
    return;
  }
  switch(size)
  {
    case F_Small:
      if(SFont == NULL)return;
      font = SFont;
      break;
    case F_Medium:
      if(SSFont == NULL)return;
      font = SSFont;
      break;
    case F_Large:
      if(LFont == NULL)return;
      font = LFont;
      break;
    case F_Digit:
      if(DFont == NULL)return;
      font = DFont;
      break;
    default:
      return;
  }
  TTF_SizeText(font,text, w, h);
}

BBox DrawTxtShadow(char *text,int sx,int sy,Uint32 color,int size,int offset)
{
  BBox b;
  b = DrawTxt(text,sx + offset,sy + offset,IndexColor(DarkGrey),size);
  DrawTxt(text,sx,sy,color,size);
  b.w += offset;
  b.h += offset;
  return b;
}

BBox DrawTxt(char *text,int sx,int sy,Uint32 color,int size)
{
  BBox boxtemp = {0,0,0,0};
  Coord pos,pos2;
  int w,h;
  GLuint image;
  SDL_Surface *temp1 = NULL;
  SDL_Surface *temp = NULL;
  SDL_Surface *fontpic = NULL;
  SDL_Color colortype,bgcolor;
  if(text == NULL)return boxtemp;
  if(strlen(text) <= 0)return boxtemp;/*DJ 6/17/09 makes more stable*/
  SDL_GetRGBA(color, pixelFormat, &colortype.r, &colortype.g, &colortype.b, &colortype.unused);
  bgcolor.r = 0;
  bgcolor.g = 0;
  bgcolor.b = 0;
  boxtemp.x=0;
  boxtemp.y=0;
  boxtemp.z=0;
  boxtemp.w=0;
  boxtemp.h=0;
  boxtemp.d=0;
  bgcolor.unused = SDL_ALPHA_TRANSPARENT;
  switch(size)
  {
    case F_Small:
      if(SFont == NULL)return boxtemp;
      temp = TTF_RenderText_Blended(SFont, text,colortype);
    break;
    case F_Medium:
      if(SSFont == NULL)return boxtemp;
      temp = TTF_RenderText_Blended(SSFont, text,colortype);
    break;
    case F_Large:
      if(LFont == NULL)return boxtemp;
      temp = TTF_RenderText_Blended(LFont, text,colortype);
    break;
    case F_Digit:
      if(DFont == NULL)return boxtemp;
      temp = TTF_RenderText_Blended(DFont, text,colortype);
    break;
    default:
      return boxtemp;
  }
  boxtemp.x=sx;
  boxtemp.y=sy;
  boxtemp.w=temp->w;
  boxtemp.h=temp->h;
  w = powerOfTwo(temp->w);
  h = powerOfTwo(temp->h);

  /*Creates an opengl compatable RGBA surface*/
  fontpic = SDL_CreateRGBSurface(SDL_HWSURFACE,w, h,S_Data.depth,rmask,gmask,bmask,amask);	
  /*Copies pixel data from the image to surface*/
  temp1 = SDL_DisplayFormatAlpha(temp);
  SDL_SetAlpha(temp1, 0, 0 );
  SDL_BlitSurface(temp1, NULL, fontpic, NULL);	
  SDL_FreeSurface(temp1);
  SDL_FreeSurface(temp);

  glGenTextures(1, &image);
  glBindTexture(GL_TEXTURE_2D,image);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, fontpic->pixels);
  SDL_FreeSurface(fontpic);
  glEnable(GL_TEXTURE_2D);
	
  sx=sx+w;
  sy=sy+h;

  pos = GetGlCoord(sx,sy,_2DPLANE_,modelview,projection,viewport);
  pos2 = GetGlCoord(sx-w,sy-h,_2DPLANE_,modelview,projection,viewport);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glBegin( GL_QUADS );

  glTexCoord2f(0.0,0.0);
  glVertex3f(pos2.x,pos2.y,pos.z);

  glTexCoord2f(0.0,1.0);
  glVertex3f( pos2.x,pos.y,pos.z);
	
  glTexCoord2f(1.0,1.0);
  glVertex3f( pos.x,pos.y,pos.z);
	
  glTexCoord2f(1.0,0.0);
  glVertex3f( pos.x,pos2.y,pos.z);
	
  glEnd( );
  glEnable(GL_DEPTH_TEST);
  
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glDeleteTextures(1, &image);
  return boxtemp;
}


void DrawTextZ(char *text,int sx,int sy,float z,Uint32 color,int size)
{
  Coord pos,pos2;
  int w,h;
  Coord center;
  GLuint image;
  SDL_Surface *temp1 = NULL;
  SDL_Surface *temp = NULL;
  SDL_Surface *fontpic = NULL;
  SDL_Color colortype,bgcolor;
  if(strlen(text) <= 0)return;/*DJ 6/17/09 makes more stable*/
    
  SDL_GetRGBA(color, pixelFormat, &colortype.r, &colortype.g, &colortype.b, &colortype.unused);
  bgcolor.r = 0;
  bgcolor.g = 0;
  bgcolor.b = 0;
  bgcolor.unused = SDL_ALPHA_TRANSPARENT;
  switch(size)
  {
    case F_Small:
      if(SFont == NULL)return;
      temp = TTF_RenderText_Blended(SFont, text,colortype);
      break;
    case F_Medium:
      if(SSFont == NULL)return;
      temp = TTF_RenderText_Blended(SSFont, text,colortype);
      break;
    case F_Large:
      if(LFont == NULL)return;
      temp = TTF_RenderText_Blended(LFont, text,colortype);
      break;
    case F_Digit:
      if(LFont == NULL)return;
      temp = TTF_RenderText_Blended(DFont, text,colortype);
      break;
    default:
      return;
  }
 
  w = powerOfTwo(temp->w);
  h = powerOfTwo(temp->h);
  center.x = temp->w>>1;
  center.y =temp->h>>1; 
  /*Creates an opengl compatable RGBA surface*/
  fontpic = SDL_CreateRGBSurface(SDL_HWSURFACE,w, h,S_Data.depth,rmask,gmask,bmask,amask);	
  /*Copies pixel data from the image to surface*/
  temp1 = SDL_DisplayFormatAlpha(temp);
  SDL_SetAlpha(temp1, 0, 0 );
  SDL_BlitSurface(temp1, NULL, fontpic, NULL);	
  SDL_FreeSurface(temp1);
  SDL_FreeSurface(temp);

  glGenTextures(1, &image);
  glBindTexture(GL_TEXTURE_2D,image);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, fontpic->pixels);
  SDL_FreeSurface(fontpic);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  sx=sx+w;
  sy=sy+h;
  sx-=center.x;
  sy-=center.y;
  pos = GetGlCoord(sx,sy,_2DPLANE_,modelview,projection,viewport);
  pos2 = GetGlCoord(sx-w,sy-h,_2DPLANE_,modelview,projection,viewport);
 
  

  glBegin( GL_QUADS );

  glTexCoord2f(0.0,0.0);
  glVertex3f(pos2.x,pos2.y,z);

  glTexCoord2f(0.0,1.0);
  glVertex3f( pos2.x,pos.y,z);
	
  glTexCoord2f(1.0,1.0);
  glVertex3f( pos.x,pos.y,z);
	
  glTexCoord2f(1.0,0.0);
  glVertex3f( pos.x,pos2.y,z);
	
  glEnd( );
  glEnable(GL_DEPTH_TEST);

  glDisable(GL_TEXTURE_2D);
  glDeleteTextures(1, &image);
  glDisable(GL_BLEND);
}

void CropStr(char *text,int length,int strl)
{
  int i;
  for(i = 0;i < strl - length;i++)
  {
    text[i] = text[i + length];
  }
  text[i] = '\0';/*null terminate in case its overwritten*/
}

int GetTextBlockHeight(char *thetext,int size,int width)
{
  char textline[512];
  char temptextline[512];
  char text[512];
  char word[80];
  int drawheight = 0;
  int done = 0;
  int w,h;
  int i;
  int space;
  int lindex = 0;
  TTF_Font *font = NULL;
  if((thetext == NULL)||(thetext[0] == '\0'))
  {
    return 0;
  }
  switch(size)
  {
    case F_Small:
      if(SFont == NULL)return 0;
      font = SFont;
      break;
    case F_Medium:
      if(SSFont == NULL)return 0;
      font = SSFont;
      break;
    case F_Large:
      if(LFont == NULL)return 0;
      font = LFont;
      break;
    case F_Digit:
      if(DFont == NULL)return 0;
      font = DFont;
      break;
    default:
      return 0;
  }
  
  strncpy(text,thetext,512);
  temptextline[0] = '\0';
  do
  {
    space = 0;
    i = 0;
    do
    {
      if(sscanf(&text[i],"%c",&word[0]) == EOF)break;
      if(word[0] == ' ')space++;
      i++;
    }while(word[0] == ' ');
    
    if(sscanf(text,"%s",word) == EOF)
    {
      return (drawheight+h) + h;
    }
    CropStr(text,strlen(word) + 1,512);
    strncpy(textline,temptextline,512);/*keep the last line that worked*/
    for(i = 0;i < (space - 1);i++)
    {
      sprintf(temptextline,"%s%c",temptextline,' '); /*add spaces*/
    }
    sprintf(temptextline,"%s %s",temptextline,word); /*add a word*/
    TTF_SizeText(font, temptextline, &w, &h); /*see how big it is now*/
    lindex += strlen(word);
    if(w > width)         /*see if we have gone over*/
    {
      drawheight += h;
      sprintf(temptextline,"%s",word); /*add a word*/
    }
  }while(!done);
  return drawheight + h;
}

void DrawTextBlock(char *thetext,int sx, int sy,Uint32 color,int size,int width)
{
  char textline[512];
  char temptextline[512];
  char text[512];
  char word[80];
  int drawheight = sy;
  int done = 0;
  int w,h;
  int i;
  int space;
  int lindex = 0;
  TTF_Font *font = NULL;
  if((thetext == NULL)||(thetext[0] == '\0'))
  {
    return;
  }
  switch(size)
  {
    case F_Small:
      if(SFont == NULL)return;
      font = SFont;
      break;
    case F_Medium:
      if(SSFont == NULL)return;
      font = SSFont;
      break;
    case F_Large:
      if(LFont == NULL)return;
      font = LFont;
      break;
    case F_Digit:
      if(DFont == NULL)return;
      font = DFont;
      break;
    default:
      return;
  }

  strncpy(text,thetext,512);
  temptextline[0] = '\0';
  do
  {
    space = 0;
    i = 0;
    do
    {
      if(sscanf(&text[i],"%c",&word[0]) == EOF)break;
      if(word[0] == ' ')space++;
      i++;
    }while(word[0] == ' ');
    if(sscanf(text,"%s",word) == EOF)
    {
      sy=drawheight+h;
      DrawTxt(temptextline,sx,sy, color, size);
      return;
    }

    CropStr(text,strlen(word) + space,512);
    strncpy(textline,temptextline,512);/*keep the last line that worked*/
    for(i = 0;i < (space - 1);i++)
    {
      sprintf(temptextline,"%s%c",temptextline,' '); /*add spaces*/
    }
    sprintf(temptextline,"%s %s",temptextline,word); /*add a word*/
    TTF_SizeText(font, temptextline, &w, &h); /*see how big it is now*/
    lindex += strlen(word);
    if(w > width)         /*see if we have gone over*/
    {
      sy=drawheight+h;
      DrawTxt(textline,sx,sy, color, size);

      /*draw the line and get ready for the next line*/
      drawheight += h;
      sprintf(temptextline,"%s",word); /*add a word*/
    }
  }while(!done);
}

TTF_Font *Load_Font(char *filename,int ptsize)
{
  TTF_Font *font = NULL;
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
    fprintf(stderr,"Unable to allocate space for loading music: %s\n",filename);
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
  font = TTF_OpenFontRW(rw,0,ptsize);
  SDL_FreeRW(rw);
  free(buffer);
  PHYSFS_close(file);
  return font;
}


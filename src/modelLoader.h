#ifndef _MODELLOADER_
#define _MODELLOADER_

#include "graphics.h"
#include "armatures.h" 

#define MAXZFMMODELS     128

typedef struct VECT3f
{
  GLfloat vert[3];
}vect3f;

typedef struct VECT2f
{
  GLfloat vert[2];
}vect2f;

typedef struct VECT3I
{
  GLint vert1[3];
  GLint vert2[3];
  GLint vert3[3];
}face3i;

typedef struct OBJREAD 
{
  char    filename[80];

  int numvertices;
  int numnormals;
  int numtexcoords;
  int numfaces;

  vect3f *vertices;
  vect3f *normals;
  vect2f *texcoords;
  face3i *faces;
  
}obj_read;

typedef struct ZFMFILE
{
  char    filename[80];

  int numvertices;
  int numnormals;
  int numtexcoords;
  int numfaces;

  vect3f *vertices;
  vect3f *normals;
  vect2f *texcoords;
  
  vect3f *verticesAtRest;
  vect3f *verticesDeformed;
  face3i *faces;
  
  int *indexarray;
  int numindex;
  
  GLuint vertexArray;
  GLuint normalArray;
  GLuint uvArray;
  
  Armature *skeleton;
  int currentFrameInMemory;
  
  float dimen[3];
  float center[3];
  
  int used;
  
}zfmModel;

obj_read *readOBJ(char *filename);
void objReadFirstPass(obj_read* model, FILE* file);
void objReadSecondPass(obj_read* model, FILE* file);

void saveBinaryModelFile(obj_read *model);
zfmModel *zfmLoadModel(char *filename);
zfmModel *zfmLoadModelWithArmature(char *filename,char *armfilename);
void InitZfmModelList();
void CloseZfmModels();
void freeZfmModel(zfmModel* model);

void zfmDrawModel(zfmModel *model, Armature *arm, Sprite *colortexture, GLuint shaderProgram,float x,float y,float z, float rotx,float roty,float rotz,float scalex,float scaley,float scalez, int drawMode, int frame, float alpha);
void zfmDrawModelNoBuff(zfmModel *model, Armature *arm, Sprite *colortexture, GLuint shaderProgram,float x,float y,float z, float rotx,float roty,float rotz,float scalex,float scaley,float scalez, int drawMode, int frame, float alpha);

void zfmSetupArmatureSpace(zfmModel *model, Armature *arm);
void zfmDeformMesh(zfmModel *model, Armature *arm, int frame);
void zfmDeformMeshNoBuff(zfmModel *model, Armature *arm, int frame);
void zfmShiftUV(zfmModel *model,float offx,float offy);

void getDimen(zfmModel *model);
void zfmScaleModel(zfmModel *model, float scale);

zfmModel *zfmLoadModelScaled(char *filename, float scale);
#endif


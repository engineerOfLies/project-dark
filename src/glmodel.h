#ifndef _GLMODEL_
#define _GLMODEL_


#include "glm.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"


typedef struct Model_T
{
  GLMmodel *object[MAX_FRAMES];
  GLMmodel *temp;
  GLfloat uvscale[2];
  GLfloat uvoffest[2];
  int numFrames;
  GLfloat dimensions[3];
  GLfloat center[3];
  char filename[50];
  int used;
}Model;

/*Loading and Drawing models*/
void InitModelList();
void CloseModels();
void FreeModel(Model *model);
Model *LoadModel(char *filename, int numFrames,int center);


/*the grand unified string theory for model drawing  does not push/pop.  Must be done*/
void DrawModelBase(Model *model, Sprite *texture, Coord pos1, Coord rot, Coord scale, float keyframe1, int keyframe2, float alpha, Coord Colorshift,int drawtype);

void centerModel(GLMmodel* model);
void centerModelNoZ(GLMmodel *model);
void centerUV(GLMmodel*model);

Model* keyFrameModel(Model *model, float frame,int keyframe1, int keyframe2);
/*the normal use DrawModel*/
void DrawModel(Model *model, Sprite *texture, Coord pos1, Coord rot, Coord scale, float keyframe1, int keyframe2, float alpha, Coord Colorshift);
GLMgroup *GetGroupByName(GLMmodel *model,char name[40]);
void ShiftDrawModel(Model *model,Sprite *texture, float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz,float alpha,float offx, float offy, float frame);
void DrawModelUVScaleShift(Model *model,Sprite *texture, Coord p, Coord r, Coord s,float offx, float offy,float scalex, float scaley,float alpha, float frame);

#endif

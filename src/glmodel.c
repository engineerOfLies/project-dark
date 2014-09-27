#include "glmodel.h"
#include "lighting.h"

#define MaxModels     255
Model ModelList[MaxModels];
extern GLint viewport[4];
extern GLdouble modelview[16];
extern GLdouble projection[16];
extern GLuint ShaderProg;

extern int OpenGLOn;
int NumModels;
float globalZero;
/*some data on the video settings that can be useful for a lot of functions*/
extern Uint32 rmask,gmask,bmask,amask;
extern ScreenData  S_Data;


/*
LoadModel: Loads .obj model files

char *filename = takes in the location of the obj, if there is only one frame of the model
put the entire filename including the .obj. If there are more than one frames just type in 
the base name minus the '_' and the number. For example for "models/dorumon_textured_000001.obj"
just put in "models/dorumon_textured"

char *texturefile = the path to the texture image

int texw, int texh = the width and hieght of the texure image

int numFrames = The number of frames starting from 0, so if your model has only one frame, just put 0;

int center = 1, If the model should be centered 2 if both the model and texture should be centered

*/
Model *LoadModel(char *filename, int numFrames,int center)
{

  int i,r;
  char temp[50];

  /*first search to see if the requested model is already loaded*/
  for(i = 0; i < MaxModels; i++)
  {
    if(strncmp(filename,ModelList[i].filename,50)==0)
    {
      ModelList[i].used++;
      return &ModelList[i];
    }
  }
  /*makesure we have the room for a new model*/
  if(NumModels + 1 >= MaxModels)
  {
    fprintf(stderr, "Maximum Models Reached.\n");
    exit(1);
  }
  /*if its not already in memory, then load it.*/
  NumModels++;
  for(i = 0;i <= MaxModels;i++)
  {
    if(!ModelList[i].used)break;
  }
  strncpy(ModelList[i].filename,filename,50);

  ModelList[i].used++;
  ModelList[i].numFrames = numFrames + 1;
  if(numFrames==0){
    ModelList[i].object[0] =glmReadOBJ(filename);
    if(ModelList[i].object[0] == NULL)
    {
      fprintf(stderr,"unable to load obj file!");
      return NULL;
    }
    glmDimensions(ModelList[i].object[0],ModelList[i].dimensions);
    if(center)centerModel(ModelList[i].object[0]);
    ModelList[i].center[0]=ModelList[i].dimensions[0]/2.0f;
    ModelList[i].center[1]=ModelList[i].dimensions[1]/2.0f;
    ModelList[i].center[2]=ModelList[i].dimensions[2]/2.0f;
  }else
  {
    for(r=0; r < ModelList[i].numFrames;r++)
    {
      sprintf(temp,"%s_%06i.obj",filename,r+1);
      ModelList[i].object[r] =glmReadOBJ(temp);
      if(ModelList[i].object[r] == NULL)
      {
        fprintf(stderr,"unable to load obj file!");
        return NULL;
      }
      glmDimensions(ModelList[i].object[r],ModelList[i].dimensions);
      if(center)centerModel(ModelList[i].object[r]);
      ModelList[i].center[0]=ModelList[i].dimensions[0]/2.0f;
      ModelList[i].center[1]=ModelList[i].dimensions[1]/2.0f;
      ModelList[i].center[2]=ModelList[i].dimensions[2]/2.0f;
    }
  }
  if(numFrames!=0)
  {
    ModelList[i].temp =glmReadOBJ(temp);
    glmDimensions(ModelList[i].temp,ModelList[i].dimensions);
    if(center)centerModel(ModelList[i].temp);
    ModelList[i].center[0]=ModelList[i].dimensions[0]/2.0f;
    ModelList[i].center[1]=ModelList[i].dimensions[1]/2.0f;
    ModelList[i].center[2]=ModelList[i].dimensions[2]/2.0f;
  }
  return &ModelList[i];

}

void InitModelList()
{
  NumModels = 0;
  memset(ModelList,0,sizeof(Model) * MaxModels);

}

void FreeModel(Model *model)
{
  int i;
  model->used--;
  if(model->used == 0)
  {
    strcpy(model->filename,"\0");
    for(i=0;i <= model->numFrames;i++)
    {
      glmDelete(model->object[i]);
    }
  }
}

void CloseModels()
{
  int i,j;
  for(j = 0;j < MaxModels;j++)
  {
    for(i=0;i <= ModelList[j].numFrames;i++)
    {
      if(ModelList[j].object[i] != NULL)glmDelete(ModelList[j].object[i]);
    }
  }
}



/*
  This is to be drawn within the push/pop
*/
void DrawModelBase(Model *model, Sprite *texture, Coord pos1, Coord rot, Coord scale, float keyframe1, int keyframe2, float alpha, Coord Colorshift, int drawtype)
{
  int i;
  GLMgroup* group;
  GLMtriangle* triangle;
  Model *temp;
  float trans[]={1.0f,1.0f,1.0f,1.0f};
 
  trans[3] = alpha;
  trans[0] = Colorshift.x;
  trans[1] = Colorshift.y;
  trans[2] = Colorshift.z;
  
  if(keyframe1 != keyframe2)
  {
    temp = keyFrameModel(model, keyframe1,keyframe1,keyframe2);
  }
  else
  {
    temp = model;
    temp->temp = model->object[(int)keyframe1];
  }
  
  glColorMaterial(GL_FRONT,GL_DIFFUSE);
  glEnable(GL_NORMALIZE);
  glEnable(GL_LIGHTING);
  glEnable(GL_COLOR_MATERIAL);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND); 
  
  if(drawtype & 1)  glDepthFunc(GL_LEQUAL);

  if(texture != NULL)
  {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,texture->image);
  }
  
  glMaterialfv(GL_FRONT,GL_DIFFUSE,trans);

  glTranslatef(pos1.x,pos1.y,pos1.z);
  glRotatef(rot.x, 1.0f, 0.0f, 0.0f);
  glRotatef(rot.y, 0.0f, 1.0f, 0.0f);
  glRotatef(rot.z, 0.0f, 0.0f, 1.0f);
  glScalef(scale.x,scale.y,scale.z);
  

  
  group = temp->temp->groups;

  while (group) 
  {
    glBegin(GL_TRIANGLES);
    for (i = 0; i < group->numtriangles; i++) 
    {
      glColor3f(Colorshift.x,Colorshift.y,Colorshift.z);
      
      triangle = &temp->temp->triangles[group->triangles[i]];
  
      glNormal3fv(&temp->temp->normals[3 * triangle->nindices[0]]);
      glTexCoord2fv(&temp->temp->texcoords[2 * triangle->tindices[0]]);
      glVertex3fv(&temp->temp->vertices[3 * triangle->vindices[0]]);
      glNormal3fv(&temp->temp->normals[3 * triangle->nindices[1]]);
      glTexCoord2fv(&temp->temp->texcoords[2 * triangle->tindices[1]]);
      glVertex3fv(&temp->temp->vertices[3 * triangle->vindices[1]]);
      glNormal3fv(&temp->temp->normals[3 * triangle->nindices[2]]);
      glTexCoord2fv(&temp->temp->texcoords[2 * triangle->tindices[2]]);
      glVertex3fv(&temp->temp->vertices[3 * triangle->vindices[2]]);
      
    }
    glEnd();
    group = group->next;
  }

  glDisable(GL_LIGHTING);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_BLEND);
  if(texture != NULL)
  {
    glDisable(GL_TEXTURE_2D);
  }
glDisable(GL_NORMALIZE);
  if(drawtype &= 1)  glDepthFunc(GL_LESS);

  temp = NULL;

}

void DrawModel(Model *model, Sprite *texture, Coord pos1, Coord rot, Coord scale, float keyframe1, int keyframe2, float alpha, Coord Colorshift)
{
  glPushMatrix();
  DrawModelBase(model, texture, pos1, rot, scale, keyframe1, keyframe2, alpha, Colorshift,0);
  glPopMatrix();
}

void ShiftDrawModel(Model *model,Sprite *texture, float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz,float alpha,float offx, float offy, float frame)
{
  int i;
  int keyframe1,keyframe2;
  GLMgroup* group;
  GLMtriangle* triangle;
  Model *temp;
  float trans[]={1.0f,1.0f,1.0f,1.0f};
  if(model == NULL)return;
  glPushMatrix();
  trans[3] = alpha;
  trans[0] = 1;
  trans[1] = 1;
  trans[2] = 1;
  
  keyframe1 = (int)floor(frame);
  keyframe2 = (int)ceil(frame);
  if(keyframe2 > model->numFrames)
  {
    frame -= keyframe1;
    keyframe1 = 0;
    keyframe2 = 1;
  }
  if(keyframe1 != keyframe2)
  {
    temp = keyFrameModel(model, frame,keyframe1,keyframe2);
  }
  else
  {
    temp = model;
    temp->temp = model->object[(int)keyframe1];
  }
  glPushAttrib(GL_LIGHTING_BIT);  
  
  glColorMaterial(GL_FRONT,GL_DIFFUSE);
  glEnable(GL_NORMALIZE);
  glEnable(GL_LIGHTING);
  glEnable(GL_COLOR_MATERIAL);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND); 
  glUseProgram(ShaderProg);
  

  if(texture != NULL)
  {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,texture->image);
  }
  
  glMaterialfv(GL_FRONT,GL_DIFFUSE,trans);
  glMaterialfv(GL_FRONT,GL_SPECULAR,trans);
  glMateriali(GL_FRONT,GL_SHININESS,128);

  glTranslatef(x,y,z);
  glRotatef(rx, 1.0f, 0.0f, 0.0f);
  glRotatef(ry, 0.0f, 1.0f, 0.0f);
  glRotatef(rz, 0.0f, 0.0f, 1.0f);
  glScalef(sx,sy,sz);
  

  
  group = temp->temp->groups;

  while (group) 
  {
    glBegin(GL_TRIANGLES);
    for (i = 0; i < group->numtriangles; i++) 
    {
      glColor3f(1,1,1);
      
      triangle = &temp->temp->triangles[group->triangles[i]];
  
      glNormal3fv(&temp->temp->normals[3 * triangle->nindices[0]]);
      glTexCoord2f(temp->temp->texcoords[2 * triangle->tindices[0]] + offx,temp->temp->texcoords[(2 * triangle->tindices[0]) + 1] + offy);
      glVertex3fv(&temp->temp->vertices[3 * triangle->vindices[0]]);
      glNormal3fv(&temp->temp->normals[3 * triangle->nindices[1]]);
      glTexCoord2f(temp->temp->texcoords[2 * triangle->tindices[1]] + offx,temp->temp->texcoords[(2 * triangle->tindices[1]) + 1] + offy);
      glVertex3fv(&temp->temp->vertices[3 * triangle->vindices[1]]);
      glNormal3fv(&temp->temp->normals[3 * triangle->nindices[2]]);
      glTexCoord2f(temp->temp->texcoords[2 * triangle->tindices[2]] + offx,temp->temp->texcoords[(2 * triangle->tindices[2]) + 1] + offy);
      glVertex3fv(&temp->temp->vertices[3 * triangle->vindices[2]]);
      
    }
    glEnd();
    group = group->next;
  }
  glUseProgram(0);
  glDisable(GL_LIGHTING);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_BLEND);
  if(texture != NULL)
  {
    glDisable(GL_TEXTURE_2D);
  }
  glDisable(GL_NORMALIZE);
  glPopMatrix();
  temp = NULL;
}


void DrawModelUVScaleShift(Model *model,Sprite *texture, Coord p, Coord r, Coord s,float offx, float offy,float scalex, float scaley,float alpha, float frame)
{
  int i;
  int keyframe1,keyframe2;
  GLMgroup* group;
  GLMtriangle* triangle;
  Model *temp;
  float trans[]={1.0f,1.0f,1.0f,1.0f};
  float skinx,skiny;
  if(model == NULL)return;
  glPushMatrix();
  trans[3] = alpha;
  trans[0] = 1;
  trans[1] = 1;
  trans[2] = 1;
  keyframe1 = (int)floor(frame);
  keyframe2 = (int)ceil(frame);
  if(keyframe2 > model->numFrames)
  {
    frame -= keyframe1;
    keyframe1 = 0;
    keyframe2 = 1;
  }
  if(keyframe1 != keyframe2)
  {
    temp = keyFrameModel(model, frame,keyframe1,keyframe2);
  }
  else
  {
    temp = model;
    temp->temp = model->object[(int)keyframe1];
  }

  skinx = (temp->temp->uvcenter[0] - (temp->temp->uvcenter[0] * scalex));
  skiny = (temp->temp->uvcenter[1] - (temp->temp->uvcenter[1] * scaley));
  glPushAttrib(GL_LIGHTING_BIT);  
  
  glColorMaterial(GL_FRONT,GL_DIFFUSE);
  glEnable(GL_NORMALIZE);
  glEnable(GL_LIGHTING);
  glEnable(GL_COLOR_MATERIAL);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND); 
 /* glUseProgram(ShaderProg);*/
  

  if(texture != NULL)
  {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,texture->image);
  }
  
  glMaterialfv(GL_FRONT,GL_DIFFUSE,trans);
  glMaterialfv(GL_FRONT,GL_SPECULAR,trans);
  glMateriali(GL_FRONT,GL_SHININESS,128);

  glTranslatef(p.x,p.y,p.z);
  glRotatef(r.x, 1.0f, 0.0f, 0.0f);
  glRotatef(r.y, 0.0f, 1.0f, 0.0f);
  glRotatef(r.z, 0.0f, 0.0f, 1.0f);
  glScalef(s.x,s.y,s.z);
  

  
  group = temp->temp->groups;

  while (group) 
  {
    glBegin(GL_TRIANGLES);
    for (i = 0; i < group->numtriangles; i++) 
    {
      glColor3f(1,1,1);
      
      triangle = &temp->temp->triangles[group->triangles[i]];
  
      glNormal3fv(&temp->temp->normals[3 * triangle->nindices[0]]);
      glTexCoord2f((temp->temp->texcoords[2 * triangle->tindices[0]] * scalex) + offx + skinx,(temp->temp->texcoords[(2 * triangle->tindices[0]) + 1] * scaley) + offy + skiny);
      glVertex3fv(&temp->temp->vertices[3 * triangle->vindices[0]]);
      glNormal3fv(&temp->temp->normals[3 * triangle->nindices[1]]);
      glTexCoord2f((temp->temp->texcoords[2 * triangle->tindices[1]] * scalex) + offx + skinx,(temp->temp->texcoords[(2 * triangle->tindices[1]) + 1] * scaley) + offy + skiny);
      glVertex3fv(&temp->temp->vertices[3 * triangle->vindices[1]]);
      glNormal3fv(&temp->temp->normals[3 * triangle->nindices[2]]);
      glTexCoord2f((temp->temp->texcoords[2 * triangle->tindices[2]] * scalex) + offx + skinx,(temp->temp->texcoords[(2 * triangle->tindices[2]) + 1] * scaley) + offy + skiny);
      glVertex3fv(&temp->temp->vertices[3 * triangle->vindices[2]]);
      
    }
    glEnd();
    group = group->next;
  }
  glUseProgram(0);
  glDisable(GL_LIGHTING);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_BLEND);
  if(texture != NULL)
  {
    glDisable(GL_TEXTURE_2D);
  }
  glDisable(GL_NORMALIZE);
  glPopMatrix();
  temp = NULL;
}

void centerUV(GLMmodel*model)
{
  GLuint  i;
  GLfloat cx,cy;
  cx = 0.5 - model->uvcenter[0];
  cy = 0.5 - model->uvcenter[1];
  for(i = 0;i <= model->numtexcoords;i++)
  {
    model->texcoords[2 * i + 0] += cx;
    model->texcoords[2 * i + 1] += cy;
  }
  model->uvcenter[0] = 0.5;
  model->uvcenter[1] = 0.5;
}

void centerModelNoZ(GLMmodel *model)
{
  GLuint  i;
  GLfloat maxx, maxy;
  GLfloat cx, cy;

  /* get the max/mins */
  maxx = 0;
  maxy = 0;
  for (i = 0; i <= model->numvertices; i++)
  {
    maxx += model->vertices[3 * i + 0];
    maxy += model->vertices[3 * i + 1];
  }
  /* calculate center of the model */
  cx = (maxx / (model->numvertices + 1));
  cy = (maxy / (model->numvertices + 1));

  /* translate around center then scale */
  for (i = 1; i <= model->numvertices; i++)
  {
    model->vertices[3 * i    ] -= cx;
    model->vertices[3 * i + 1] -= cy;

  }
}

void centerModel(GLMmodel* model)
{
  GLuint  i;
  GLfloat maxx, minx, maxy, miny, maxz, minz;
  GLfloat cx, cy, cz, w, h, d;

  /* get the max/mins */
  maxx = minx = model->vertices[3 + 0];
  maxy = miny = model->vertices[3 + 1];
  maxz = minz = model->vertices[3 + 2];
  for (i = 1; i <= model->numvertices; i++) {
    if (maxx < model->vertices[3 * i + 0])
      maxx = model->vertices[3 * i + 0];
    if (minx > model->vertices[3 * i + 0])
      minx = model->vertices[3 * i + 0];
        
    if (maxy < model->vertices[3 * i + 1])
      maxy = model->vertices[3 * i + 1];
    if (miny > model->vertices[3 * i + 1])
      miny = model->vertices[3 * i + 1];
        
    if (maxz < model->vertices[3 * i + 2])
      maxz = model->vertices[3 * i + 2];
    if (minz > model->vertices[3 * i + 2])
      minz = model->vertices[3 * i + 2];
  }
    
  /* calculate model width, height, and depth */
  w = maxx - minx;
  h = maxy - miny;
  d = maxz - minz;
    
  /* calculate center of the model */
  cx = (maxx + minx) * .5;
  cy = (maxy + miny) * .5;
  cz = (maxz + minz) * .5;

  /* translate around center then scale */
  for (i = 1; i <= model->numvertices; i++) {
    model->vertices[3 * i    ] -= cx;
    model->vertices[3 * i + 1] -= cy;
    model->vertices[3 * i + 2] -= cz;

  }
}


Model* keyFrameModel(Model *model, float frame,int keyframe1, int keyframe2)
{
  GLMgroup* group1;
  GLMgroup* group2;
  GLMtriangle* triangle1;
  GLMtriangle* triangle2;
  float num1;
  float num2;
  Model *temp;
  int i;
  
  temp = model;
  if(keyframe1 == keyframe2)
  {
    temp = model;
    temp->temp = model->object[keyframe1];
    return temp;
  }
  num2 = frame - keyframe1;
  num1 = 1.0f - num2;
  group1 = model->object[keyframe1]->groups;
  group2 = model->object[keyframe2]->groups;
  while (group1) 
  {
    for (i = 0; i < group1->numtriangles; i++) 
    {
      triangle1 = &model->object[keyframe1]->triangles[group1->triangles[i]];
      triangle2 = &model->object[keyframe2]->triangles[group2->triangles[i]];

      temp->temp->vertices[3 * triangle1->vindices[0]]=(model->object[keyframe1]->vertices[3 * triangle1->vindices[0]]*num1) + (model->object[keyframe2]->vertices[3 * triangle2->vindices[0]]*num2);
      temp->temp->vertices[3 * triangle1->vindices[1]]=(model->object[keyframe1]->vertices[3 * triangle1->vindices[1]]*num1) + (model->object[keyframe2]->vertices[3 * triangle2->vindices[1]]*num2);
      temp->temp->vertices[3 * triangle1->vindices[2]]=(model->object[keyframe1]->vertices[3 * triangle1->vindices[2]]*num1) + (model->object[keyframe2]->vertices[3 * triangle2->vindices[2]]*num2);

    }
    group1 = group1->next;
    group2 = group2->next;
  }
  return temp;
}

GLMgroup *GetGroupByName(GLMmodel *model,char name[40])
{
  GLMgroup *group;
  group = model->groups;
  while (group) 
  {
    if(strncmp(group->name,name,40)==0)
    return group;
    group = group->next;
  }
  return NULL;/*group does not exist*/
}


/*end of file*/

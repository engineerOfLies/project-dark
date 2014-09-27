#include "modelLoader.h"
 
zfmModel zfmModelList[MAXZFMMODELS];
int numZfmModels;

obj_read *readOBJ(char *filename)
{
  obj_read *model;
  FILE *file;

  file = fopen(filename, "r");
  if (!file)
  {
    fprintf(stderr, "failed to open file: %s\n",filename);
    return NULL;
  }
  model = (obj_read*)malloc(sizeof(obj_read));
  strcpy(model->filename,filename);
  
  objReadFirstPass(model,file);
  
  if(model->numvertices != 0) model->vertices = (vect3f*)malloc(sizeof(vect3f)*(model->numvertices));
  if(model->numnormals != 0) model->normals = (vect3f*)malloc(sizeof(vect3f)*(model->numnormals));
  if(model->numtexcoords != 0) model->texcoords = (vect2f*)malloc(sizeof(vect2f)*(model->numtexcoords));
  if(model->numfaces != 0) model->faces = (face3i*)malloc(sizeof(face3i)*(model->numfaces));

  rewind(file);
  
  objReadSecondPass(model,file);
  
  fclose(file);
  return model;
}

void objReadFirstPass(obj_read* model, FILE* file)
{
  char buf[128];
  int  numvertices = 0;
  int  numnormals = 0;
  int  numtexcoords = 0;
  int  numfaces = 0;
  
  while(fscanf(file, "%s", buf) != EOF)
  {
    switch(buf[0]) 
    {
      case 'v':
        switch(buf[1]) 
        {
          case '\0':
            fgets(buf, sizeof(buf), file);
            numvertices++;
            break;
          case 'n':
            fgets(buf, sizeof(buf), file);
            numnormals++;
            break;
          case 't':
            fgets(buf, sizeof(buf), file);
            numtexcoords++;
            break;
          default:
            break;
        }
        break;
      case 'f':
        fgets(buf, sizeof(buf), file);
        numfaces++;
        break;
      default:
        fgets(buf, sizeof(buf), file);
        break;
    }
  }
  
  model->numvertices  = numvertices+1;
  model->numnormals   = numnormals+1;
  model->numtexcoords = numtexcoords+1;
  model->numfaces = numfaces+1;
  
}

void objReadSecondPass(obj_read* model, FILE* file)
{
  int  numvertices = 1;
  int  numnormals = 1;
  int  numtexcoords = 1;
  int  numfaces = 1;
  char buf[128];
  
  while(fscanf(file, "%s", buf) != EOF)
  {
    switch(buf[0]) 
    {
      case 'v':
        switch(buf[1]) 
        {
          case '\0':
            fscanf(file, "%f %f %f", &model->vertices[numvertices].vert[0], &model->vertices[numvertices].vert[1],&model->vertices[numvertices].vert[2]);
            numvertices++;
            break;
          case 'n':
            fscanf(file, "%f %f %f", &model->normals[numnormals].vert[0], &model->normals[numnormals].vert[1],&model->normals[numnormals].vert[2]);
            numnormals++;
            break;
          case 't':
            fscanf(file, "%f %f", &model->texcoords[numtexcoords].vert[0], &model->texcoords[numtexcoords].vert[1]);
            model->texcoords[numtexcoords].vert[1] = 1 - model->texcoords[numtexcoords].vert[1];
            numtexcoords++;
            break;
          default:
            break;
        }
        break;
      case 'f':
        fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d", &model->faces[numfaces].vert1[0], &model->faces[numfaces].vert1[1], &model->faces[numfaces].vert1[2], &model->faces[numfaces].vert2[0], &model->faces[numfaces].vert2[1], &model->faces[numfaces].vert2[2], &model->faces[numfaces].vert3[0], &model->faces[numfaces].vert3[1], &model->faces[numfaces].vert3[2]);
        numfaces++;
        break;
      default:
        fgets(buf, sizeof(buf), file);
        break;
    }
  }
}

void saveBinaryModelFile(obj_read *model)
{
  FILE *file;
  char  filename[80];
  int l;
  if(model == NULL) exit(0);
  l = strlen(model->filename);
  model->filename[l -4] = '\0';
  sprintf(filename,"%s.zfm",model->filename);
  
  file = fopen(filename,"wb");
  if(file==NULL)
  {
    exit(0);
  }
  if(fwrite(filename,80*sizeof(char),1,file) != 1)
  {
    exit(0);
  }
  if(fwrite(&model->numfaces,sizeof(int),1,file) != 1)
  {
    exit(0);
  }
  if(fwrite(&model->numvertices,sizeof(int),1,file) != 1)
  {
    exit(0);
  }
  if(fwrite(&model->numnormals,sizeof(int),1,file) != 1)
  {
    exit(0);
  }
  if(fwrite(&model->numtexcoords,sizeof(int),1,file) != 1)
  {
    exit(0);
  }
  if(fwrite(model->vertices,model->numvertices*sizeof(vect3f),1,file) != 1)
  {
    exit(0);
  }
  if(fwrite(model->normals,model->numnormals*sizeof(vect3f),1,file) != 1)
  {
    exit(0);
  }
  if(fwrite(model->texcoords,model->numtexcoords*sizeof(vect2f),1,file) != 1)
  {
    exit(0);
  }
  if(fwrite(model->faces,model->numfaces*sizeof(face3i),1,file) != 1)
  {
    exit(0);
  }

  fclose(file);	
}

zfmModel *zfmLoadModel(char *filename)
{
  FILE *file;
  char modelname[80];
  int numfaces;
  int numvertices;
  int numnormals;
  int numtexcoords;
  vect3f *vertices = NULL;
  vect3f *normals = NULL;
  vect2f *texcoords = NULL;
  face3i *faces = NULL;
  int i;
  int k;
  int index;
  
  /*first search to see if the requested model is alreday loaded*/
  for(k = 0; k < MAXZFMMODELS; k++)
  {
    if(strcmp(filename,zfmModelList[k].filename)==0)
    {
      zfmModelList[k].used++;
      return &zfmModelList[k];
    }
  }
  /*makesure we have the room for a new model*/
  if(numZfmModels + 1 >= MAXZFMMODELS)
  {
    fprintf(stderr, "Maximum Models Reached.\n");
    exit(1);
  }
  /*if its not already in memory, then load it.*/
  numZfmModels++;
  for(k = 0;k <= MAXZFMMODELS;k++)
  {
    if(!zfmModelList[k].used)break;
  }
  zfmModelList[k].used++;
  strcpy(zfmModelList[k].filename,filename);
  file = fopen(filename,"rb");
  if(file==NULL)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
  }
  if(fread(&modelname,80*sizeof(char),1,file) != 1)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
  }
  if(fread(&numfaces,sizeof(int),1,file) != 1)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
  }
  if(fread(&numvertices,sizeof(int),1,file) != 1)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
  }
  if(fread(&numnormals,sizeof(int),1,file) != 1)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
  }
  if(fread(&numtexcoords,sizeof(int),1,file) != 1)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
  }
  if(numvertices > 0)
  {
    vertices = (vect3f*)malloc(sizeof(vect3f)*numvertices);
    if(fread(vertices,numvertices*sizeof(vect3f),1,file) != 1)
    {
      fprintf(stderr, "failed to load vertices %s.\n",filename);
    }
  }
  if(numnormals > 0)
  {
    normals = (vect3f*)malloc(sizeof(vect3f)*numnormals);
    if(fread(normals,numnormals*sizeof(vect3f),1,file) != 1)
    {
      fprintf(stderr, "failed to load normals: %s.\n",filename);
    }
  }
  if(numtexcoords > 0)
  {
    texcoords = (vect2f*)malloc(sizeof(vect2f)*numtexcoords);
    if(fread(texcoords,numtexcoords*sizeof(vect2f),1,file) != 1)
    {
      fprintf(stderr, "failed to load tex coords: %s.\n",filename);
    }
  }
  if(numfaces > 0)
  {
    faces = (face3i*)malloc(sizeof(face3i)*numfaces);
    if(fread(faces,numfaces*sizeof(face3i),1,file) != 1)
    {
      fprintf(stderr, "failed to load faces: %s.\n",filename);
    }
  }
  fclose(file);
  zfmModelList[k].numvertices=numvertices;
  zfmModelList[k].numfaces=numfaces;
  zfmModelList[k].faces = (face3i*)malloc(sizeof(face3i)*(zfmModelList[k].numfaces));
  memcpy( zfmModelList[k].faces, faces, sizeof(face3i) * numfaces);
  zfmModelList[k].verticesAtRest = (vect3f*)malloc(sizeof(vect3f) * numvertices);
  zfmModelList[k].verticesDeformed = (vect3f*)malloc(sizeof(vect3f) * numvertices);
  memcpy( zfmModelList[k].verticesAtRest, vertices, sizeof(vect3f) * numvertices);
  zfmModelList[k].vertices = (vect3f*)malloc(sizeof(vect3f) * ((numfaces-1)*3));
  zfmModelList[k].normals = (vect3f*)malloc(sizeof(vect3f) * ((numfaces-1)*3));
  zfmModelList[k].texcoords = (vect2f*)malloc(sizeof(vect2f) * ((numfaces-1)*3));
  zfmModelList[k].indexarray = (int*)malloc(sizeof(int) * ((numfaces-1)*3));
  
  index=0;
  for(i = 1; i < (numfaces);i++)
  {
    zfmModelList[k].indexarray[index] = index;
    zfmModelList[k].vertices[index] = vertices[faces[i].vert1[0]];
    zfmModelList[k].normals[index] = normals[faces[i].vert1[2]];
    zfmModelList[k].texcoords[index] = texcoords[faces[i].vert1[1]];
    index++;
    zfmModelList[k].indexarray[index] = index;
    zfmModelList[k].vertices[index] = vertices[faces[i].vert2[0]];
    zfmModelList[k].normals[index] = normals[faces[i].vert2[2]];
    zfmModelList[k].texcoords[index] = texcoords[faces[i].vert2[1]];
    index++;
    zfmModelList[k].indexarray[index] = index;
    zfmModelList[k].vertices[index] = vertices[faces[i].vert3[0]];
    zfmModelList[k].normals[index] = normals[faces[i].vert3[2]];
    zfmModelList[k].texcoords[index] = texcoords[faces[i].vert3[1]];
    index++;
  }

  getDimen(&zfmModelList[k]);
  if(vertices)free(vertices);
  vertices=NULL;
  if(normals)free(normals);
  normals=NULL;
  if(texcoords)free(texcoords);
  texcoords=NULL;
  if(faces)free(faces);
  faces=NULL;

  zfmModelList[k].numindex = index;
      
  glGenBuffers( 1, &zfmModelList[k].vertexArray);			
  /*bind it to GL_ARRAY_BUFFER */
  glBindBuffer( GL_ARRAY_BUFFER, zfmModelList[k].vertexArray);		
  /*apply the array data to the buffer sending it to the graphics card */
  glBufferData( GL_ARRAY_BUFFER, zfmModelList[k].numindex*sizeof(vect3f), zfmModelList[k].vertices, GL_STATIC_DRAW);
  
  /*first generate a buffer index*/
  glGenBuffers( 1, &zfmModelList[k].normalArray);			
  /*bind it to GL_ARRAY_BUFFER */
  glBindBuffer( GL_ARRAY_BUFFER, zfmModelList[k].normalArray);
  /*apply the array data to the buffer sending it to the graphics card */
  glBufferData( GL_ARRAY_BUFFER, zfmModelList[k].numindex*sizeof(vect3f), zfmModelList[k].normals, GL_STATIC_DRAW);
  
  /*first generate a buffer index*/
  glGenBuffers( 1, &zfmModelList[k].uvArray);			
  /*bind it to GL_ARRAY_BUFFER */
  glBindBuffer( GL_ARRAY_BUFFER, zfmModelList[k].uvArray);		
  /*apply the array data to the buffer sending it to the graphics card */
  glBufferData( GL_ARRAY_BUFFER, zfmModelList[k].numindex*sizeof(vect2f), zfmModelList[k].texcoords, GL_STATIC_DRAW);


  if(zfmModelList[k].normals)free(zfmModelList[k].normals);
  zfmModelList[k].normals=NULL;

  
  return &zfmModelList[k];
}

zfmModel *zfmLoadModelWithArmature(char *filename,char *armfilename)
{
  FILE *file;
  char modelname[80];
  int numfaces;
  int numvertices;
  int numnormals;
  int numtexcoords;
  vect3f *vertices = NULL;
  vect3f *normals = NULL;
  vect2f *texcoords = NULL;
  face3i *faces = NULL;
  int i;
  int k;
  int index;
  
  /*first search to see if the requested model is alreday loaded*/
  for(k = 0; k < MAXZFMMODELS; k++)
  {
    if(strcmp(filename,zfmModelList[k].filename)==0)
    {
      zfmModelList[k].used++;
      return &zfmModelList[k];
    }
  }
  /*makesure we have the room for a new model*/
  if(numZfmModels + 1 >= MAXZFMMODELS)
  {
    fprintf(stderr, "Maximum Models Reached.\n");
    exit(1);
  }
  /*if its not already in memory, then load it.*/
  numZfmModels++;
  for(k = 0;k <= MAXZFMMODELS;k++)
  {
    if(!zfmModelList[k].used)break;
  }
  zfmModelList[k].used++;
  strcpy(zfmModelList[k].filename,filename);
  file = fopen(filename,"rb");
  if(file==NULL)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
  }
  if(fread(&modelname,80*sizeof(char),1,file) != 1)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
  }
  if(fread(&numfaces,sizeof(int),1,file) != 1)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
  }
  if(fread(&numvertices,sizeof(int),1,file) != 1)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
  }
  if(fread(&numnormals,sizeof(int),1,file) != 1)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
  }
  if(fread(&numtexcoords,sizeof(int),1,file) != 1)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
  }
  if(numvertices > 0)
  {
    vertices = (vect3f*)malloc(sizeof(vect3f)*numvertices);
    if(fread(vertices,numvertices*sizeof(vect3f),1,file) != 1)
    {
      fprintf(stderr, "failed to load vertices %s.\n",filename);
    }
  }
  if(numnormals > 0)
  {
    normals = (vect3f*)malloc(sizeof(vect3f)*numnormals);
    if(fread(normals,numnormals*sizeof(vect3f),1,file) != 1)
    {
      fprintf(stderr, "failed to load normals: %s.\n",filename);
    }
  }
  if(numtexcoords > 0)
  {
    texcoords = (vect2f*)malloc(sizeof(vect2f)*numtexcoords);
    if(fread(texcoords,numtexcoords*sizeof(vect2f),1,file) != 1)
    {
      fprintf(stderr, "failed to load tex coords: %s.\n",filename);
    }
  }
  if(numfaces > 0)
  {
    faces = (face3i*)malloc(sizeof(face3i)*numfaces);
    if(fread(faces,numfaces*sizeof(face3i),1,file) != 1)
    {
      fprintf(stderr, "failed to load faces: %s.\n",filename);
    }
  }
  fclose(file);
  zfmModelList[k].numvertices=numvertices;
  zfmModelList[k].numfaces=numfaces;
  zfmModelList[k].faces = (face3i*)malloc(sizeof(face3i)*(zfmModelList[k].numfaces));
  memcpy( zfmModelList[k].faces, faces, sizeof(face3i) * numfaces);
  zfmModelList[k].verticesAtRest = (vect3f*)malloc(sizeof(vect3f) * numvertices);
  zfmModelList[k].verticesDeformed = (vect3f*)malloc(sizeof(vect3f) * numvertices);
  memcpy( zfmModelList[k].verticesAtRest, vertices, sizeof(vect3f) * numvertices);
  zfmModelList[k].vertices = (vect3f*)malloc(sizeof(vect3f) * ((numfaces-1)*3));
  zfmModelList[k].normals = (vect3f*)malloc(sizeof(vect3f) * ((numfaces-1)*3));
  zfmModelList[k].texcoords = (vect2f*)malloc(sizeof(vect2f) * ((numfaces-1)*3));
  zfmModelList[k].indexarray = (int*)malloc(sizeof(int) * ((numfaces-1)*3));
  
  index=0;
  for(i = 1; i < (numfaces);i++)
  {
    zfmModelList[k].indexarray[index] = index;
    zfmModelList[k].vertices[index] = vertices[faces[i].vert1[0]];
    zfmModelList[k].normals[index] = normals[faces[i].vert1[2]];
    zfmModelList[k].texcoords[index] = texcoords[faces[i].vert1[1]];
    index++;
    zfmModelList[k].indexarray[index] = index;
    zfmModelList[k].vertices[index] = vertices[faces[i].vert2[0]];
    zfmModelList[k].normals[index] = normals[faces[i].vert2[2]];
    zfmModelList[k].texcoords[index] = texcoords[faces[i].vert2[1]];
    index++;
    zfmModelList[k].indexarray[index] = index;
    zfmModelList[k].vertices[index] = vertices[faces[i].vert3[0]];
    zfmModelList[k].normals[index] = normals[faces[i].vert3[2]];
    zfmModelList[k].texcoords[index] = texcoords[faces[i].vert3[1]];
    index++;
  }

  if(vertices)free(vertices);
  vertices=NULL;
  if(normals)free(normals);
  normals=NULL;
  if(texcoords)free(texcoords);
  texcoords=NULL;
  if(faces)free(faces);
  faces=NULL;
  
  zfmModelList[k].numindex = index;
      
  glGenBuffers( 1, &zfmModelList[k].vertexArray);			
  /*bind it to GL_ARRAY_BUFFER */
  glBindBuffer( GL_ARRAY_BUFFER, zfmModelList[k].vertexArray);		
  /*apply the array data to the buffer sending it to the graphics card */
  glBufferData( GL_ARRAY_BUFFER, zfmModelList[k].numindex*sizeof(vect3f), zfmModelList[k].vertices, GL_DYNAMIC_DRAW);
  
  /*first generate a buffer index*/
  glGenBuffers( 1, &zfmModelList[k].normalArray);			
  /*bind it to GL_ARRAY_BUFFER */
  glBindBuffer( GL_ARRAY_BUFFER, zfmModelList[k].normalArray);
  /*apply the array data to the buffer sending it to the graphics card */
  glBufferData( GL_ARRAY_BUFFER, zfmModelList[k].numindex*sizeof(vect3f), zfmModelList[k].normals, GL_DYNAMIC_DRAW);
  zfmModelList[k].numnormals = numnormals;

  /*first generate a buffer index*/
  glGenBuffers( 1, &zfmModelList[k].uvArray);			
  /*bind it to GL_ARRAY_BUFFER */
  glBindBuffer( GL_ARRAY_BUFFER, zfmModelList[k].uvArray);		
  /*apply the array data to the buffer sending it to the graphics card */
  glBufferData( GL_ARRAY_BUFFER, zfmModelList[k].numindex*sizeof(vect2f), zfmModelList[k].texcoords, GL_DYNAMIC_DRAW);
  zfmModelList[k].numtexcoords = numtexcoords;

  zfmModelList[k].skeleton = LoadArmatureFile(armfilename);
  if(zfmModelList[k].skeleton)zfmSetupArmatureSpace(&zfmModelList[k], zfmModelList[k].skeleton);
  getDimen(&zfmModelList[k]);
  return &zfmModelList[k];
}


void InitZfmModelList()
{
  numZfmModels = 0;
  memset(zfmModelList,0,sizeof(zfmModel) * MAXZFMMODELS);

}

void CloseZfmModels()
{
  int i;
  for(i = 0;i < MAXZFMMODELS;i++)
  {
    if(zfmModelList[i].used > 0)
    {
      if (zfmModelList[i].vertices)	free(zfmModelList[i].vertices);
      zfmModelList[i].vertices=NULL;
      if (zfmModelList[i].normals)	free(zfmModelList[i].normals);
      zfmModelList[i].normals=NULL;
      if (zfmModelList[i].texcoords)	free(zfmModelList[i].texcoords);
      zfmModelList[i].texcoords=NULL;
      if (zfmModelList[i].indexarray)     free(zfmModelList[i].indexarray);
      zfmModelList[i].indexarray=NULL;

      if(glIsBuffer(zfmModelList[i].vertexArray) == GL_TRUE)glDeleteBuffers(1, &zfmModelList[i].vertexArray);
      if(glIsBuffer(zfmModelList[i].normalArray) == GL_TRUE)glDeleteBuffers(1, &zfmModelList[i].normalArray);
      if(glIsBuffer(zfmModelList[i].uvArray) == GL_TRUE)glDeleteBuffers(1, &zfmModelList[i].uvArray);
    }
  }
}

void freeZfmModel(zfmModel* model)
{
  model->used--;
  if(model->used <= 0)
  {
    if (model->vertices)	free(model->vertices);
    model->vertices=NULL;
    if (model->verticesAtRest)	free(model->verticesAtRest);
    model->verticesAtRest=NULL;
    if (model->verticesDeformed)	free(model->verticesDeformed);
    model->verticesDeformed=NULL;
    if (model->faces)	free(model->faces);
    model->faces=NULL;
    if (model->normals)	free(model->normals);
    model->normals=NULL;
    if (model->texcoords)	free(model->texcoords);
    model->texcoords=NULL;
    if (model->indexarray)free(model->indexarray);
    model->indexarray=NULL;

    if(glIsBuffer(model->vertexArray) == GL_TRUE)glDeleteBuffers(1, &model->vertexArray);
    if(glIsBuffer(model->normalArray) == GL_TRUE)glDeleteBuffers(1, &model->normalArray);
    if(glIsBuffer(model->uvArray) == GL_TRUE)glDeleteBuffers(1, &model->uvArray);

    
    numZfmModels--;
  }
}

void zfmShiftUV(zfmModel *model,float offx,float offy)
{
  int i;
  if(model == NULL)
  {
    fprintf(stderr,"trying to shift the UV of NO model\n");
    return;
  }
  if(model->numtexcoords == 0)fprintf(stderr,"model has no texture coordinates!\n");
  fprintf(stderr,"num texture coords: %i\n",model->numtexcoords);
  for(i = 0;i < model->numtexcoords;i++)
  {
    model->texcoords[i].vert[0] += offx;
    model->texcoords[i].vert[1] += offy;
  }

  if(glIsBuffer(model->uvArray) == GL_TRUE)glDeleteBuffers(1, &model->uvArray);
    /*first generate a buffer index*/
  glGenBuffers( 1, &model->uvArray);			
  /*bind it to GL_ARRAY_BUFFER */
  glBindBuffer( GL_ARRAY_BUFFER, model->uvArray);		
  /*apply the array data to the buffer sending it to the graphics card */
  glBufferData( GL_ARRAY_BUFFER, model->numindex*sizeof(vect2f), model->texcoords, GL_DYNAMIC_DRAW);

/*  glBindBuffer(GL_ARRAY_BUFFER, model->uvArray);
  vertsInVideoMemory = glMapBuffer(GL_ARRAY_BUFFER,GL_WRITE_ONLY);
  for(j = 0; j < model->numtexcoords; j++)
  {
    *(vect2f*)vertsInVideoMemory= model->texcoords[j];
    vertsInVideoMemory+=sizeof(vect2f);
  }*/
  glUnmapBuffer(GL_ARRAY_BUFFER);
}

void zfmDrawModelNoBuff(zfmModel *model, Armature *arm, Sprite *colortexture, GLuint shaderProgram,float x,float y,float z, float rotx,float roty,float rotz,float scalex,float scaley,float scalez, int drawMode, int frame, float alpha)
{
  int f;
  face3i *face;
  vect3f *vertices;
  if(model == NULL)return;
  if(scalex !=  1.0f && scaley != 1.0f && scalez != 1.0f)glEnable(GL_NORMALIZE);
  
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glPushMatrix();
  glPushAttrib(GL_LIGHTING_BIT);
  glColor4f(1,1,1,alpha);

  glTranslatef(x,y,z);
  glRotatef(rotz, 0.0f, 0.0f, 1.0f);
  glRotatef(rotx, 1.0f, 0.0f, 0.0f);
  glRotatef(roty, 0.0f, 1.0f, 0.0f);
  glScalef(scalex,scaley,scalez);

  if(colortexture != NULL)
  {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, colortexture->image);
  }
  

  glUseProgram(shaderProgram);
  

  glUniform1i(glGetUniformLocation(shaderProgram, "colorMap"), 0);

  /*draw everything, the index array identifies the indinces of the arrays*/
  
  if(arm == NULL)
  {
    /*draw without using armature*/
    vertices = model->verticesAtRest;
  }
  else
  {
    zfmDeformMeshNoBuff(model,arm,frame);
  }
  glBegin(drawMode);
  for(f = 0; f < model->numfaces;f++)
  {
    face = &model->faces[f];
    glNormal3f(model->normals[face->vert1[0]].vert[0],model->normals[face->vert1[0]].vert[1],model->normals[face->vert1[0]].vert[2]);
    glTexCoord2f(model->normals[face->vert1[1]].vert[0],model->normals[face->vert1[1]].vert[1]);
    glVertex3f(vertices[face->vert1[2]].vert[0],vertices[face->vert1[2]].vert[1],vertices[face->vert1[2]].vert[2]);

    glNormal3f(model->normals[face->vert2[0]].vert[0],model->normals[face->vert2[0]].vert[1],model->normals[face->vert2[0]].vert[2]);
    glTexCoord2f(model->normals[face->vert2[1]].vert[0],model->normals[face->vert2[1]].vert[1]);
    glVertex3f(vertices[face->vert2[2]].vert[0],vertices[face->vert2[2]].vert[1],vertices[face->vert2[2]].vert[2]);

    glNormal3f(model->normals[face->vert3[0]].vert[0],model->normals[face->vert3[0]].vert[1],model->normals[face->vert3[0]].vert[2]);
    glTexCoord2f(model->normals[face->vert3[1]].vert[0],model->normals[face->vert3[1]].vert[1]);
    glVertex3f(vertices[face->vert3[2]].vert[0],vertices[face->vert3[2]].vert[1],vertices[face->vert3[2]].vert[2]);
  }
  glEnd();
  glUseProgram(0);
  /*disable the arrays*/
  glDisableClientState( GL_VERTEX_ARRAY );				
  glDisableClientState( GL_NORMAL_ARRAY );
  glDisableClientState( GL_TEXTURE_COORD_ARRAY );

  glPopAttrib();
  glColor4f(1,1,1,1);
  glPopMatrix();
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_NORMALIZE);
  glDisable(GL_BLEND);
}


void zfmDrawModel(zfmModel *model, Armature *arm, Sprite *colortexture, GLuint shaderProgram,float x,float y,float z, float rotx,float roty,float rotz,float scalex,float scaley,float scalez, int drawMode, int frame, float alpha)
{
  float trans[]={1.0f,1.0f,1.0f,1.0f};
  
  trans[3] = alpha;
  if(model == NULL)return;
  if(arm != NULL)zfmDeformMesh(model,arm,frame);
  if(scalex !=  1.0f && scaley != 1.0f && scalez != 1.0f)glEnable(GL_NORMALIZE);
  glColor4f(1,1,1,alpha);
  
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glPushMatrix();
  glPushAttrib(GL_LIGHTING_BIT);
  glMaterialfv(GL_FRONT,GL_DIFFUSE,trans);
  glTranslatef(x,y,z);
  glRotatef(rotx, 1.0f, 0.0f, 0.0f);
  glRotatef(roty, 0.0f, 1.0f, 0.0f);
  glRotatef(rotz, 0.0f, 0.0f, 1.0f);
  glScalef(scalex,scaley,scalez);

  if(colortexture != NULL)
  {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, colortexture->image);
  }
  

  /*Bind the generated vertex buffer*/
  glBindBuffer(GL_ARRAY_BUFFER, model->vertexArray);
  /*Identify the buffer as a vertex array*/
  glVertexPointer(3, GL_FLOAT, 0, 0);
  glEnableClientState(GL_VERTEX_ARRAY);
  
  /*Bind the generated normal buffer*/
  glBindBuffer(GL_ARRAY_BUFFER,  model->normalArray);
  /*Identify the buffer as a normal array*/
  glNormalPointer(GL_FLOAT, 0, 0);
  glEnableClientState(GL_NORMAL_ARRAY);
  
  /*Bind the generated uv buffer*/
  glBindBuffer(GL_ARRAY_BUFFER,  model->uvArray);
  /*Identify the buffer as a texture array*/
  glTexCoordPointer(2, GL_FLOAT, 0, 0);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);


  glUseProgram(shaderProgram);
  

  glUniform1i(glGetUniformLocation(shaderProgram, "colorMap"), 0);

  /*draw everything, the index array identifies the indinces of the arrays*/
  glDrawElements(drawMode, model->numindex, GL_UNSIGNED_INT, model->indexarray);
  
  glUseProgram(0);
  /*disable the arrays*/
  glDisableClientState( GL_VERTEX_ARRAY );				
  glDisableClientState( GL_NORMAL_ARRAY );
  glDisableClientState( GL_TEXTURE_COORD_ARRAY );
  
  glPopAttrib();
  glPopMatrix();
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_NORMALIZE);
  glDisable(GL_BLEND);
  glColor4f(1,1,1,1);
}

void zfmSetupArmatureSpace(zfmModel *model, Armature *arm)
{
  int j;
  float rot90matrix[4][4];
  GLubyte *vertsInVideoMemory;
  
  rot90matrix[0][0]=1.0f;
  rot90matrix[1][0]=0.0f;
  rot90matrix[2][0]=0.0f;
  rot90matrix[3][0]=0.0f;
  
  rot90matrix[0][1]=0.0f;
  rot90matrix[1][1]=0.0f;
  rot90matrix[2][1]=1.0f;
  rot90matrix[3][1]=0.0f;
  
  rot90matrix[0][2]=0.0f;
  rot90matrix[1][2]=-1.0f;
  rot90matrix[2][2]=0.0f;
  rot90matrix[3][2]=0.0f;
  
  rot90matrix[0][3]=0.0f;
  rot90matrix[1][3]=0.0f;
  rot90matrix[2][3]=0.0f;
  rot90matrix[3][3]=0.0f;
  
  if(model!=NULL && arm!=NULL)
  {
    for(j = 0; j < model->numvertices; j++)
    {
      Mat4MulVecfl(arm->obmatrix, model->verticesAtRest[j].vert);
      Mat4MulVecfl(rot90matrix, model->verticesAtRest[j].vert);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, model->vertexArray);
    vertsInVideoMemory = glMapBuffer(GL_ARRAY_BUFFER,GL_WRITE_ONLY);
    for(j = 1; j < model->numfaces; j++)
    {
      *(vect3f*)vertsInVideoMemory= model->verticesAtRest[model->faces[j].vert1[0]];
      vertsInVideoMemory+=sizeof(vect3f);
      *(vect3f*)vertsInVideoMemory= model->verticesAtRest[model->faces[j].vert2[0]];
      vertsInVideoMemory+=sizeof(vect3f);
      *(vect3f*)vertsInVideoMemory= model->verticesAtRest[model->faces[j].vert3[0]];
      vertsInVideoMemory+=sizeof(vect3f);
    }
    model->currentFrameInMemory = 0;
    glUnmapBuffer(GL_ARRAY_BUFFER);
  }
}

void zfmDeformMeshNoBuff(zfmModel *model, Armature *arm, int frame)
{
  Bone *tempBone;
  Pose *tempframe; 
  vect3f tempVert;
  int i,j;
 
  if(model==NULL || arm==NULL)
  {
    return;
  } 
  if((frame >= arm->numframes) || (frame == model->currentFrameInMemory))return;
  tempframe=&arm->frame[frame];
  for(j = 0; j < arm->numbones; j++)
  {
    tempBone=&arm->bones[j];
    for(i = 0; i < tempBone->numVerts; i++)
    {
      tempVert=model->verticesAtRest[tempBone->vindices[i]];

      Mat4MulVecfl(tempframe->bonepose[j].matrix, tempVert.vert);

      model->verticesDeformed[tempBone->vindices[i]] = tempVert;

    }
  }

  glBindBuffer(GL_ARRAY_BUFFER, model->vertexArray);
  model->currentFrameInMemory = frame;
}


void zfmDeformMesh(zfmModel *model, Armature *arm, int frame)
{
  Bone *tempBone;
  Pose *tempframe; 
  vect3f tempVert;
  int i,j;
  GLubyte *vertsInVideoMemory;
  
  if(model==NULL || arm==NULL)
  {
    return;
  } 
  if(frame >= arm->numframes || frame == model->currentFrameInMemory)return;
  tempframe=&arm->frame[frame];
  for(j = 0; j < arm->numbones; j++)
  {
    tempBone=&arm->bones[j];
    for(i = 0; i < tempBone->numVerts; i++)
    {
      tempVert=model->verticesAtRest[tempBone->vindices[i]];

      Mat4MulVecfl(tempframe->bonepose[j].matrix, tempVert.vert);

      model->verticesDeformed[tempBone->vindices[i]] = tempVert;

    }
  }

  glBindBuffer(GL_ARRAY_BUFFER, model->vertexArray);
  vertsInVideoMemory = glMapBuffer(GL_ARRAY_BUFFER,GL_WRITE_ONLY);
  for(j = 1; j < model->numfaces; j++)
  {
    *(vect3f*)vertsInVideoMemory= model->verticesDeformed[model->faces[j].vert1[0]];
    vertsInVideoMemory+=sizeof(vect3f);
    *(vect3f*)vertsInVideoMemory = model->verticesDeformed[model->faces[j].vert2[0]];
    vertsInVideoMemory+=sizeof(vect3f);
    *(vect3f*)vertsInVideoMemory = model->verticesDeformed[model->faces[j].vert3[0]];
    vertsInVideoMemory+=sizeof(vect3f);
  }
  glUnmapBuffer(GL_ARRAY_BUFFER);
  model->currentFrameInMemory = frame;
}


void getDimen(zfmModel *model)
{
  GLuint  j;
  GLfloat maxx, minx, maxy, miny, maxz, minz;

  
  maxx = minx = model->verticesAtRest[1].vert[0];
  maxy = miny = model->verticesAtRest[1].vert[1];
  maxz = minz = model->verticesAtRest[1].vert[2];
  for(j = 1; j < model->numvertices; j++)
  {
    if (maxx < model->verticesAtRest[j].vert[0])
      maxx = model->verticesAtRest[j].vert[0];
    if (minx > model->verticesAtRest[j].vert[0])
      minx = model->verticesAtRest[j].vert[0];
    
    if (maxy < model->verticesAtRest[j].vert[1])
      maxy = model->verticesAtRest[j].vert[1];
    if (miny > model->verticesAtRest[j].vert[1])
      miny = model->verticesAtRest[j].vert[1];
    
    if (maxz < model->verticesAtRest[j].vert[2])
      maxz = model->verticesAtRest[j].vert[2];
    if (minz > model->verticesAtRest[j].vert[2])
      minz = model->verticesAtRest[j].vert[2];

  }
  model->dimen[0] = maxx - minx;
  model->dimen[1] = maxy - miny;
  model->dimen[2] = maxz - minz;
    
  model->center[0] = (maxx + minx) / 2.0;
  model->center[1] = (maxy + miny) / 2.0;
  model->center[2] = (maxz + minz) / 2.0;


}

void zfmScaleModel(zfmModel *model, float scale)
{
  int i,j;
  GLubyte *vertsInVideoMemory;
  
  for (i = 1; i < model->numvertices; i++) 
  {
    model->verticesAtRest[i].vert[0] *= scale;
    model->verticesAtRest[i].vert[1] *= scale;
    model->verticesAtRest[i].vert[2] *= scale;
  }
  glBindBuffer(GL_ARRAY_BUFFER, model->vertexArray);
  vertsInVideoMemory = glMapBuffer(GL_ARRAY_BUFFER,GL_WRITE_ONLY);
  for(j = 1; j < model->numfaces; j++)
  {
    *(vect3f*)vertsInVideoMemory= model->verticesAtRest[model->faces[j].vert1[0]];
    vertsInVideoMemory+=sizeof(vect3f);
    *(vect3f*)vertsInVideoMemory = model->verticesAtRest[model->faces[j].vert2[0]];
    vertsInVideoMemory+=sizeof(vect3f);
    *(vect3f*)vertsInVideoMemory = model->verticesAtRest[model->faces[j].vert3[0]];
    vertsInVideoMemory+=sizeof(vect3f);
  }
  glUnmapBuffer(GL_ARRAY_BUFFER);
  getDimen(model);
}

zfmModel *zfmLoadModelScaled(char *filename, float scale)
{
  FILE *file;
  char modelname[80];
  int numfaces;
  int numvertices;
  int numnormals;
  int numtexcoords;
  vect3f *vertices = NULL;
  vect3f *normals = NULL;
  vect2f *texcoords = NULL;
  face3i *faces = NULL;
  int i;
  int k;
  int index;
  
  /*first search to see if the requested model is alreday loaded*/
  for(k = 0; k < MAXZFMMODELS; k++)
  {
    if(strcmp(filename,zfmModelList[k].filename)==0)
    {
      zfmModelList[k].used++;
      return &zfmModelList[k];
    }
  }
  /*makesure we have the room for a new model*/
  if(numZfmModels + 1 >= MAXZFMMODELS)
  {
    fprintf(stderr, "Maximum Models Reached.\n");
    exit(1);
  }
  /*if its not already in memory, then load it.*/
  numZfmModels++;
  for(k = 0;k <= MAXZFMMODELS;k++)
  {
    if(!zfmModelList[k].used)break;
  }
  zfmModelList[k].used++;
  strcpy(zfmModelList[k].filename,filename);
  file = fopen(filename,"rb");
  if(file==NULL)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
  }
  if(fread(&modelname,80*sizeof(char),1,file) != 1)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
  }
  if(fread(&numfaces,sizeof(int),1,file) != 1)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
  }
  if(fread(&numvertices,sizeof(int),1,file) != 1)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
  }
  if(fread(&numnormals,sizeof(int),1,file) != 1)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
  }
  if(fread(&numtexcoords,sizeof(int),1,file) != 1)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
  }
  if(numvertices > 0)
  {
    vertices = (vect3f*)malloc(sizeof(vect3f)*numvertices);
    if(fread(vertices,numvertices*sizeof(vect3f),1,file) != 1)
    {
      fprintf(stderr, "failed to load vertices %s.\n",filename);
    }
  }
  if(numnormals > 0)
  {
    normals = (vect3f*)malloc(sizeof(vect3f)*numnormals);
    if(fread(normals,numnormals*sizeof(vect3f),1,file) != 1)
    {
      fprintf(stderr, "failed to load normals: %s.\n",filename);
    }
  }
  if(numtexcoords > 0)
  {
    texcoords = (vect2f*)malloc(sizeof(vect2f)*numtexcoords);
    if(fread(texcoords,numtexcoords*sizeof(vect2f),1,file) != 1)
    {
      fprintf(stderr, "failed to load tex coords: %s.\n",filename);
    }
  }
  if(numfaces > 0)
  {
    faces = (face3i*)malloc(sizeof(face3i)*numfaces);
    if(fread(faces,numfaces*sizeof(face3i),1,file) != 1)
    {
      fprintf(stderr, "failed to load faces: %s.\n",filename);
    }
  }
  fclose(file);
  zfmModelList[k].numvertices=numvertices;
  zfmModelList[k].numfaces=numfaces;
  zfmModelList[k].faces = (face3i*)malloc(sizeof(face3i)*(zfmModelList[k].numfaces));
  memcpy( zfmModelList[k].faces, faces, sizeof(face3i) * numfaces);
  zfmModelList[k].verticesAtRest = (vect3f*)malloc(sizeof(vect3f) * numvertices);
  zfmModelList[k].verticesDeformed = (vect3f*)malloc(sizeof(vect3f) * numvertices);
  memcpy( zfmModelList[k].verticesAtRest, vertices, sizeof(vect3f) * numvertices);
  zfmModelList[k].vertices = (vect3f*)malloc(sizeof(vect3f) * ((numfaces-1)*3));
  zfmModelList[k].normals = (vect3f*)malloc(sizeof(vect3f) * ((numfaces-1)*3));
  zfmModelList[k].texcoords = (vect2f*)malloc(sizeof(vect2f) * ((numfaces-1)*3));
  zfmModelList[k].indexarray = (int*)malloc(sizeof(int) * ((numfaces-1)*3));
  
  index=0;
  for(i = 1; i < (numfaces);i++)
  {
    zfmModelList[k].indexarray[index] = index;
    zfmModelList[k].vertices[index] = vertices[faces[i].vert1[0]];
    zfmModelList[k].normals[index] = normals[faces[i].vert1[2]];
    zfmModelList[k].texcoords[index] = texcoords[faces[i].vert1[1]];
    index++;
    zfmModelList[k].indexarray[index] = index;
    zfmModelList[k].vertices[index] = vertices[faces[i].vert2[0]];
    zfmModelList[k].normals[index] = normals[faces[i].vert2[2]];
    zfmModelList[k].texcoords[index] = texcoords[faces[i].vert2[1]];
    index++;
    zfmModelList[k].indexarray[index] = index;
    zfmModelList[k].vertices[index] = vertices[faces[i].vert3[0]];
    zfmModelList[k].normals[index] = normals[faces[i].vert3[2]];
    zfmModelList[k].texcoords[index] = texcoords[faces[i].vert3[1]];
    index++;
  }

  getDimen(&zfmModelList[k]);
  if(vertices)free(vertices);
  vertices=NULL;
  if(normals)free(normals);
  normals=NULL;
  if(texcoords)free(texcoords);
  texcoords=NULL;
  if(faces)free(faces);
  faces=NULL;

  zfmModelList[k].numindex = index;
      
  glGenBuffers( 1, &zfmModelList[k].vertexArray);			
  /*bind it to GL_ARRAY_BUFFER */
  glBindBuffer( GL_ARRAY_BUFFER, zfmModelList[k].vertexArray);		
  /*apply the array data to the buffer sending it to the graphics card */
  glBufferData( GL_ARRAY_BUFFER, zfmModelList[k].numindex*sizeof(vect3f), zfmModelList[k].vertices, GL_STATIC_DRAW);
  
  /*first generate a buffer index*/
  glGenBuffers( 1, &zfmModelList[k].normalArray);			
  /*bind it to GL_ARRAY_BUFFER */
  glBindBuffer( GL_ARRAY_BUFFER, zfmModelList[k].normalArray);
  /*apply the array data to the buffer sending it to the graphics card */
  glBufferData( GL_ARRAY_BUFFER, zfmModelList[k].numindex*sizeof(vect3f), zfmModelList[k].normals, GL_STATIC_DRAW);
  
  /*first generate a buffer index*/
  glGenBuffers( 1, &zfmModelList[k].uvArray);			
  /*bind it to GL_ARRAY_BUFFER */
  glBindBuffer( GL_ARRAY_BUFFER, zfmModelList[k].uvArray);		
  /*apply the array data to the buffer sending it to the graphics card */
  glBufferData( GL_ARRAY_BUFFER, zfmModelList[k].numindex*sizeof(vect2f), zfmModelList[k].texcoords, GL_STATIC_DRAW);


  if(zfmModelList[k].normals)free(zfmModelList[k].normals);
  zfmModelList[k].normals=NULL;
  if(zfmModelList[k].texcoords)free(zfmModelList[k].texcoords);
  zfmModelList[k].texcoords=NULL;

  zfmScaleModel(&zfmModelList[k], scale);
  return &zfmModelList[k];
}


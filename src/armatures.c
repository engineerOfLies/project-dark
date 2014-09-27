#include "armatures.h"

#define ABS(x)	((x) < 0 ? -(x) : (x))
#define SWAP(type, a, b)	{ type sw_ap; sw_ap=(a); (a)=(b); (b)=sw_ap; }

Armature ArmList[MAXARMS];    /*global list of armatures*/
int NumArms;                  /*how many armatures we have*/


/*clears and zeroes the list of armatures, do not call if any armatures have been created*/
void InitArmatures()
{
  int i;
  for(i = 0;i < MAXARMS;i++)
    if(ArmList[i].RefCount)return;/*we don't make memory leaks with this function*/
  memset(ArmList,0,sizeof(Armature) * MAXARMS);
  

}

/*clears all poses, bones and aramtures*/
void CloseArmatures()
{
  int i,b,p;
  for(i = 0;i < MAXARMS;i++)
  {
    if(ArmList[i].RefCount)
    { /*delete all of our bones.*/
      for(b = 0; b < ArmList[i].numbones;b++)
      {
        if(ArmList[i].bones != NULL)
        {
          free(ArmList[i].bones);
          ArmList[i].bones = NULL;
        }
      }
      for(p = 0; p < ArmList[i].numframes; p++)
      {
        for(b = 0;b < ArmList[i].frame[p].numbones;b++)
        {
          if(ArmList[i].frame[p].bonepose != NULL)
          {
            free(ArmList[i].frame[p].bonepose);
            ArmList[i].frame[p].bonepose = NULL;
          }
        }
      }
      free(ArmList[i].frame);/*once all of our sub pointers have been cleaned up, free the frames.*/
      ArmList[i].frame = NULL;
    }
  }
  /*once the list has had all of the pointers freed and cleaned up, we can zero it for future use.*/
  memset(ArmList,0,sizeof(Armature) * MAXARMS);
}

 /*we're done with a single armature, please remove refcount*/
void FreeArmature(Armature *arm)
{
  int b,p;
  if(arm == NULL)return;
  arm->RefCount--;
  if(arm->RefCount <= 0)
  { /*RefCount should never be less than zero, but here we are hedging out bets*/
    arm->RefCount = 0;
    for(b = 0; b < arm->numbones;b++)
    {
      if(arm->bones != NULL)
      {
        free(arm->bones);
        arm->bones = NULL;
      }
    }
    for(p = 0; p < arm->numframes; p++)
    {
      if(arm->frame != NULL)
      { /*delete all of our bone poses*/
        for(b = 0;b < arm->frame[p].numbones;b++)
        {
          if(arm->frame[p].bonepose != NULL)
          {
            free(arm->frame[p].bonepose);
            arm->frame[p].bonepose = NULL;
          }
        }
      }
      free(arm->frame);/*once all of our sub pointers have been cleaned up, free the frame.*/
      arm->frame = NULL;
    }
  }
}


/*New Armature flags an armature from the list, and if given the number of bones will allocate the bones.  If it is also given frames (and bones) it will allocate those as well.*/
Armature *NewArmature(char name[80], int numbones,int numframes)
{
  int i,j,k;
  if(NumArms + 1 >= MAXARMS)
  {
    return NULL;
    fprintf(stderr,"We ran out of space for Armatures.\n");
  }
  for(i = 0;i < MAXARMS;i++)
  {
    if(ArmList[i].RefCount == 0)
    {
      ArmList[i].RefCount = 1;
      strncpy(ArmList[i].name,name,80);
      if(numbones)
      {
        ArmList[i].numbones = numbones;
        ArmList[i].bones = (Bone*)malloc(sizeof(Bone)*numbones);
        if(ArmList[i].bones == NULL)
        {
          fprintf(stderr,"Big Problem:  Ran out of space for bones.\n");
          ArmList[i].RefCount = 0;
          return NULL;
        }
        memset(ArmList[i].bones,0,sizeof(Bone)*numbones);
        if(numframes)
        {
          ArmList[i].numframes = numframes;
          ArmList[i].frame = (Pose*)malloc(sizeof(Pose)*numframes);
          if(ArmList[i].frame == NULL)
          {
            fprintf(stderr,"Big Problem:  Ran out of space for frames.\n");
            ArmList[i].RefCount = 0;
            free(ArmList[i].bones);
            ArmList[i].bones = NULL;/*gotta clean up anything we have yet done.*/
            return NULL;
          }
          memset(ArmList[i].frame,0,sizeof(Pose)*numframes);
          for(j = 0;j < numframes;j++)
          {
            ArmList[i].frame[j].numbones = numbones;
            ArmList[i].frame[j].bonepose = (BonePose*)malloc(sizeof(BonePose)*numbones);
            if(ArmList[i].frame[j].bonepose == NULL)
            {
              fprintf(stderr,"Big Problem:  Ran out of space for poses.\n");
              ArmList[i].RefCount = 0;
              free(ArmList[i].bones);
              ArmList[i].bones = NULL;/*gotta clean up anything we have yet done.*/
              for(k = 0;k < j;k++)
              {
                free(ArmList[i].frame[k].bonepose);
                /*free every pose we made so far*/
              }
              free(ArmList[i].frame);
              ArmList[i].frame = NULL;
              return NULL;
            }
          }
        }
      }
      /*by here, we have everything allocated that we need.*/
      return &ArmList[i];
    }
  }
  fprintf(stderr,"Big Problem: We don't have a free slot for Armatures that we should have.\n");
  return NULL;
}

/*Allocates the bones for an armature, if bones already exist, deletes them and allocates new ones.*/
int AllocateBones(Armature *arm,int numbones)
{
  if(numbones)
  {
    if(arm->bones != NULL)
    {
      free(arm->bones);
      arm->bones = NULL;
    }
    arm->numbones = numbones;
    arm->bones = (Bone*)malloc(sizeof(Bone)*numbones);
    if(arm->bones == NULL)
    {
      fprintf(stderr,"Big Problem:  Ran out of space for bones.\n");
      arm->RefCount = 0;
      return 0;
    }
    memset(arm->bones,0,sizeof(Bone)*numbones);
    return 1;
  }
  return 0;
}
/*Allocates the frames for an armature IF bones already exist.  If frames already exist, it deletes them and allocates new ones.*/
int  AllocateFrames(Armature *arm,int numframes)
{
  int j,k;
  if((numframes) && (arm->numbones))
  {
    if(arm->frame != NULL)
    {
      for(j = 0;j < arm->numframes;j++)
      {
        if(arm->frame[j].bonepose != NULL)
        {
          free(arm->frame[j].bonepose);
          arm->frame[j].bonepose = NULL;
        }
      }
      free(arm->frame);
      arm->frame = NULL;
    }
    arm->numframes = numframes;
    arm->frame = (Pose*)malloc(sizeof(Pose)*numframes);
    if(arm->frame == NULL)
    {
      fprintf(stderr,"Big Problem:  Ran out of space for frames.\n");
      return 0;
    }
    memset(arm->frame,0,sizeof(Pose)*numframes);
    for(j = 0;j < numframes;j++)
    {
      arm->frame[j].numbones = arm->numbones;
      arm->frame[j].bonepose = (BonePose*)malloc(sizeof(BonePose)* arm->numbones);
      if(arm->frame[j].bonepose == NULL)
      {
        fprintf(stderr,"Big Problem:  Ran out of space for poses.\n");
        for(k = 0;k < j;k++)
        {
          free(arm->frame[k].bonepose);
          /*free every pose we made so far*/
        }
        free(arm->frame);
        arm->frame = NULL;
        return 0;
      }
      memset(arm->frame[j].bonepose,0,sizeof(BonePose)* arm->numbones);
    }
    return 1;
  }
  return 0;
}

Bone *GetEmptyBone(Armature *arm)
{
  int i;
  for(i = 0;i < arm->numbones;i++)
  {
    if(strlen(arm->bones[i].name) == 0)
      return &arm->bones[i];
  }
  return NULL;
  /*all filled up*/
}

int ArmatureFirstPass(char filename[80],char groupName[80])
{
    FILE *file;
    char buf[120];
    int VertNum=0;
    char *c;

    file = fopen(filename, "r");
    
    if(file == NULL)
    {
      fprintf(stderr, "LoadArmatureObj failed: can't open data file \"%s\".\n",filename);
      return 0;
    }
    while(fscanf(file, "%s", buf) != EOF)
    {
      if(strcmp(buf,"GroupName:") == 0)
      {
        fgetc(file);  /*clear the space before the word*/
        fgets(buf, 120, file);
        c = strchr(buf, '\n');
        if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
        if(strcmp(buf,groupName) ==0)
        {
          VertNum++;
        }
      }
    }
    fclose(file);
    return VertNum;
}

/*loads the armature dater from file & puts it into the armature structure,  returns a pointer to the armature or NULL on failure.*/
Armature *LoadArmatureObj(char filename[80])
{
  FILE *file;
  int mnum = 0;
  int lmnum=0;
  int armnum=0;
  char *c;
  int framenum = 0;
  int bonenum = 0;
  int childnum = 0;
  int tempNum = 0;
  int obmnum=0;
  Bone *bonetemp = NULL;
  Bone *bonetemp2 = NULL;
  char buf[120];
  char temp[80];
  int vNum=0;
  Armature *arm = NULL;
  int i;
  
  for(i = 0; i < MAXARMS; i++)
  {
    if(strncmp(filename,ArmList[i].filename,50)==0)
    {
      return &ArmList[i];
    }
  }
  
  file = fopen(filename, "r");
  if(file == NULL)
  {
    fprintf(stderr, "LoadArmatureObj failed: can't open data file \"%s\".\n",filename);
    return NULL;
  }

  while(fscanf(file, "%s", buf) != EOF)
  {
    if(strcmp(buf,"#") ==0)
    {
      fgets(buf, sizeof(buf), file);
      continue;/*ignore the rest of the line.*/
    }
    
    /*armature section*/
    if(strcmp(buf,"armature") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(temp, 80, file);
      c = strchr(temp, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      arm = NewArmature(temp, 0,0);
      strncpy(arm->filename,filename,50);
      if(arm == NULL)
      {
        fclose(file);
        return NULL;
      }
      continue;
    }
    if(strcmp(buf,"armmat:") ==0)
    {
      if(arm == NULL)
      {
        fprintf(stderr,"Armature file is not valid.\n");
        return NULL;
      }
      fscanf(file, "%f %f %f %f", &arm->armmatrix[armnum][0], &arm->armmatrix[armnum][1], &arm->armmatrix[armnum][2], &arm->armmatrix[armnum][3]);
      armnum++;
      continue;
    }
    if(strcmp(buf,"bonecount") ==0)
    {
      if(arm == NULL)
      {
        fprintf(stderr,"Armature file is not valid.\n");
        return NULL;
      }
      fscanf(file, "%i", &arm->numbones);
      if(AllocateBones(arm,arm->numbones) == 0)
      { /*error message should already be printed if there was a problem.*/
        free(arm);
        return NULL;
      }
      continue;
    }
    if(strcmp(buf,"bone:") ==0)
    {
      if(arm == NULL)
      {
        fprintf(stderr,"Armature file is not valid.\n");
        return NULL;
      }
      fgetc(file);  /*clear the space before the word*/
      fgets(temp, 80, file);
      c = strchr(temp, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      bonetemp = GetEmptyBone(arm);
      strncpy(bonetemp->name,temp,80);
      bonetemp->numVerts=ArmatureFirstPass(filename,temp);
      bonetemp->curVert=0;
      bonetemp->vindices = (int*)malloc(sizeof(int)* bonetemp->numVerts);
      bonetemp->vWeights = (float*)malloc(sizeof(float)* bonetemp->numVerts);
      mnum = 0;
      continue;
    }
    if(strcmp(buf,"parent:") ==0)
    {
      if(bonetemp == NULL)
      {
        fprintf(stderr,"Armature file is not valid.\n");
        return NULL;
      }
      fgetc(file);  /*clear the space before the word*/
      fgets(temp, 80, file);
      c = strchr(temp, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      strncpy(bonetemp->pname,temp,80);
      bonetemp->hasparent=1;
      continue;
    }
    if(strcmp(buf,"childcount") ==0)
    {
      if(bonetemp == NULL)
      {
        fprintf(stderr,"Armature file is not valid.\n");
        return NULL;
      }
      fscanf(file, "%i", &bonetemp->childcount);
      childnum = 0;
      continue;
    }
    if(strcmp(buf,"child:") ==0)
    {
      if(bonetemp == NULL)
      {
        fprintf(stderr,"Armature file is not valid.\n");
        return NULL;
      }
      fgetc(file);  /*clear the space before the word*/
      fgets(temp, 80, file);
      c = strchr(temp, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      strncpy(bonetemp->cname[childnum++],temp,80);
      continue;
    }
    if(strcmp(buf,"m:") ==0)
    {
      if(bonetemp == NULL)
      {
        fprintf(stderr,"Armature file is not valid.\n");
        return NULL;
      }
      fscanf(file, "%f %f %f %f", &bonetemp->matrix[mnum][0], &bonetemp->matrix[mnum][1], &bonetemp->matrix[mnum][2], &bonetemp->matrix[mnum][3]);
      mnum++;
      continue;
    }
    if(strcmp(buf,"obm:") ==0)
    {
      fscanf(file, "%f %f %f %f", &arm->obmatrix[obmnum][0], &arm->obmatrix[obmnum][1], &arm->obmatrix[obmnum][2], &arm->obmatrix[obmnum][3]);
      obmnum++;
      continue;
    }
    if(strcmp(buf,"v") ==0)
    {
      vNum++;
      continue;
    }
    if(strcmp(buf,"influence:") ==0)
    {
      fscanf(file, "%d", &tempNum);
      continue;
    }

    if(strcmp(buf,"GroupName:") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(temp, 80, file);
      c = strchr(temp, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      bonetemp2=GetBoneByName(arm, temp);
      if(bonetemp2 == NULL)
      {
        fprintf(stderr,"failed to load bone: %s\n",temp);
        return NULL;
      }
      bonetemp2->vindices[bonetemp2->curVert]=vNum;
      continue;
    }
    if(strcmp(buf,"Weight:") ==0)
    {
      if(bonetemp2 == NULL)
      {
        fprintf(stderr,"Armature file is not valid.\n");
        return NULL;
      }
      fscanf(file, "%f", &bonetemp2->vWeights[bonetemp2->curVert]);
      bonetemp2->curVert++;
      continue;
    }


    /*pose section*/
    if(strcmp(buf,"framecount") ==0)
    {
      if(arm == NULL)
      {
        fprintf(stderr,"Armature file is not valid.\n");
        return NULL;
      }
      fscanf(file, "%i", &arm->numframes);
      if(AllocateFrames(arm,arm->numframes) == 0)
      { /*error message should already be printed if there was a problem.*/
        free(arm->bones);
        free(arm);
        return NULL;
      }
      framenum = 0;
      continue;
    }
    if(strcmp(buf,"frame:") ==0)
    {
      if(arm == NULL)
      {
        fprintf(stderr,"Armature file is not valid.\n");
        return NULL;
      }
      fscanf(file, "%i", &framenum);
      framenum--;
      bonenum = -1;
      continue;
    }
    if(strcmp(buf,"posebone:") ==0)
    {
      if((arm == NULL) ||(arm->frame == NULL))
      {
        fprintf(stderr,"Armature file is not valid.\n");
        return NULL;
      }
      fgetc(file);  /*clear the space before the word*/
      fgets(temp, 80, file);
      c = strchr(temp, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      bonenum++;
      lmnum=0;
      strncpy(arm->frame[framenum].bonepose[bonenum].name,temp,80);
      continue;
    }
 
    if(strcmp(buf,"lm:") ==0)
    {
      fscanf(file, "%f %f %f %f", &arm->frame[framenum].bonepose[bonenum].matrix[lmnum][0], &arm->frame[framenum].bonepose[bonenum].matrix[lmnum][1], &arm->frame[framenum].bonepose[bonenum].matrix[lmnum][2], &arm->frame[framenum].bonepose[bonenum].matrix[lmnum][3]);

      lmnum++;
      continue;
    }

  }

  LinkBones(arm);
  return arm;
}

Bone *GetBoneByName(Armature *arm, char name[80])
{
  int i;
  if(arm == NULL)return NULL;/*invalid Armature*/
  for(i = 0; i < arm->numbones;i++)
  {
    if(strncmp(arm->bones[i].name,name,80) == 0)
    {
      return &arm->bones[i];
    }
  }
  return NULL;/*invalid name*/
}

int GetBoneIndexByName(Armature *arm, char name[80])
{
  int i;
  if(arm == NULL)return -1;/*invalid Armature*/
  for(i = 0; i < arm->numbones;i++)
  {
    if(strncmp(arm->bones[i].name,name,80) == 0)
    {
      return i;
    }
  }
  return -1;/*invalid name*/
}

BonePose *GetPoseBoneByName(Armature *arm, char name[80], int frame)
{
  int i;
  if(arm == NULL)return NULL;
  for(i = 0; i < arm->frame[frame].numbones;i++)
  {
    if(strncmp(arm->frame[frame].bonepose[i].name,name,80) == 0)
    {
      return &arm->frame[frame].bonepose[i];
    }
  }
  return NULL;
}

    /*creates the links for parents/children from names*/
void LinkBones(Armature *arm)
{
  int i,j;
  if(arm == NULL)return;
  for(i = 0;i < arm->numbones;i++)
  {
    if(arm->bones[i].hasparent)
    {
      arm->bones[i].parent = GetBoneByName(arm, arm->bones[i].pname);
    }
    else arm->root = &arm->bones[i];
    for(j = 0;j < arm->bones[i].childcount;j++)
    {
      arm->bones[i].child[j] = GetBoneByName(arm, arm->bones[i].cname[j]);
    }
  }
}



void Mat4MulVecfl( float mat[][4], float *vec)
{
  float x,y;

  x=vec[0]; 
  y=vec[1];
  vec[0]=x*mat[0][0] + y*mat[1][0] + mat[2][0]*vec[2] + mat[3][0];
  vec[1]=x*mat[0][1] + y*mat[1][1] + mat[2][1]*vec[2] + mat[3][1];
  vec[2]=x*mat[0][2] + y*mat[1][2] + mat[2][2]*vec[2] + mat[3][2];
}


void Mat4MulMat4(float m1[][4], float m2[][4], float m3[][4])
{
  /* matrix product: m1[j][k] = m2[j][i].m3[i][k] */

  m1[0][0] = m2[0][0]*m3[0][0] + m2[0][1]*m3[1][0] + m2[0][2]*m3[2][0] + m2[0][3]*m3[3][0];
  m1[0][1] = m2[0][0]*m3[0][1] + m2[0][1]*m3[1][1] + m2[0][2]*m3[2][1] + m2[0][3]*m3[3][1];
  m1[0][2] = m2[0][0]*m3[0][2] + m2[0][1]*m3[1][2] + m2[0][2]*m3[2][2] + m2[0][3]*m3[3][2];
  m1[0][3] = m2[0][0]*m3[0][3] + m2[0][1]*m3[1][3] + m2[0][2]*m3[2][3] + m2[0][3]*m3[3][3];

  m1[1][0] = m2[1][0]*m3[0][0] + m2[1][1]*m3[1][0] + m2[1][2]*m3[2][0] + m2[1][3]*m3[3][0];
  m1[1][1] = m2[1][0]*m3[0][1] + m2[1][1]*m3[1][1] + m2[1][2]*m3[2][1] + m2[1][3]*m3[3][1];
  m1[1][2] = m2[1][0]*m3[0][2] + m2[1][1]*m3[1][2] + m2[1][2]*m3[2][2] + m2[1][3]*m3[3][2];
  m1[1][3] = m2[1][0]*m3[0][3] + m2[1][1]*m3[1][3] + m2[1][2]*m3[2][3] + m2[1][3]*m3[3][3];

  m1[2][0] = m2[2][0]*m3[0][0] + m2[2][1]*m3[1][0] + m2[2][2]*m3[2][0] + m2[2][3]*m3[3][0];
  m1[2][1] = m2[2][0]*m3[0][1] + m2[2][1]*m3[1][1] + m2[2][2]*m3[2][1] + m2[2][3]*m3[3][1];
  m1[2][2] = m2[2][0]*m3[0][2] + m2[2][1]*m3[1][2] + m2[2][2]*m3[2][2] + m2[2][3]*m3[3][2];
  m1[2][3] = m2[2][0]*m3[0][3] + m2[2][1]*m3[1][3] + m2[2][2]*m3[2][3] + m2[2][3]*m3[3][3];

  m1[3][0] = m2[3][0]*m3[0][0] + m2[3][1]*m3[1][0] + m2[3][2]*m3[2][0] + m2[3][3]*m3[3][0];
  m1[3][1] = m2[3][0]*m3[0][1] + m2[3][1]*m3[1][1] + m2[3][2]*m3[2][1] + m2[3][3]*m3[3][1];
  m1[3][2] = m2[3][0]*m3[0][2] + m2[3][1]*m3[1][2] + m2[3][2]*m3[2][2] + m2[3][3]*m3[3][2];
  m1[3][3] = m2[3][0]*m3[0][3] + m2[3][1]*m3[1][3] + m2[3][2]*m3[2][3] + m2[3][3]*m3[3][3];

}

void Mat4CpyMat4(float m1[][4], float m2[][4]) 
{
  memcpy(m1, m2, 4*4*sizeof(float));
}

void Mat4One(float m[][4])
{

  m[0][0]= m[1][1]= m[2][2]= m[3][3]= 1.0;
  m[0][1]= m[0][2]= m[0][3]= 0.0;
  m[1][0]= m[1][2]= m[1][3]= 0.0;
  m[2][0]= m[2][1]= m[2][3]= 0.0;
  m[3][0]= m[3][1]= m[3][2]= 0.0;
}


void transferArmatureFile(Armature *arm)
{
  FILE *file;
  char filename[80];
  int i,j;
  
  sprintf(filename,"%s.ngarm",arm->name);
  sprintf(arm->filename,"%s",filename);
  file = fopen(filename,"wb");
  if(file==NULL)
  {
    exit(0);
  }
  if(fwrite(&arm->RefCount,sizeof(int),1,file) != 1)
  {
    exit(0);
  }
  if(fwrite(arm->name,80*sizeof(char),1,file) != 1)
  {
    exit(0);
  }
  if(fwrite(arm->filename,80*sizeof(char),1,file) != 1)
  {
    exit(0);
  }
  if(fwrite(&arm->numbones,sizeof(int),1,file) != 1)
  {
    exit(0);
  }
  if(fwrite(&arm->numframes,sizeof(int),1,file) != 1)
  {
    exit(0);
  }
  if(fwrite(&arm->root,sizeof(Bone),1,file) != 1)
  {
    exit(0);
  }
  for(i=0;i < arm->numframes;i++)
  {
    if(fwrite(&arm->frame[i].numbones,sizeof(int),1,file) != 1)
    {
      exit(0);
    }
    for(j=0;j < arm->numbones;j++)
    {
      if(fwrite(&arm->frame[i].bonepose[j],sizeof(BonePose),1,file) != 1)
      {
        exit(0);
      }
    }
  }
  for(i=0;i < arm->numbones;i++)
  {
    if(fwrite(&arm->bones[i].name,80*sizeof(char),1,file) != 1)
    {
      exit(0);
    }
    if(fwrite(&arm->bones[i].hasparent,sizeof(int),1,file) != 1)
    {
      exit(0);
    }
    if(fwrite(&arm->bones[i].pname,80*sizeof(char),1,file) != 1)
    {
      exit(0);
    }
    if(fwrite(&arm->bones[i].childcount,sizeof(int),1,file) != 1)
    {
      exit(0);
    }
    if(fwrite(&arm->bones[i].cname,MAX_KIDS*80*sizeof(char),1,file) != 1)
    {
      exit(0);
    }
    if(fwrite(&arm->bones[i].numVerts,sizeof(int),1,file) != 1)
    {
      exit(0);
    }
    if(fwrite(&arm->bones[i].curVert,sizeof(int),1,file) != 1)
    {
      exit(0);
    }
    if(fwrite(&arm->bones[i].matrix,sizeof(float)*4*4,1,file) != 1)
    {
      exit(0);
    }
    if(arm->bones[i].numVerts>0)
    {
      if(fwrite(arm->bones[i].vindices,(arm->bones[i].numVerts)*sizeof(int),1,file) != 1)
      {
        exit(0);
      }
      if(fwrite(arm->bones[i].vWeights,(arm->bones[i].numVerts)*sizeof(float),1,file) != 1)
      {
        exit(0);
      }
    }

  }
  fclose(file);	
}

Armature *LoadArmatureFile(char *filename)
{
  FILE *file;
  int i,j,k;

  for(k = 0; k < MAXARMS; k++)
  {
    if(strncmp(filename,ArmList[k].filename,50)==0)
    {
      return &ArmList[k];
    }
  }
  for(k = 0;k <= MAXARMS;k++)
  {
    if(ArmList[k].used != 1)break;
  }
  ArmList[k].used=1;
  NumArms++;

  file = fopen(filename,"rb");
  if(file==NULL)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
    return NULL;
  }
  if(fread(&ArmList[k].RefCount,sizeof(int),1,file) != 1)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
    fclose(file);
    return NULL;
  }
  if(fread(ArmList[k].name,80*sizeof(char),1,file) != 1)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
    fclose(file);
    return NULL;
  }
  if(fread(ArmList[k].filename,80*sizeof(char),1,file) != 1)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
    fclose(file);
    return NULL;
  }
  if(fread(&ArmList[k].numbones,sizeof(int),1,file) != 1)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
    fclose(file);
    return NULL;
  }
  if(fread(&ArmList[k].numframes,sizeof(int),1,file) != 1)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
    fclose(file);
    return NULL;
  }
  if(fread(&ArmList[k].root,sizeof(Bone),1,file) != 1)
  {
    fprintf(stderr, "failed to load file %s.\n",filename);
    fclose(file);
    return NULL;
  }
  ArmList[k].frame = (Pose*)malloc((ArmList[k].numframes)*sizeof(Pose));
  for(i=0;i < ArmList[k].numframes;i++)
  {
    if(fread(&ArmList[k].frame[i].numbones,sizeof(int),1,file) != 1)
    {
      fprintf(stderr, "failed to load file %s.\n",filename);
      fclose(file);
      return NULL;
    }
    ArmList[k].frame[i].bonepose = (BonePose*)malloc((ArmList[k].numbones)*sizeof(BonePose));
    for(j=0;j < ArmList[k].numbones;j++)
    {
      if(fread(&ArmList[k].frame[i].bonepose[j],sizeof(BonePose),1,file) != 1)
      {
        fprintf(stderr, "failed to load file %s.\n",filename);
        fclose(file);
        return NULL;
      }
    }
  }
  ArmList[k].bones = (Bone*)malloc((ArmList[k].numbones)*sizeof(Bone));
  for(i=0;i < ArmList[k].numbones;i++)
  {
    if(fread(ArmList[k].bones[i].name,80*sizeof(char),1,file) != 1)
    {
      fprintf(stderr, "failed to load file %s.\n",filename);
      fclose(file);
      return NULL;
    }
    if(fread(&ArmList[k].bones[i].hasparent,sizeof(int),1,file) != 1)
    {
      fprintf(stderr, "failed to load file %s.\n",filename);
      fclose(file);
      return NULL;
    }
    if(fread(ArmList[k].bones[i].pname,80*sizeof(char),1,file) != 1)
    {
      fprintf(stderr, "failed to load file %s.\n",filename);
      fclose(file);
      return NULL;
    }
    if(fread(&ArmList[k].bones[i].childcount,sizeof(int),1,file) != 1)
    {
      fprintf(stderr, "failed to load file %s.\n",filename);
      fclose(file);
      return NULL;
    }
    if(fread(ArmList[k].bones[i].cname,MAX_KIDS*80*sizeof(char),1,file) != 1)
    {
      fprintf(stderr, "failed to load file %s.\n",filename);
      fclose(file);
      return NULL;
    }
    if(fread(&ArmList[k].bones[i].numVerts,sizeof(int),1,file) != 1)
    {
      fprintf(stderr, "failed to load file %s.\n",filename);
      fclose(file);
      return NULL;
    }
    if(fread(&ArmList[k].bones[i].curVert,sizeof(int),1,file) != 1)
    {
      fprintf(stderr, "failed to load file %s.\n",filename);
      fclose(file);
      return NULL;
    }
    if(fread(&ArmList[k].bones[i].matrix,sizeof(float)*4*4,1,file) != 1)
    {
      fprintf(stderr, "failed to load file %s.\n",filename);
      fclose(file);
      return NULL;
    }
    if(ArmList[k].bones[i].numVerts>0)
    {
      ArmList[k].bones[i].vindices = (int*)malloc((ArmList[k].bones[i].numVerts)*sizeof(int));
      if(fread(ArmList[k].bones[i].vindices,(ArmList[k].bones[i].numVerts)*sizeof(int),1,file) != 1)
      {
        fprintf(stderr, "failed to load file %s.\n",filename);
        fclose(file);
        return NULL;
      }
      ArmList[k].bones[i].vWeights = (float*)malloc((ArmList[k].bones[i].numVerts)*sizeof(float));
      if(fread(ArmList[k].bones[i].vWeights,(ArmList[k].bones[i].numVerts)*sizeof(float),1,file) != 1)
      {
        fprintf(stderr, "failed to load file %s.\n",filename);
        fclose(file);
        return NULL;
      }
    }
  }
  strncpy(ArmList[k].filename,filename,50);
  fclose(file);	
  LinkBones(&ArmList[k]);
  ArmList[k].used=1;
  return &ArmList[k];
}

/*end of file*/


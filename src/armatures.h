#ifndef __ARMATURES__
#define __ARMATURES__

#include "graphics.h"

#define MAX_KIDS 32
#define MAXARMS  64

typedef struct S_Bone
{
  char    name[80];                 /*name of bone = name of associate vertex group*/
  int     hasparent;                /*0 = no parent, 1 = has parent*/
  char    pname[80];                /*name of parent bone*/
  struct S_Bone *parent;            /*pointer to parent, NULL if not assigned or none*/
  int     childcount;               /*number of children*/
  char    cname[MAX_KIDS][80];      /*names of children*/
  struct S_Bone* child[MAX_KIDS];   /*array of pointers to the kids*/
  float   matrix[4][4];             /*the matrix for the initial rotation*/
  int     numVerts;
  int     *vindices;
  float   *vWeights;
  int     curVert;
}Bone;

typedef struct S_BonePose
{
  char    name[80];                 /*seams redundant, but needed to search by*/
  float   matrix[4][4];               /*matrix rotation of bone*/
}BonePose;

typedef struct S_Pose
{
  int       numbones;               /*number of bones, equal to the number of bones in the armature*/
  BonePose  *bonepose;              /*list of bone poses*/
}Pose;

typedef struct S_Armature
{
  int     RefCount;                 /*the number of objects using this armature*/
  char    name[80];                 /*name of the armature*/
  char    filename[80];                 /*name of the armature*/
  int     numbones;                 /*how many bones are in the skeleton*/
  Bone    *root;                    /*pointer to the bone that is teh root.  Once armature cannot have more than one root.*/
  Bone    *bones;                   /*the list of bones in this armature*/
  int     numframes;                /*the number of frames of animation for this armature*/
  Pose    *frame;                   /*the list of poses for each frame*/
  float   obmatrix[4][4];               /*matrix rotation of object*/
  float   armmatrix[4][4];               /*matrix rotation of armature*/
  int used;
}Armature;

void InitArmatures();/*clears and zeroes the list of armatures*/
void CloseArmatures();/*clears all poses, bones and aramtures*/
/*New Armature flags an armature from the list, and if given the number of bones will allocate the bones.  If it is also given frames (and bones) it will allocate those as well.*/
Armature *NewArmature(char name[80], int numbones,int numframes);
/*Allocates the bones for an armature, if bones already exist, deletes them and allocates new ones.*/
int  AllocateBones(Armature *arm,int numbones);
/*Allocates the frames for an armature IF bones already exist.  If frames already exist, it deletes them and allocates new ones.*/
int  AllocateFrames(Armature *arm,int numframes);
void LinkBones(Armature *arm);    /*creates the links for parents/children from names*/
Armature *LoadArmatureObj(char filename[80]);/*loads the armature dater from file & puts it into the armature structure,  returns a pointer to the armature or NULL on failure.*/
void FreeArmature(Armature *arm); /*we're done with a single armature, please remove refcount*/

Bone *GetBoneByName(Armature *arm, char name[80]);
int GetBoneIndexByName(Armature *arm, char name[80]);
BonePose *GetPoseBoneByName(Armature *arm, char name[80], int frame);
int ArmatureFirstPass(char filename[80],char groupName[80] );
void Mat4MulVecfl( float mat[][4], float *vec);
void Mat4One(float m[][4]);
void Mat4MulMat4(float m1[][4], float m2[][4], float m3[][4]);
void Mat4CpyMat4(float m1[][4], float m2[][4]);

void transferArmatureFile(Armature *arm);
Armature *LoadArmatureFile(char *filename);
#endif

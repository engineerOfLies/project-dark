/*    
      glm.c
      Nate Robins, 1997, 2000
      nate@pobox.com, http://www.pobox.com/~nate

      Wavefront OBJ model file format reader/writer/manipulator.

      Includes routines for generating smooth normals with
      preservation of edges, welding redundant vertices & texture
      coordinate generation (spheremap and planar projections) + more.
*/


#include "glm.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define T(x) (model->triangles[(x)])


/* glmFindGroup: Find a group in the model */
GLMgroup*
glmFindGroup(GLMmodel* model, char* name)
{
    GLMgroup* group;
    
    assert(model);
    
    group = model->groups;
    while(group) {
        if (!strcmp(name, group->name))
            break;
        group = group->next;
    }
    
    return group;
}

/* glmAddGroup: Add a group to the model */
GLMgroup*
glmAddGroup(GLMmodel* model, char* name)
{
    GLMgroup* group;
    
    group = glmFindGroup(model, name);
    if (!group) {
        group = (GLMgroup*)malloc(sizeof(GLMgroup));
        group->name = strdup(name);
        group->numtriangles = 0;
        group->triangles = NULL;
        group->next = model->groups;
        model->groups = group;
        model->numgroups++;
    }
    
    return group;
}


/* glmFirstPass: first pass at a Wavefront OBJ file that gets all the
 * statistics of the model (such as #vertices, #normals, etc)
 * Also allocates memory in each groups for triangle _indices_ (not the triangles themselves though.)
 *
 * model - properly initialized GLMmodel structure
 * file  - (fopen'd) file descriptor
 */
static GLvoid
glmFirstPass(GLMmodel* model, FILE* file) 
{
    GLuint  numvertices;        /* number of vertices in model */
    GLuint  numnormals;         /* number of normals in model */
    GLuint  numtexcoords;       /* number of texcoords in model */
    GLuint  numtriangles;       /* number of triangles in model */
    GLMgroup* group;            /* current group */
    int    v, n, t;
    char        buf[128];
    /* make a default group */
    group = glmAddGroup(model, "default");
    
    numvertices = numnormals = numtexcoords = numtriangles = 0;
    while(fscanf(file, "%s", buf) != EOF) {
        switch(buf[0]) {
        case '#':               /* comment */
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        case 'v':               /* v, vn, vt */
            switch(buf[1]) {
            case '\0':          /* vertex */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                numvertices++;
                break;
            case 'n':           /* normal */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                numnormals++;
                break;
            case 't':           /* texcoord */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                numtexcoords++;
                break;
            default:
                fprintf(stderr,"glmFirstPass(): Unknown token \"%s\".\n", buf);
                exit(1);
                break;
            }
            break;

		case 'u':				/* usemtl */
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
/*    case 'o':
      fgets(buf, sizeof(buf), file);
      sscanf(buf, "%s", buf);
      group = glmAddGroup(model, buf);
      break;*/
		case 'o':               /* group */
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
#if SINGLE_STRING_GROUP_NAMES
			sscanf(buf, "%s", buf);
			group = glmAddGroup(model, buf);
#else
			buf[strlen(buf) - 1] = '\0';  /* nuke '\n' */
			group = glmAddGroup(model, buf+1);
#endif
			break;
		case 'f':               /* face */
			v = n = t = 0;
			fscanf(file, "%s", buf);
			/* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
			/* Faces with > 3 vertices will be broken into triangles. */
			if (strstr(buf, "//")) {
				/* v//n */
				sscanf(buf, "%d//%d", &v, &n);
				fscanf(file, "%d//%d", &v, &n);
				fscanf(file, "%d//%d", &v, &n);
				numtriangles++;
				group->numtriangles++;
				while(fscanf(file, "%d//%d", &v, &n) > 0) {
					numtriangles++;
					group->numtriangles++;
				}
			} else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
				/* v/t/n */
				fscanf(file, "%d/%d/%d", &v, &t, &n);
				fscanf(file, "%d/%d/%d", &v, &t, &n);
				numtriangles++;
				group->numtriangles++;
				while(fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
					numtriangles++;
					group->numtriangles++;
				}
			} else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
				/* v/t */
				fscanf(file, "%d/%d", &v, &t);
				fscanf(file, "%d/%d", &v, &t);
				numtriangles++;
				group->numtriangles++;
				while(fscanf(file, "%d/%d", &v, &t) > 0) {
					numtriangles++;
					group->numtriangles++;
				}
			} else {
				/* v */
				fscanf(file, "%d", &v);
				fscanf(file, "%d", &v);
				numtriangles++;
				group->numtriangles++;
				while(fscanf(file, "%d", &v) > 0) {
					numtriangles++;
					group->numtriangles++;
				}
			}
			break;
                
		default:
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
        }
	}
  
	/* set the stats in the model structure */
	model->numvertices  = numvertices;
	model->numnormals   = numnormals;
	model->numtexcoords = numtexcoords;
	model->numtriangles = numtriangles;
  
	/* allocate memory for the triangle _indices_ in each group */
	group = model->groups;
	while(group) {
		group->triangles = (GLuint*)malloc(sizeof(GLuint) * group->numtriangles);
		group->numtriangles = 0;
		group = group->next;
	}
}

/* glmSecondPass: second pass at a Wavefront OBJ file that gets all
 * the data.
 *
 * model - properly initialized GLMmodel structure
 * file  - (fopen'd) file descriptor 
 */
static GLvoid
glmSecondPass(GLMmodel* model, FILE* file) 
{
	GLuint		numvertices;		/* number of vertices in model */
    GLuint		numnormals;			/* number of normals in model */
    GLuint		numtexcoords;		/* number of texcoords in model */
    GLuint		numtriangles;		/* number of triangles in model */
    GLfloat*    vertices;			/* array of vertices  */
    GLfloat*    normals;			/* array of normals */
    GLfloat*    texcoords;			/* array of texture coordinates */
    GLMgroup*	group;				/* current group pointer */

    long int		v, n, t;
    char        buf[128];			
    
    /* set the pointer shortcuts */
    vertices	= model->vertices;
    normals		= model->normals;
    texcoords	= model->texcoords;
    group		= model->groups;
    
    /* on the second pass through the file, read all the data into the
    allocated arrays */
    numvertices = numnormals = numtexcoords = 1;	
    numtriangles = 0;

    while(fscanf(file, "%s", buf) != EOF) {
        switch(buf[0]) {
        case '#':               /* comment */
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        case 'v':               /* v, vn, vt */
            switch(buf[1]) {
            case '\0':          /* vertex */
                fscanf(file, "%f %f %f", 
                    &vertices[3 * numvertices + 0], 
                    &vertices[3 * numvertices + 1], 
                    &vertices[3 * numvertices + 2]);
                numvertices++;
                break;
            case 'n':           /* normal */
                fscanf(file, "%f %f %f", 
                    &normals[3 * numnormals + 0],
                    &normals[3 * numnormals + 1], 
                    &normals[3 * numnormals + 2]);
                numnormals++;
                break;
            case 't':           /* texcoord */
                fscanf(file, "%f %f", 
                    &texcoords[2 * numtexcoords + 0],
                    &texcoords[2 * numtexcoords + 1]);
                texcoords[2 * numtexcoords + 1] = 1 - texcoords[2 * numtexcoords + 1];
                numtexcoords++;
                break;
            }
            break;

		case 'o':               /* group */
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);		
#if SINGLE_STRING_GROUP_NAMES
			sscanf(buf, "%s", buf);				
			group = glmFindGroup(model, buf);
#else
			buf[strlen(buf)-1] = '\0';  /* nuke '\n' */
			group = glmFindGroup(model, buf+1);	
#endif
			break;
		case 'f':               /* face */
			v = n = t = 0;
			fscanf(file, "%s", buf);
			/* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
			/* Faces with > 3 vertices will be broken into triangle fans. */
			if (strstr(buf, "//")) {
				/* v//n */
				sscanf(buf, "%ld//%ld", &v, &n);
				T(numtriangles).vindices[0] = v;
				T(numtriangles).nindices[0] = n;
				fscanf(file, "%ld//%ld", &v, &n);
				T(numtriangles).vindices[1] = v;
				T(numtriangles).nindices[1] = n;
				fscanf(file, "%ld//%ld", &v, &n);
				T(numtriangles).vindices[2] = v;
				T(numtriangles).nindices[2] = n;
				group->triangles[group->numtriangles++] = numtriangles;
				numtriangles++;
				while(fscanf(file, "%ld//%ld", &v, &n) > 0) {
					T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
					T(numtriangles).nindices[0] = T(numtriangles-1).nindices[0];
					T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
					T(numtriangles).nindices[1] = T(numtriangles-1).nindices[2];
					T(numtriangles).vindices[2] = v;
					T(numtriangles).nindices[2] = n;
					group->triangles[group->numtriangles++] = numtriangles;
					numtriangles++;
				}
			} else if (sscanf(buf, "%ld/%ld/%ld", &v, &t, &n) == 3) {
				/* v/t/n */
				T(numtriangles).vindices[0] = v;
				T(numtriangles).tindices[0] = t;
				T(numtriangles).nindices[0] = n;
				fscanf(file, "%ld/%ld/%ld", &v, &t, &n);
				T(numtriangles).vindices[1] = v;
				T(numtriangles).tindices[1] = t;
				T(numtriangles).nindices[1] = n;
				fscanf(file, "%ld/%ld/%ld", &v, &t, &n);
				T(numtriangles).vindices[2] = v;
				T(numtriangles).tindices[2] = t;
				T(numtriangles).nindices[2] = n;
				group->triangles[group->numtriangles++] = numtriangles;
				numtriangles++;
				while(fscanf(file, "%ld/%ld/%ld", &v, &t, &n) > 0) {
					T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
					T(numtriangles).tindices[0] = T(numtriangles-1).tindices[0];
					T(numtriangles).nindices[0] = T(numtriangles-1).nindices[0];
					T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
					T(numtriangles).tindices[1] = T(numtriangles-1).tindices[2];
					T(numtriangles).nindices[1] = T(numtriangles-1).nindices[2];
					T(numtriangles).vindices[2] = v;
					T(numtriangles).tindices[2] = t;
					T(numtriangles).nindices[2] = n;
					group->triangles[group->numtriangles++] = numtriangles;
					numtriangles++;
				}
			} else if (sscanf(buf, "%ld/%ld", &v, &t) == 2) {
				/* v/t */
				T(numtriangles).vindices[0] = v;
				T(numtriangles).tindices[0] = t;
				fscanf(file, "%ld/%ld", &v, &t);
				T(numtriangles).vindices[1] = v;
				T(numtriangles).tindices[1] = t;
				fscanf(file, "%ld/%ld", &v, &t);
				T(numtriangles).vindices[2] = v;
				T(numtriangles).tindices[2] = t;
				group->triangles[group->numtriangles++] = numtriangles;
				numtriangles++;
				while(fscanf(file, "%ld/%ld", &v, &t) > 0) {
					T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
					T(numtriangles).tindices[0] = T(numtriangles-1).tindices[0];
					T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
					T(numtriangles).tindices[1] = T(numtriangles-1).tindices[2];
					T(numtriangles).vindices[2] = v;
					T(numtriangles).tindices[2] = t;
					group->triangles[group->numtriangles++] = numtriangles;
					numtriangles++;
				}
			} else {
				/* v */
				sscanf(buf, "%ld", &v);
				T(numtriangles).vindices[0] = v;
				fscanf(file, "%ld", &v);
				T(numtriangles).vindices[1] = v;
				fscanf(file, "%ld", &v);
				T(numtriangles).vindices[2] = v;
				group->triangles[group->numtriangles++] = numtriangles;
				numtriangles++;
				while(fscanf(file, "%ld", &v) > 0) {
					T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
					T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
					T(numtriangles).vindices[2] = v;
					group->triangles[group->numtriangles++] = numtriangles;
					numtriangles++;
				}
			}
			break;
		default:
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		}	/* switch(buf[0]); */
	}	/* !EOF */
}



/* glmDimensions: Calculates the dimensions (width, height, depth) of
 * a model.
 *
 * model   - initialized GLMmodel structure
 * dimensions - array of 3 GLfloats (GLfloat dimensions[3])
 */
GLvoid
glmDimensions(GLMmodel* model, GLfloat* dimensions)
{
    GLuint i;
    GLfloat maxx, minx, maxy, miny, maxz, minz;
    
    assert(model);
    assert(model->vertices);
    assert(dimensions);
    
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
    dimensions[0] = maxx - minx;
    dimensions[1] = maxy - miny;
    dimensions[2] = maxz - minz;
    minx = 0;
    miny = 0;
    for(i = 0; i <= model->numtexcoords;i++)
    {
      minx += model->texcoords[2 * i + 0];
      miny += model->texcoords[2 * i + 1];      
     
    }
    model->uvcenter[0] = minx / (model->numtexcoords + 1);
    model->uvcenter[1] = miny / (model->numtexcoords + 1);
}

/* glmScale: Scales a model by a given amount.
 * 
 * model - properly initialized GLMmodel structure
 * scale - scalefactor (0.5 = half as large, 2.0 = twice as large)
 */
GLvoid
glmScale(GLMmodel* model, GLfloat scale)
{
    GLuint i;
    
    for (i = 1; i <= model->numvertices; i++) {
        model->vertices[3 * i + 0] *= scale;
        model->vertices[3 * i + 1] *= scale;
        model->vertices[3 * i + 2] *= scale;
    }
}


/* glmDelete: Deletes a GLMmodel structure.
 *
 * model - initialized GLMmodel structure
 */
GLvoid
glmDelete(GLMmodel* model)
{
    GLMgroup* group;
    
    assert(model);
    
    if (model->pathname)	free(model->pathname);
    if (model->mtllibname)	free(model->mtllibname);
    if (model->vertices)	free(model->vertices);
    if (model->normals)		free(model->normals);
    if (model->texcoords)	free(model->texcoords);
    if (model->facetnorms)	free(model->facetnorms);
    if (model->triangles)	free(model->triangles);
    while(model->groups) {
        group = model->groups;
        model->groups = model->groups->next;
        free(group->name);
        free(group->triangles);
        free(group);
    }
    
    free(model);
}

/* glmReadOBJ: Reads a model description from a Wavefront .OBJ file.
 * Returns a pointer to the created object which should be free'd with
 * glmDelete().
 *
 * filename - name of the file containing the Wavefront .OBJ format data.
 * contextIndex - PRL: index to the current OpenGL context (for texturing.) If you have only
 *             one OpenGL context (the most common case) set this parameter to 0.
 */
GLMmodel* 
glmReadOBJ(char* filename)
{
    GLMmodel* model;
    FILE*   file;
    
    /* open the file */
    file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "glmReadOBJ() failed: can't open data file \"%s\".\n",
            filename);
        return NULL;
    }
    
    /* allocate a new model */
    model = (GLMmodel*)malloc(sizeof(GLMmodel));
    model->pathname		= strdup(filename);
    model->mtllibname	= NULL;
    model->numvertices	= 0;
    model->vertices		= NULL;
    model->numnormals	= 0;
    model->normals		= NULL;
    model->numtexcoords	= 0;
    model->texcoords	= NULL;
    model->numfacetnorms	= 0;
    model->facetnorms	= NULL;
    model->numtriangles	= 0;
    model->triangles	= NULL;
    model->numgroups	= 0;
    model->groups		= NULL;
    model->position[0]   = 0.0;
    model->position[1]   = 0.0;
    model->position[2]   = 0.0;
    
    /* make a first pass through the file to get a count of the number
    of vertices, normals, texcoords & triangles */
    glmFirstPass(model, file);
    
    /* allocate memory */
    model->vertices = (GLfloat*)malloc(sizeof(GLfloat) *
        3 * (model->numvertices + 1));			
    model->triangles = (GLMtriangle*)malloc(sizeof(GLMtriangle) *
        model->numtriangles);
    if (model->numnormals) {
        model->normals = (GLfloat*)malloc(sizeof(GLfloat) *
            3 * (model->numnormals + 1));		
    }
    if (model->numtexcoords) {
        model->texcoords = (GLfloat*)malloc(sizeof(GLfloat) *
            2 * (model->numtexcoords + 1));		
    }
    
    /* rewind to beginning of file and read in the data this pass */
    rewind(file);
    
    glmSecondPass(model, file);
    
    /* close the file */
    fclose(file);
    
    return model;
}







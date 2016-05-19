#ifndef __3DS_H
#define __3DS_H

struct chunk3ds_t
{
	unsigned short id;
	unsigned long length;
};

struct material3ds_t
{
	char     name[32];
	float    ambient[4];
	float    diffuse[4];
	float    specular[4];
	float    emission[4];
	float    shininess;
	char     texture[32];
};

struct group3ds_t
{
	char     name[32];
	int      mat;
	int      start;
	int      size;
	long     *tris;
};

struct mesh3ds_t
{
	char           name[32];
	int            vertCount;
	int            texCoordCount;
	int            triCount;
	int            groupCount;
	float          (*verts)[3];
	float          (*norms)[3];
	float          (*texCoords)[2];
	float          (*tangentSpace)[9];
	long           (*tris)[3];
	group3ds_t     *groups;
	long           *smooth;
	float          axis[3][3];
	float          position[3];
	float          min[3];
	float          max[3];
	float          center[3];
};

struct data3ds_t
{
	int            materialCount;
	int            meshCount;
	int            vertCount;
	int            triCount;
	material3ds_t  *materials;
	mesh3ds_t      *meshes;
	float          min[3];
	float          max[3];
	float          center[3];
};

struct triangle_t
{
   long     mat;
   long     verts[3];
   long     index;
   long     smooth;
};

long Read3dsFile(char const* pFilename, data3ds_t* output);
void Free3dsData(data3ds_t* data);
void Calculate3dsNormals(data3ds_t* model);
void Calculate3dsTangentSpace(data3ds_t* model);
void Calculate3dsBoundingBox(data3ds_t* model);

#endif

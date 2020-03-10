#pragma once

#include "vertexBufferObject.h"

//sphere
unsigned int const rings = 14;
unsigned int const sectors = 120;
unsigned int const ringSectors = 670;

extern glm::vec3 vCubeVertices[36];
//extern glm::vec3 vCubeVertices2[24];
extern glm::vec3 vCubeVertices2[24];
extern unsigned int iCubeindices[36];
extern unsigned int iSphereindices[rings*sectors*6];
extern unsigned int iRingindices[ringSectors * 6];
extern glm::vec2 vCubeTexCoords[6];
extern glm::vec3 vCubeNormals[6];
extern glm::vec3 vGround[6];


int GenerateTorus(CVertexBufferObject &vboDest, float fRadius, float fTubeRadius, int iSubDivAround, int iSubDivTube);
int GenerateCilinder(CVertexBufferObject &vboDest, float fRadius, float fHeight, int iSubDivAround, float fMapU = 1.0f, float fMapV = 1.0f);
void AddSceneObjects(CVertexBufferObject& vboDest);
void AddCube(CVertexBufferObject& vboDest);
void SolidSphere(CVertexBufferObject& vboDest, float fRadius);
void Ring(CVertexBufferObject &vboDest, float innerRadius, float outerRadius);
extern int iTorusFaces, iTorusFaces2, iCilinderFaces;
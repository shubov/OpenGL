#include "common_header.h"

#include "static_geometry.h"

unsigned int iSphereindices[rings * sectors * 6];
unsigned int iRingindices[ringSectors * 6];
const float PI = float(atan(1.0)*4.0);

void Ring(CVertexBufferObject &vboDest, float innerRadius, float outerRadius) {
	glm::vec3 vertices[ringSectors * 2];
	glm::vec3 normals[ringSectors * 2];
	glm::vec2 texcoords[ringSectors * 2];
	float radius[2] = { innerRadius, outerRadius };

	float const R = 1.0f / (ringSectors-1);

	int v = 0;
	int n = 0;
	int t = 0;
	for (int r = 0; r < ringSectors; r++) 
		for (int i = 0; i <= 1; i++) {
			float const y = 0;
			float const x = cos(2 * PI * r * R);
			float const z = sin(2 * PI * r * R);

			vertices[v] = glm::vec3(x * radius[i], y * radius[i], z * radius[i]);
			vboDest.AddData(&vertices[v++], sizeof(glm::vec3));

			texcoords[t] = glm::vec2(i, r*R);
			vboDest.AddData(&texcoords[t++], sizeof(glm::vec2));

			normals[n] = glm::vec3(0.0f, 0.0f, 1.0f);
			vboDest.AddData(&normals[n++], sizeof(glm::vec3));
		}

}

void SolidSphere(CVertexBufferObject &vboDest, float fRadius)
{
	glm::vec3 vertices[rings * sectors];
	glm::vec3 normals[rings * sectors];
	glm::vec2 texcoords[rings * sectors];

	float const R = 1.0f / (float)(rings - 1);
	float const S = 1.0f / (float)(sectors - 1);
	int r, s;

	int v = 0;
	int n = 0;
	int t = 0;
	for (r = 0; r < rings; r++) for (s = 0; s < sectors; s++) {
		float const y = sin(-PI / 2 + PI * r * R);
		float const x = cos(2 * PI * s * S) * sin(PI * r * R);
		float const z = sin(2 * PI * s * S) * sin(PI * r * R);

		vertices[v] = glm::vec3(x * fRadius, y * fRadius, z * fRadius);
		vboDest.AddData(&vertices[v++], sizeof(glm::vec3));

		texcoords[t] = glm::vec2(1.0f - s*S, r*R);
		vboDest.AddData(&texcoords[t++], sizeof(glm::vec2));

		normals[n] = glm::vec3(x, y, z);
		vboDest.AddData(&normals[n++], sizeof(glm::vec3));
	}

	int i = 0;
	for (r = 0; r < rings-1; r++) for (s = 0; s < sectors; s++) {
		iSphereindices[i++] = r * sectors + s;
		iSphereindices[i++] = r * sectors + (s + 1);
		iSphereindices[i++] = (r + 1) * sectors + (s + 1);
		iSphereindices[i++] = r * sectors + s;
		iSphereindices[i++] = (r + 1) * sectors + s;
		iSphereindices[i++] = (r + 1) * sectors + (s + 1);
	}
}

// Generates centered torus with specified parameters and stores it in VBO.
// vboDest - VBO where to store torus
// fRadius - outer radius
// fTubeRadius - inner radius
// iSubDivAround - subdivisions around torus
// iSubDivTube - subdivisions of tube
int GenerateTorus(CVertexBufferObject &vboDest, float fRadius, float fTubeRadius, int iSubDivAround, int iSubDivTube)
{
	float fAddAngleAround = 360.0f / (float)iSubDivAround;
	float fAddAngleTube = 360.0f / (float)iSubDivTube;

	float fCurAngleAround = 0.0f;
	int iStepsAround = 1;
	const float PI = float(atan(1.0)*4.0);

	int iFacesAdded = 0;

	while (iStepsAround <= iSubDivAround)
	{
		float fSineAround = sin(fCurAngleAround / 180.0f*PI);
		float fCosineAround = cos(fCurAngleAround / 180.0f*PI);
		glm::vec3 vDir1(fSineAround, fCosineAround, 0.0f);
		float fNextAngleAround = fCurAngleAround + fAddAngleAround;
		float fNextSineAround = sin(fNextAngleAround / 180.0f*PI);
		float fNextCosineAround = cos(fNextAngleAround / 180.0f*PI);
		glm::vec3 vDir2(fNextSineAround, fNextCosineAround, 0.0f);
		float fCurAngleTube = 0.0f;
		int iStepsTube = 1;
		while (iStepsTube <= iSubDivTube)
		{
			float fSineTube = sin(fCurAngleTube / 180.0f*PI);
			float fCosineTube = cos(fCurAngleTube / 180.0f*PI);
			float fNextAngleTube = fCurAngleTube + fAddAngleTube;
			float fNextSineTube = sin(fNextAngleTube / 180.0f*PI);
			float fNextCosineTube = cos(fNextAngleTube / 180.0f*PI);
			glm::vec3 vMid1 = vDir1*(fRadius - fTubeRadius / 2), vMid2 = vDir2*(fRadius - fTubeRadius / 2);
			glm::vec3 vQuadPoints[] =
			{
				vMid1 + glm::vec3(0.0f, 0.0f, -fNextSineTube*fTubeRadius) + vDir1*fNextCosineTube*fTubeRadius,
				vMid1 + glm::vec3(0.0f, 0.0f, -fSineTube*fTubeRadius) + vDir1*fCosineTube*fTubeRadius,
				vMid2 + glm::vec3(0.0f, 0.0f, -fSineTube*fTubeRadius) + vDir2*fCosineTube*fTubeRadius,
				vMid2 + glm::vec3(0.0f, 0.0f, -fNextSineTube*fTubeRadius) + vDir2*fNextCosineTube*fTubeRadius
			};

			glm::vec3 vNormals[] =
			{
				glm::vec3(0.0f, 0.0f, -fNextSineTube) + vDir1*fNextCosineTube,
				glm::vec3(0.0f, 0.0f, -fSineTube) + vDir1*fCosineTube,
				glm::vec3(0.0f, 0.0f, -fSineTube) + vDir2*fCosineTube,
				glm::vec3(0.0f, 0.0f, -fNextSineTube) + vDir2*fNextCosineTube
			};

			glm::vec2 vTexCoords[] =
			{
				glm::vec2(fCurAngleAround / 360.0f, fNextAngleTube / 360.0f),
				glm::vec2(fCurAngleAround / 360.0f, fCurAngleTube / 360.0f),
				glm::vec2(fNextAngleAround / 360.0f, fCurAngleTube / 360.0f),
				glm::vec2(fNextAngleAround / 360.0f, fNextAngleTube / 360.0f)
			};

			int iIndices[] = { 0, 1, 2, 2, 3, 0 };

			FOR(i, 6)
			{
				int index = iIndices[i];
				vboDest.AddData(&vQuadPoints[index], sizeof(glm::vec3));
				vboDest.AddData(&vTexCoords[index], sizeof(glm::vec2));
				vboDest.AddData(&vNormals[index], sizeof(glm::vec3));
			}
			iFacesAdded += 2; // Keep count of added faces
			fCurAngleTube += fAddAngleTube;
			iStepsTube++;
		}
		fCurAngleAround += fAddAngleAround;
		iStepsAround++;
	}
	return iFacesAdded;
}

// Generates centered opened cylinder and stores it in VBO.
// vboDest - VBO where to store torus
// fRadius - outer radius
// fHeight - height of cylinder
// iSubDivAround - subdivisions around cylinder
int GenerateCilinder(CVertexBufferObject &vboDest, float fRadius, float fHeight, int iSubDivAround, float fMapU, float fMapV)
{
	float fAddAngleAround = 360.0f / (float)(iSubDivAround - 1);

	float fCurAngleAround = 0.0f;
	int iStepsAround = 1;
	const float PI = float(atan(1.0)*4.0);

	int iFacesAdded = 0;

	while (iStepsAround <= iSubDivAround)
	{
		float fSineAround = sin(fCurAngleAround / 180.0f*PI);
		float fCosineAround = cos(fCurAngleAround / 180.0f*PI);
		glm::vec3 vDir1(fCosineAround, 0.0f, fSineAround);
		float fNextAngleAround = fCurAngleAround + fAddAngleAround;
		float fNextSineAround = sin(fNextAngleAround / 180.0f*PI);
		float fNextCosineAround = cos(fNextAngleAround / 180.0f*PI);
		glm::vec3 vDir2(fNextCosineAround, 0.0f, fNextSineAround);

		glm::vec3 vQuadPoints[] =
		{
			glm::vec3(0.0f, fHeight, 0.0f) + vDir1*fRadius,
			vDir1*fRadius,
			vDir2*fRadius,
			glm::vec3(0.0f, fHeight, 0.0f) + vDir2*fRadius
		};


		glm::vec2 vTexCoords[] =
		{
			glm::vec2(fMapU*fCurAngleAround / 360.0f, fMapV),
			glm::vec2(fMapU*fCurAngleAround / 360.0f, 0.0f),
			glm::vec2(fMapU*fNextAngleAround / 360.0f, 0.0f),
			glm::vec2(fMapU*fNextAngleAround / 360.0f, fMapV)
		};

		glm::vec3 vNormals[] =
		{
			vDir1,
			vDir1,
			vDir2,
			vDir2
		};

		int iIndices[] = { 0, 1, 2, 2, 3, 0 };

		FOR(i, 6)
		{
			int index = iIndices[i];
			vboDest.AddData(&vQuadPoints[index], sizeof(glm::vec3));
			vboDest.AddData(&vTexCoords[index], sizeof(glm::vec2));
			vboDest.AddData(&vNormals[index], sizeof(glm::vec3));
		}
		iFacesAdded += 2; // Keep count of added faces

		fCurAngleAround += fAddAngleAround;
		iStepsAround++;
	}
	return iFacesAdded;
}

glm::vec3 vCubeVertices2[24] =
{
	// Front face
	glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, 0.5f),
	// Back face
	glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f),
	// Left face
	glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f),
	// Right face
	glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f),
	// Top face
	glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f),
	// Bottom face
	glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, 0.5f),
};

unsigned int iCubeindices[36] =
{
	0, 2, 1,  1, 2, 3, // front
	4, 6, 5,  5, 6, 7, // back
	8, 10, 9,  9, 10, 11, // left
	12, 14, 13, 13, 14, 15,  // right
	16, 18, 17, 17, 18, 19, // top
	20, 22, 21, 21, 22, 23 // bottom
};

glm::vec2 vCubeTexCoords[6] = { 
	glm::vec2(0.0f, 1.0f), 
	glm::vec2(1.0f, 1.0f), 
	glm::vec2(1.0f, 0.0f), 
	glm::vec2(1.0f, 0.0f), 
	glm::vec2(0.0f, 0.0f), 
	glm::vec2(0.0f, 1.0f) 
};

glm::vec2 vCubeTexCoords2[4] = { 
	glm::vec2(0.0f, 1.0f), 
	glm::vec2(0.0f, 0.0f), 
	glm::vec2(1.0f, 1.0f), 
	glm::vec2(1.0f, 0.0f) 
};

glm::vec3 vCubeNormals[6] =
{
	glm::vec3(0.0f, 0.0f, 1.0f),
	glm::vec3(0.0f, 0.0f, -1.0f),
	glm::vec3(-1.0f, 0.0f, 0.0f),
	glm::vec3(1.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(0.0f, -1.0f, 0.0f)
};

glm::vec3 vGround[6] =
{
	glm::vec3(-1000, 0, -1000), glm::vec3(-1000, 0, 1000), glm::vec3(1000, 0, 1000), glm::vec3(1000, 0, 1000), glm::vec3(1000, 0, -1000), glm::vec3(-1000, 0, -1000)
};

int iTorusFaces, iTorusFaces2, iCilinderFaces;

// Adds all static objects to scene.
// vboDest - VBO where to store objects
void AddSceneObjects(CVertexBufferObject& vboDest)
{
	// Add ground to VBO
	FOR(i, 6)
	{
		vboDest.AddData(&vGround[i], sizeof(glm::vec3));
		glm::vec2 vCoord = vCubeTexCoords[i] * 50.0f;
		vboDest.AddData(&vCoord, sizeof(glm::vec2));
		glm::vec3 vGroundNormal(0.0f, 1.0f, 0.0f);
		vboDest.AddData(&vGroundNormal, sizeof(glm::vec3));
	}

	iTorusFaces = GenerateTorus(vboDest, 7.0f, 2.0f, 20, 20);
	iTorusFaces2 = GenerateTorus(vboDest, 3.0f, 1.0f, 20, 20);
	iCilinderFaces = GenerateCilinder(vboDest, 50.0f, 70.0f, 5, 1.0f, 1.0f);
}

// Adds all static objects to scene.
// vboDest - VBO where to store objects
void AddCube(CVertexBufferObject& vboDest)
{
	FOR(i, 24)
	{
		vboDest.AddData(&vCubeVertices2[i], sizeof(glm::vec3));
		vboDest.AddData(&vCubeTexCoords2[i % 4], sizeof(glm::vec2));
		vboDest.AddData(&vCubeNormals[i / 4], sizeof(glm::vec3));
	}
}

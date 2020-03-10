#include "common_header.h"

#include <FreeImage.h>
#include "heightmap.h"

CShaderProgram CMultiLayeredHeightmap::spTerrain;
CShader CMultiLayeredHeightmap::shTerrainShaders[NUMTERRAINSHADERS];

CMultiLayeredHeightmap::CMultiLayeredHeightmap()
{
	vRenderScale = glm::vec3(1.0f, 1.0f, 1.0f);
}

/*-----------------------------------------------
Name:	LoadHeightMapFromImage
Params:	sImagePath - path to the (optimally) grayscale
		image containing heightmap data.
Result: Loads a heightmap and builds up all OpenGL
		structures for rendering.
/*---------------------------------------------*/

bool CMultiLayeredHeightmap::LoadHeightMapFromImage(string sImagePath)
{
	if(bLoaded)
	{
		bLoaded = false;
		ReleaseHeightmap();
	}
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	FIBITMAP* dib(0);

	fif = FreeImage_GetFileType(sImagePath.c_str(), 0); // Check the file signature and deduce its format

	if(fif == FIF_UNKNOWN) // If still unknown, try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilename(sImagePath.c_str());

	if(fif == FIF_UNKNOWN) // If still unknown, return failure
		return false;

	if(FreeImage_FIFSupportsReading(fif)) // Check if the plugin has reading capabilities and load the file
		dib = FreeImage_Load(fif, sImagePath.c_str());
	if(!dib)
		return false;

	BYTE* bDataPointer = FreeImage_GetBits(dib); // Retrieve the image data
	iRows = FreeImage_GetHeight(dib);
	iCols = FreeImage_GetWidth(dib);

	// We also require our image to be either 24-bit (classic RGB) or 8-bit (luminance)
	if(bDataPointer == NULL || iRows == 0 || iCols == 0 || (FreeImage_GetBPP(dib) != 24 && FreeImage_GetBPP(dib) != 8))
		return false;

	// How much to increase data pointer to get to next pixel data
	unsigned int ptr_inc = FreeImage_GetBPP(dib) == 24 ? 3 : 1;
	// Length of one row in data
	unsigned int row_step = ptr_inc*iCols;

	vboHeightmapData.CreateVBO();
	// All vertex data are here (there are iRows*iCols vertices in this heightmap), we will get to normals later
	vector< vector< glm::vec3> > vVertexData(iRows, vector<glm::vec3>(iCols));
	vector< vector< glm::vec2> > vCoordsData(iRows, vector<glm::vec2>(iCols));

	float fTextureU = float(iCols)*0.1f;
	float fTextureV = float(iRows)*0.1f;

	FOR(i, iRows)
	{
		FOR(j, iCols)
		{
			float fScaleC = float(j)/float(iCols-1);
			float fScaleR = float(i)/float(iRows-1);
			float fVertexHeight = float(*(bDataPointer+row_step*i+j*ptr_inc))/255.0f;
			vVertexData[i][j] = glm::vec3(-0.5f+fScaleC, fVertexHeight, -0.5f+fScaleR);
			vCoordsData[i][j] = glm::vec2(fTextureU*fScaleC, fTextureV*fScaleR);
		}
	}

	// Normals are here - the heightmap contains ( (iRows-1)*(iCols-1) quads, each one containing 2 triangles, therefore array of we have 3D array)
	vector< vector<glm::vec3> > vNormals[2];
	FOR(i, 2)vNormals[i] = vector< vector<glm::vec3> >(iRows-1, vector<glm::vec3>(iCols-1));

	FOR(i, iRows-1)
	{
		FOR(j, iCols-1)
		{
			glm::vec3 vTriangle0[] = 
			{
				vVertexData[i][j],
				vVertexData[i+1][j],
				vVertexData[i+1][j+1]
			};
			glm::vec3 vTriangle1[] = 
			{
				vVertexData[i+1][j+1],
				vVertexData[i][j+1],
				vVertexData[i][j]
			};

			glm::vec3 vTriangleNorm0 = glm::cross(vTriangle0[0]-vTriangle0[1], vTriangle0[1]-vTriangle0[2]);
			glm::vec3 vTriangleNorm1 = glm::cross(vTriangle1[0]-vTriangle1[1], vTriangle1[1]-vTriangle1[2]);

			vNormals[0][i][j] = glm::normalize(vTriangleNorm0);
			vNormals[1][i][j] = glm::normalize(vTriangleNorm1);
		}
	}

	vector< vector<glm::vec3> > vFinalNormals = vector< vector<glm::vec3> >(iRows, vector<glm::vec3>(iCols));

	FOR(i, iRows)
	FOR(j, iCols)
	{
		// Now we wanna calculate final normal for [i][j] vertex. We will have a look at all triangles this vertex is part of, and then we will make average vector
		// of all adjacent triangles' normals

		glm::vec3 vFinalNormal = glm::vec3(0.0f, 0.0f, 0.0f);

		// Look for upper-left triangles
		if(j != 0 && i != 0)
			FOR(k, 2)vFinalNormal += vNormals[k][i-1][j-1];
		// Look for upper-right triangles
		if(i != 0 && j != iCols-1)vFinalNormal += vNormals[0][i-1][j];
		// Look for bottom-right triangles
		if(i != iRows-1 && j != iCols-1)
			FOR(k, 2)vFinalNormal += vNormals[k][i][j];
		// Look for bottom-left triangles
		if(i != iRows-1 && j != 0)
			vFinalNormal += vNormals[1][i][j-1];
		vFinalNormal = glm::normalize(vFinalNormal);

		vFinalNormals[i][j] = vFinalNormal; // Store final normal of j-th vertex in i-th row
	}

	// First, create a VBO with only vertex data
	vboHeightmapData.CreateVBO(iRows*iCols*(2*sizeof(glm::vec3)+sizeof(glm::vec2))); // Preallocate memory
	FOR(i, iRows)
	{
		FOR(j, iCols)
		{
			vboHeightmapData.AddData(&vVertexData[i][j], sizeof(glm::vec3)); // Add vertex
			vboHeightmapData.AddData(&vCoordsData[i][j], sizeof(glm::vec2)); // Add tex. coord
			vboHeightmapData.AddData(&vFinalNormals[i][j], sizeof(glm::vec3)); // Add normal
		}
	}
	// Now create a VBO with heightmap indices
	vboHeightmapIndices.CreateVBO();
	int iPrimitiveRestartIndex = iRows*iCols;
	FOR(i, iRows-1)
	{
		FOR(j, iCols)
		FOR(k, 2)
		{
			int iRow = i+(1-k);
			int iIndex = iRow*iCols+j;
			vboHeightmapIndices.AddData(&iIndex, sizeof(int));
		}
		// Restart triangle strips
		vboHeightmapIndices.AddData(&iPrimitiveRestartIndex, sizeof(int));
	}

	glGenVertexArrays(1, &uiVAO);
	glBindVertexArray(uiVAO);
	// Attach vertex data to this VAO
	vboHeightmapData.BindVBO();
	vboHeightmapData.UploadDataToGPU(GL_STATIC_DRAW);

	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3)+sizeof(glm::vec2), 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3)+sizeof(glm::vec2), (void*)sizeof(glm::vec3));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3)+sizeof(glm::vec2), (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)));

	// And now attach index data to this VAO
	// Here don't forget to bind another type of VBO - the element array buffer, or simplier indices to vertices
	vboHeightmapIndices.BindVBO(GL_ELEMENT_ARRAY_BUFFER);
	vboHeightmapIndices.UploadDataToGPU(GL_STATIC_DRAW);

	bLoaded = true; // If get here, we succeeded with generating heightmap
	return true;
}

/*-----------------------------------------------
Name:	LoadTerrainShaderProgram
Params:	none
Result: Loads common shader program used for
		rendering heightmaps.
/*---------------------------------------------*/

bool CMultiLayeredHeightmap::LoadTerrainShaderProgram()
{
	bool bOK = true;
	bOK &= shShaders[0].LoadShader("data\\shaders\\terrain.vert", GL_VERTEX_SHADER);
	bOK &= shShaders[1].LoadShader("data\\shaders\\terrain.frag", GL_FRAGMENT_SHADER);
	bOK &= shShaders[2].LoadShader("data\\shaders\\dirLight.frag", GL_FRAGMENT_SHADER);

	spTerrain.CreateProgram();
	FOR(i, NUMTERRAINSHADERS)spTerrain.AddShaderToProgram(&shShaders[i]);
	spTerrain.LinkProgram();

	return bOK;
}

/*-----------------------------------------------
Name:	SetRenderSize
Params:	fRenderX, fHeight, fRenderZ - enter all 3
		dimensions separately		
		OR
		fQuadSize, fHeight - how big should be one quad
		of heightmap and height is just height :)
Result: Sets rendering size (scaling) of heightmap.
/*---------------------------------------------*/

void CMultiLayeredHeightmap::SetRenderSize(float fRenderX, float fHeight, float fRenderZ)
{
	vRenderScale = glm::vec3(fRenderX, fHeight, fRenderZ);
}

void CMultiLayeredHeightmap::SetRenderSize(float fQuadSize, float fHeight)
{
	vRenderScale = glm::vec3(float(iCols)*fQuadSize, fHeight, float(iRows)*fQuadSize);
}

/*-----------------------------------------------
Name:	RenderHeightmap
Params:	none
Result: Guess what it does :)
/*---------------------------------------------*/

void CMultiLayeredHeightmap::RenderHeightmap()
{
	spTerrain.UseProgram();

	spTerrain.SetUniform("fRenderHeight", vRenderScale.y);
	spTerrain.SetUniform("fMaxTextureU", float(iCols)*0.1f);
	spTerrain.SetUniform("fMaxTextureV", float(iRows)*0.1f);

	spTerrain.SetUniform("HeightmapScaleMatrix", GetScaleMatrix());

	// Now we're ready to render - we are drawing set of triangle strips using one call, but we g otta enable primitive restart
	glBindVertexArray(uiVAO);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(iRows*iCols);

	int iNumIndices = (iRows-1)*iCols*2 + iRows-1;
	glDrawElements(GL_TRIANGLE_STRIP, iNumIndices, GL_UNSIGNED_INT, 0);
}

void CMultiLayeredHeightmap::RenderHeightmapForNormals()
{
	// Now we're ready to render - we are drawing set of triangle strips using one call, but we g otta enable primitive restart
	glBindVertexArray(uiVAO);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(iRows*iCols);

	int iNumIndices = (iRows-1)*iCols*2 + iRows-1;
	glDrawElements(GL_POINTS, iNumIndices, GL_UNSIGNED_INT, 0);
}


/*-----------------------------------------------
Name:	ReleaseHeightmap
Params:	none
Result: Releases all data of one heightmap instance.
/*---------------------------------------------*/

void CMultiLayeredHeightmap::ReleaseHeightmap()
{
	if(!bLoaded)
		return; // Heightmap must be loaded
	vboHeightmapData.DeleteVBO();
	vboHeightmapIndices.DeleteVBO();
	glDeleteVertexArrays(1, &uiVAO);
	bLoaded = false;
}

/*-----------------------------------------------
Name:	GetShaderProgram
Params:	none
Result: Returns pointer to shader program ussed for
		rendering heightmaps.
/*---------------------------------------------*/

CShaderProgram* CMultiLayeredHeightmap::GetShaderProgram()
{
	return &spTerrain;
}

/*-----------------------------------------------
Name:	ReleaseTerrainShaderProgramx
Params:	none
Result: Releases a common shader program used for
		rendering heightmaps.
/*---------------------------------------------*/

void CMultiLayeredHeightmap::ReleaseTerrainShaderProgram()
{
	spTerrain.DeleteProgram();
	FOR(i, NUMTERRAINSHADERS)shShaders[i].DeleteShader();
}

/*-----------------------------------------------
Name:	Getters
Params:	none
Result:	They get something :)
/*---------------------------------------------*/

int CMultiLayeredHeightmap::GetNumHeightmapRows()
{
	return iRows;
}

int CMultiLayeredHeightmap::GetNumHeightmapCols()
{
	return iCols;
}

glm::mat4 CMultiLayeredHeightmap::GetScaleMatrix()
{
	return glm::scale(glm::mat4(1.0), glm::vec3(vRenderScale));
}
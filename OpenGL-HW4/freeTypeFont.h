#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include "texture.h"
#include "shaders.h"
#include "vertexBufferObject.h"

/********************************
Class:		CFreeTypeFont

Purpose:	Wraps FreeType fonts and
			their usage with OpenGL.

********************************/

class CFreeTypeFont
{
public:
	bool LoadFont(string sFile, int iPXSize);
	bool LoadSystemFont(string sName, int iPXSize);

	int GetTextWidth(string sText, int iPXSize);

	void Print(string sText, int x, int y, int iPXSize = -1);
	void PrintFormatted(int x, int y, int iPXSize, char* sText, ...);

	void DeleteFont();

	void SetShaderProgram(CShaderProgram* a_shShaderProgram);

	CFreeTypeFont();
private:
	void CreateChar(int iIndex);

	CTexture tCharTextures[256];
	int iAdvX[256], iAdvY[256];
	int iBearingX[256], iBearingY[256];
	int iCharWidth[256], iCharHeight[256];
	int iLoadedPixelSize, iNewLine;

	bool bLoaded;

	UINT uiVAO;
	CVertexBufferObject vboData;

	FT_Library ftLib;
	FT_Face ftFace;
	CShaderProgram* shShaderProgram;
};

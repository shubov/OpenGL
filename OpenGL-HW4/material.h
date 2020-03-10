#pragma once

#include "shaders.h"

/********************************
Class:		CMaterial

Purpose:	Support class for handling materials
			in the scene.

********************************/

class CMaterial
{
public:
	float fSpecularIntensity;
	float fSpecularPower;

	void SetUniformData(CShaderProgram* spProgram, string sMaterialVarName);

	CMaterial();
	CMaterial(float a_fSpecularIntensity, float a_fSpecularPower);
};
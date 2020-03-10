#pragma warning(disable:4996)

#include "common_header.h"

#include "shaders.h"

CShader::CShader()
{
	bLoaded = false;
}

// Loads and compiles shader.
// sFile - path to a file
// a_iType - type of shader (fragment, vertex, geometry)
bool CShader::LoadShader(string sFile, int a_iType)
{
	FILE* fp = fopen(sFile.c_str(), "rt");
	if(!fp)return false;

	// Get all lines from a file

	vector<string> sLines;
	char sLine[255];
	while(fgets(sLine, 255, fp))sLines.push_back(sLine);
	fclose(fp);

	const char** sProgram = new const char*[ESZ(sLines)];
	FOR(i, ESZ(sLines))sProgram[i] = sLines[i].c_str();
	
	uiShader = glCreateShader(a_iType);

	// load and compile
	glShaderSource(uiShader, ESZ(sLines), sProgram, NULL);
	glCompileShader(uiShader);

	delete[] sProgram;

	int iCompilationStatus;
	glGetShaderiv(uiShader, GL_COMPILE_STATUS, &iCompilationStatus);

	if(iCompilationStatus == GL_FALSE)return false;
	iType = a_iType;
	bLoaded = true;

	return 1;
}

// True if shader was loaded and compiled.
bool CShader::IsLoaded()
{
	return bLoaded;
}

// Returns ID of a generated shader.
UINT CShader::GetShaderID()
{
	return uiShader;
}

// Deletes shader and frees memory in GPU.
void CShader::DeleteShader()
{
	if(!IsLoaded())return;
	bLoaded = false;
	glDeleteShader(uiShader);
}

CShaderProgram::CShaderProgram()
{
	bLinked = false;
}

// Creates a new program.
void CShaderProgram::CreateProgram()
{
	uiProgram = glCreateProgram();
}

// Adds a shader (like source file) to a program, but only compiled one.
bool CShaderProgram::AddShaderToProgram(CShader* shShader)
{
	if(!shShader->IsLoaded())return false;

	glAttachShader(uiProgram, shShader->GetShaderID());

	return true;
}

// Performs final linkage of OpenGL program.
bool CShaderProgram::LinkProgram()
{
	glLinkProgram(uiProgram);
	int iLinkStatus;
	glGetProgramiv(uiProgram, GL_LINK_STATUS, &iLinkStatus);
	bLinked = iLinkStatus == GL_TRUE;
	return bLinked;
}

// Deletes program and frees memory on GPU.
void CShaderProgram::DeleteProgram()
{
	if(!bLinked)return;
	bLinked = false;
	glDeleteProgram(uiProgram);
}

// Tells OpenGL to use this program.
void CShaderProgram::UseProgram()
{
	if(bLinked)glUseProgram(uiProgram);
}
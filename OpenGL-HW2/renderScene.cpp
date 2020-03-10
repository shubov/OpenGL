// Шубов Михаил Павлович БПИ164
// OpenGL HW2
// Реализованы: Земля (текстура ночной Земли плавно переходит в текстуру дневной Земли по меридианам),
// Солнце с текстурой, Сатурн с кольцом обладающем прозрачной текстурой.
// На каждой грани куба разные пары текстур перетекают друг в друга с определенной переодичностью.
// Торы, куб и Земля вращаются вокруг своей оси.
// Скайбокс ночного неба без швов.
#include "common_header.h"
#include "win_OpenGLApp.h"
#include "shaders.h"
#include "texture.h"
#include "vertexBufferObject.h"
#include "flyingCamera.h"
#include "skybox.h"
#include "dirLight.h"
#include "static_geometry.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define NUMTEXTURES 11

CTexture tTextures[NUMTEXTURES];
CFlyingCamera cCamera;
CSkybox sbMainSkybox;
CDirectionalLight dlSun;
unsigned int uiVAOs[4]; // Only one VAO now
CVertexBufferObject vboSceneObjects, vboCubeInd, vboCube, vboSphere, vboSphereInd, vboRingInd, vboRing;

void SetUpAttributes() {
	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3) + sizeof(glm::vec2), 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3) + sizeof(glm::vec2), (void*)sizeof(glm::vec3));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3) + sizeof(glm::vec2), (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
}

void SetupSphere(unsigned int& vao) {
	glBindVertexArray(vao);
	vboSphere.CreateVBO();
	vboSphere.BindVBO();
	SolidSphere(vboSphere, 1.0f);
	vboSphere.UploadDataToGPU(GL_STATIC_DRAW);
	vboSphereInd.CreateVBO();
	vboSphereInd.BindVBO(GL_ELEMENT_ARRAY_BUFFER);
	vboSphereInd.AddData(&iSphereindices, rings*sectors * 6 * sizeof(unsigned int));
	vboSphereInd.UploadDataToGPU(GL_STATIC_DRAW);
	SetUpAttributes();
}


// Initializes OpenGL features that will be used.
// lpParam - Pointer to anything you want.
void InitScene(LPVOID lpParam)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	vboSceneObjects.CreateVBO();
	glGenVertexArrays(4, uiVAOs); 

	glBindVertexArray(uiVAOs[0]);
	vboSceneObjects.BindVBO();
	AddSceneObjects(vboSceneObjects);
	vboSceneObjects.UploadDataToGPU(GL_STATIC_DRAW);
	SetUpAttributes();

	//******CUBE
	glBindVertexArray(uiVAOs[1]);
	vboCube.CreateVBO();
	vboCube.BindVBO();
	AddCube(vboCube);
	vboCube.UploadDataToGPU(GL_STATIC_DRAW);
	vboCubeInd.CreateVBO();
	vboCubeInd.BindVBO(GL_ELEMENT_ARRAY_BUFFER);
	vboCubeInd.AddData(&iCubeindices, sizeof(iCubeindices));
	vboCubeInd.UploadDataToGPU(GL_STATIC_DRAW);
	SetUpAttributes();

	//******SPHERE
	SetupSphere(uiVAOs[2]);

	//******Ring
	glBindVertexArray(uiVAOs[3]);
	vboRing.CreateVBO();
	vboRing.BindVBO();
	Ring(vboRing, 1.1f, 1.8f);
	vboRing.UploadDataToGPU(GL_STATIC_DRAW);
	SetUpAttributes();

	if(!PrepareShaderPrograms())
	{
		PostQuitMessage(0);
		return;
	}

	string sTextureNames[] = {
		"grass.png", "met_wall01a.jpg", "tower.jpg", "box.jpg", "ground.jpg", "mars.jpg", "earthd.jpg","earthn.jpg", "sun.jpg", "saturn.jpg", "ring.png"
	};
	FOR(i, NUMTEXTURES)
	{
		tTextures[i].LoadTexture2D("data\\textures\\"+sTextureNames[i], true);
		tTextures[i].SetFiltering(TEXTURE_FILTER_MAG_BILINEAR, TEXTURE_FILTER_MIN_BILINEAR_MIPMAP);
	}

	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	
	cCamera = CFlyingCamera(glm::vec3(0.0f, 10.0f, 120.0f), glm::vec3(0.0f, 10.0f, 119.0f), glm::vec3(0.0f, 1.0f, 0.0f), 25.0f, 0.001f);
	cCamera.SetMovingKeys('W', 'S', 'A', 'D');

	sbMainSkybox.LoadSkybox("data\\skyboxes\\stars\\", "front.png", "back.png", "right.png", "left.png", "top.png", "bottom.png");
	//sbMainSkybox.LoadSkybox("data\\skyboxes\\nature\\", "posz.jpg", "negz.jpg", "posx.jpg", "negx.jpg", "posy.jpg", "negy.jpg");
	dlSun = CDirectionalLight(glm::vec3(1.0f), glm::vec3(sqrt(2.0f) / 2, -sqrt(2.0f) / 2, 0), 1.0f);
}



float fGlobalAngle;
float earthAngle;
float fTextureContribution = 0.5f;
glm::mat4 mModelMatrix, mView;


//***Transform by Translating, Rotating and Scaling
void Transform(glm::vec3 translate = glm::vec3(0.0f), 
	           glm::vec3 scale     = glm::vec3(1.0f),
			   glm::vec3 rotate    = glm::vec3(0.0f), 
			   float angle = fGlobalAngle) {
	mModelMatrix = glm::mat4(1.0f);
	mModelMatrix = glm::translate(mModelMatrix, translate);
	mModelMatrix = glm::scale(mModelMatrix, scale);
	if (rotate != glm::vec3(0.0f)) {
		mModelMatrix = glm::rotate(mModelMatrix, angle, rotate); 
	}
	spMain.SetUniform("matrices.normalMatrix", glm::transpose(glm::inverse(mModelMatrix)));
	spMain.SetUniform("matrices.modelMatrix", mModelMatrix);
}

float textureDelta = -0.2f;

// Renders whole scene.
// lpParam - Pointer to anything you want.
void RenderScene(LPVOID lpParam)
{
	// Typecast lpParam to COpenGLControl pointer
	COpenGLControl* oglControl = (COpenGLControl*)lpParam;

	glm::vec3 vCameraDir = glm::normalize(cCamera.vView - cCamera.vEye);
	mView = cCamera.Look();

	oglControl->ResizeOpenGLViewportFull();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	spMain.UseProgram();
	
	spMain.SetUniform("matrices.projMatrix", oglControl->GetProjectionMatrix());
	spMain.SetUniform("gSamplers[0]", 0);
	spMain.SetUniform("gSamplers[1]", 1);
	spMain.SetUniform("fTextureContributions[0]", 1.0f);
	spMain.SetUniform("fTextureContributions[1]", fTextureContribution);
	spMain.SetUniform("numTextures", 1);
	spMain.SetUniform("matrices.viewMatrix", &mView);
	mModelMatrix = glm::translate(glm::mat4(1.0f), cCamera.vEye);
	spMain.SetUniform("matrices.modelMatrix", mModelMatrix);
	spMain.SetUniform("matrices.normalMatrix", glm::transpose(glm::inverse(mView*mModelMatrix)));

	CDirectionalLight dlSun2 = dlSun;
	dlSun2.fAmbient = 1.0f;
	dlSun2.vColor = glm::vec3(1.0f);
	dlSun2.SetUniformData(&spMain, "sunLight");
	sbMainSkybox.RenderSkybox();

	dlSun.SetUniformData(&spMain, "sunLight");
	spMain.SetUniform("vColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	spMain.SetUniform("matrices.modelMatrix", glm::mat4(1.0f));
	spMain.SetUniform("matrices.normalMatrix", glm::mat4(1.0f));

	glBindVertexArray(uiVAOs[0]);

	//*********************
	int renderPosition = 0;
	int groundSize = 6;
	//*********************  RENDER GROUND
	//tTextures[0].BindTexture();
	//glDrawArrays(GL_TRIANGLES, 0, 6);

	//*********************
	renderPosition += groundSize;
	int torus1Size = iTorusFaces * 3;
	//*********************  RENDER TORUS 1
	tTextures[1].BindTexture();
	Transform(glm::vec3(100.0f, 10.0f, 100.0f), glm::vec3(1.0f), glm::vec3(0, 1.0f, 0));
	glDrawArrays(GL_TRIANGLES, renderPosition, torus1Size);

	//*********************
	renderPosition += torus1Size;
	int torus2Size = iTorusFaces2 * 3;
	//*********************  RENDER TORUS 2
	tTextures[2].BindTexture();
	Transform(glm::vec3(100.0f, 10.0f, 100.0f), glm::vec3(1.0f), glm::vec3(1.0f, 0, 0));
	glDrawArrays(GL_TRIANGLES, renderPosition, torus2Size);

	//*********************
	renderPosition += torus2Size;
	int cilinderSize = iCilinderFaces * 3;
	//*********************  RENDER CILINDER
	//tTextures[2].BindTexture();
	//Transform(glm::vec3(40.0f, 0.0f, 40.0f), glm::vec3(10.0f, 10.0f, 10.0f));
	//glDrawArrays(GL_TRIANGLES, renderPosition, cilinderSize);

	//RENDER CUBE
	glBindVertexArray(uiVAOs[1]);
	spMain.SetUniform("fTextureContributions[0]", 1.0f - fTextureContribution);
	spMain.SetUniform("numTextures", 2);
	glEnable(GL_CULL_FACE);
	Transform(glm::vec3(60.0f, 30.0f, 0.0f), glm::vec3(16.0f), glm::vec3(1.0f));
	for (int i = 0; i < 6; i++) {
		tTextures[i % 3 + 1].BindTexture();
		tTextures[(i+1) % 3 + 1].BindTexture(1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(6 * i * sizeof(unsigned int)));
	}
	glDisable(GL_CULL_FACE);

	//RENDER Earth
	glBindVertexArray(uiVAOs[2]);
	earthAngle += appMain.sof(0.25f);
	float EarthTextureContributions[sectors];
	float delta = 2.0f / (sectors);
	for (int i = 0; i <= sectors / 2; i++) {
		EarthTextureContributions[i] = delta*i;
		EarthTextureContributions[sectors - 1 - i] = delta*i;
	}
	//if (sectors % 2 == 1) { EarthTextureContributions[sectors / 2 + 1] = 0.5; }
	tTextures[6].BindTexture();
	tTextures[7].BindTexture(1);
	Transform(glm::vec3(0.0f), glm::vec3(50.0f), glm::vec3(0, 1.0f, 0), earthAngle);
	for (int i = 0; i < rings*sectors * 6; i += 6) {
		//if (i % (6 * sectors) >= 6 * sectors - 6) continue;
		spMain.SetUniform("fTextureContributions[0]", 1.0f - EarthTextureContributions[i /6 % (sectors)]);
		spMain.SetUniform("fTextureContributions[1]", EarthTextureContributions[i / 6 % (sectors)]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void const*)((i) * sizeof(unsigned int)));
	}
	//restore texture settings 
	spMain.SetUniform("fTextureContributions[0]", 1.0f);
	spMain.SetUniform("numTextures", 1);

	//RENDER Sun
	tTextures[8].BindTexture();
	Transform(glm::vec3(400.0f, -200.0f, 0.0f), glm::vec3(200.0f));
	glDrawElements(GL_TRIANGLES, rings*sectors * 6, GL_UNSIGNED_INT, (void const*)0);

	//RENDER Saturn
	tTextures[9].BindTexture();
	Transform(glm::vec3(-400.0f, 200.0f, 0.0f), glm::vec3(200.0f));
	glDrawElements(GL_TRIANGLES, rings*sectors * 6, GL_UNSIGNED_INT, (void const*)0);

	//RENDER Ring
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(uiVAOs[3]);
	vboRing.BindVBO();
	tTextures[10].BindTexture(); 
	Transform(glm::vec3(-400.0f, 200.0f, 0.0f), glm::vec3(200.0f));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 2 * ringSectors);
	glDisable(GL_BLEND);


	cCamera.Update();

	glEnable(GL_DEPTH_TEST);

	//if (Keys::Key('Q')) fTextureContribution += appMain.sof(-0.2f);
	//if (Keys::Key('E')) fTextureContribution += appMain.sof(0.2f);
	//fTextureContribution = min(max(0.0f, fTextureContribution), 1.0f);
	
	fTextureContribution += appMain.sof(textureDelta);
	fTextureContribution = min(max(0.0f, fTextureContribution), 1.0f);
	if (fTextureContribution == 0.0f) textureDelta = 0.2;
	if (fTextureContribution == 1.0f) textureDelta = -0.2;

	if(Keys::Onekey(VK_ESCAPE))PostQuitMessage(0);
	fGlobalAngle += appMain.sof(1.0f);
	oglControl->SwapBuffers();
}

// Releases OpenGL scene.
// lpParam - Pointer to anything you want.
void ReleaseScene(LPVOID lpParam)
{
	FOR(i, NUMTEXTURES)tTextures[i].DeleteTexture();
	sbMainSkybox.DeleteSkybox();
	spMain.DeleteProgram();
	FOR(i, NUMSHADERS)shShaders[i].DeleteShader();

	glDeleteVertexArrays(4, uiVAOs);
	vboSceneObjects.DeleteVBO();
	vboSphereInd.DeleteVBO();
	vboSphere.DeleteVBO();
	vboCubeInd.DeleteVBO();
	vboCube.DeleteVBO();
}
// ����� ������ �������� ���164
// ��������� �� ��� �������� � ��������� ����� � ����������.
// ������ ����������� � ������� ������������ ��������� ������������� �����(����/����)
// ����� ���������� ����������, ���� �����������
// ��������� ��������� �� ����� ������� ������� ������
// ��������� ��������� ����� ���������� �������
// ������������ ����� ���������� �� 0 �� 1 �� �����
// Q - toggle Sun
// E - toggle Spotlight
// R - toggle Spotlight(camera)
// T - toggle Spotlight(blue)
// Y - toggle Spotlight(red)
#include "common_header.h"
#include "win_OpenGLApp.h"
#include "shaders.h"
#include "texture.h"
#include "vertexBufferObject.h"
#include "flyingCamera.h"
#include "spotLight.h"
#include "dirLight.h"
#include "pointLight.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define NUMTEXTURES 8

/* One VBO, where all static data are stored now,
in this tutorial vertex is stored as 3 floats for
position, 2 floats for texture coordinate and
3 floats for normal vector. */

CVertexBufferObject vboSceneObjects, vboSphere, vboSphereInd;
UINT uiVAOs[2]; // Only one VAO now

CTexture tTextures[NUMTEXTURES];
CFlyingCamera cCamera;
CDirectionalLight dlSun;
CSpotLight slFlashLight[2];
CPointLight plLight[2];

float sunRotationAngle = 90;
float sunRotationSpeed = 0.1;
float fGlobalAngle;
const float PI = float(atan(1.0)*4.0);
glm::mat4 mModelMatrix, mView;

#include "static_geometry.h"

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

// Initializes OpenGL features that will be used.
// lpParam - Pointer to anything you want.
void InitScene(LPVOID lpParam)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Prepare all scene objects
	glGenVertexArrays(2, uiVAOs); // Create one VAO
	glBindVertexArray(uiVAOs[0]);

	vboSceneObjects.CreateVBO();
	vboSceneObjects.BindVBO();
	AddSceneObjects(vboSceneObjects);
	vboSceneObjects.UploadDataToGPU(GL_STATIC_DRAW);
	SetUpAttributes();

	glBindVertexArray(uiVAOs[1]);
	vboSphere.CreateVBO();
	vboSphere.BindVBO();
	SolidSphere(vboSphere, 1.0f);
	vboSphere.UploadDataToGPU(GL_STATIC_DRAW);
	vboSphereInd.CreateVBO();
	vboSphereInd.BindVBO(GL_ELEMENT_ARRAY_BUFFER);
	vboSphereInd.AddData(&iSphereindices, rings*sectors * 6 * sizeof(unsigned int));
	vboSphereInd.UploadDataToGPU(GL_STATIC_DRAW);
	SetUpAttributes();

	if(!PrepareShaderPrograms()) {
		PostQuitMessage(0);
		return;
	}

	// Load textures
	string sTextureNames[] = {"grass.png", "met_wall01a.jpg", "tower.jpg", "box.jpg", "ground.jpg", "red.jpg", "blue.jpg", "lamp.jpg"};
	FOR(i, NUMTEXTURES) {
		tTextures[i].LoadTexture2D("data\\textures\\"+sTextureNames[i], true);
		tTextures[i].SetFiltering(TEXTURE_FILTER_MAG_BILINEAR, TEXTURE_FILTER_MIN_BILINEAR_MIPMAP);
	}

	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	
	cCamera = CFlyingCamera(glm::vec3(0.0f, 10.0f, 120.0f), glm::vec3(0.0f, 10.0f, 119.0f), glm::vec3(0.0f, 1.0f, 0.0f), 25.0f, 0.001f);
	cCamera.SetMovingKeys('W', 'S', 'A', 'D');

	dlSun = CDirectionalLight(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(sqrt(2.0f) / 2, -sqrt(2.0f) / 2, 0), 0.2f);
	// Creating spotlight, position and direction will get updated every frame, that's why zero vectors
	slFlashLight[0] = CSpotLight(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 1, 15.0f, 0.017f);
	slFlashLight[1] = CSpotLight(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(100.0f, 15.0f, 0.0f), glm::vec3(0.0f), 1, 25.0f, 0.05f);
	plLight[0] = CPointLight(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(100.0f, 10.0f, 100.0f), 0.5f, 0.3f, 0.007f, 0.00008f);
	plLight[1] = CPointLight(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 10.0f, 0.0f), 0.15f, 0.7f, 0.07f, 0.007f);
}

void Transform(glm::vec3 translate = glm::vec3(0.0f),
	glm::vec3 scale = glm::vec3(1.0f),
	glm::vec3 rotate = glm::vec3(0.0f),
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

void TransformR(glm::vec3 translate = glm::vec3(0.0f),
	glm::vec3 scale = glm::vec3(1.0f),
	glm::vec3 rotate = glm::vec3(0.0f),
	float angle = fGlobalAngle) {
	mModelMatrix = glm::mat4(1.0f);
	if (rotate != glm::vec3(0.0f)) {
		mModelMatrix = glm::rotate(mModelMatrix, angle, rotate);
	}
	mModelMatrix = glm::translate(mModelMatrix, translate);
	mModelMatrix = glm::scale(mModelMatrix, scale);
	spMain.SetUniform("matrices.normalMatrix", glm::transpose(glm::inverse(mModelMatrix)));
	spMain.SetUniform("matrices.modelMatrix", mModelMatrix);
}

// Renders whole scene.
// lpParam - Pointer to anything you want.
void RenderScene(LPVOID lpParam)
{
	// Typecast lpParam to COpenGLControl pointer
	COpenGLControl* oglControl = (COpenGLControl*)lpParam;
	oglControl->ResizeOpenGLViewportFull();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	spMain.UseProgram();

	// Set spotlight parameters
	glm::vec3 vSpotLightPos = cCamera.vEye;
	glm::vec3 vCameraDir = glm::normalize(cCamera.vView - cCamera.vEye);
	vSpotLightPos.y -= 3.2f;
	glm::vec3 vSpotLightDir = (vSpotLightPos + vCameraDir*75.0f) - vSpotLightPos;
	vSpotLightDir = glm::normalize(vSpotLightDir);
	glm::vec3 vHorVector = glm::cross(cCamera.vView - cCamera.vEye, cCamera.vUp);
	vSpotLightPos += vHorVector*3.3f;
	slFlashLight[0].vPosition = vSpotLightPos;
	slFlashLight[0].vDirection = vSpotLightDir;
	slFlashLight[0].SetUniformData(&spMain, "spotLight");
	slFlashLight[1].SetUniformData(&spMain, "spotLight2");

	plLight[0].SetUniformData(&spMain, "pointLight");
	plLight[1].SetUniformData(&spMain, "pointLight2");
	dlSun.SetUniformData(&spMain, "sunLight");

	mView = cCamera.Look();
	mModelMatrix = glm::translate(glm::mat4(1.0f), cCamera.vEye);
	spMain.SetUniform("matrices.viewMatrix", &mView);
	spMain.SetUniform("matrices.modelMatrix", &mModelMatrix);
	spMain.SetUniform("matrices.normalMatrix", glm::transpose(glm::inverse(mView*mModelMatrix)));
	spMain.SetUniform("matrices.projMatrix", oglControl->GetProjectionMatrix());
	spMain.SetUniform("gSampler", 0);
	spMain.SetUniform("matrices.modelMatrix", glm::mat4(1.0f));
	spMain.SetUniform("matrices.normalMatrix", glm::mat4(1.0f));

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	glBindVertexArray(uiVAOs[0]);

	// Render Ground
	tTextures[0].BindTexture();
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// Render Spheres
	glBindVertexArray(uiVAOs[1]);
	tTextures[5].BindTexture();
	Transform(glm::vec3(0.0f, 10.0f, 0.0f));
	glDrawElements(GL_TRIANGLES, rings*sectors * 6, GL_UNSIGNED_INT, (void const*)0);
	tTextures[6].BindTexture();
	Transform(glm::vec3(100.0f, 10.0f, 100.0f));
	glDrawElements(GL_TRIANGLES, rings*sectors * 6, GL_UNSIGNED_INT, (void const*)0);
	tTextures[7].BindTexture();
	Transform(glm::vec3(100.0f, 15.0f, 0.0f));
	glDrawElements(GL_TRIANGLES, rings*sectors * 6, GL_UNSIGNED_INT, (void const*)0);

	glBindVertexArray(uiVAOs[0]);
	// Render Toruses
	tTextures[1].BindTexture();
	Transform(glm::vec3(0, 10.0f, 0), glm::vec3(1.0f), glm::vec3(0, 1.0f, 0));
	glDrawArrays(GL_TRIANGLES, 42, iTorusFaces * 3);
	tTextures[2].BindTexture();
	Transform(glm::vec3(0, 10.0f, 0), glm::vec3(1.0f), glm::vec3(1.0f, 0, 0));
	glDrawArrays(GL_TRIANGLES, 42 + iTorusFaces * 3, iTorusFaces2 * 3);

	//// Render Cilinder
	tTextures[2].BindTexture();
	Transform(glm::vec3(100.0f, 0.0f, 0.0f), glm::vec3(1.0f));
	glDrawArrays(GL_TRIANGLES, 42 + (iTorusFaces + iTorusFaces2) * 3, iCilinderFaces*3);

	// Render Cubes
	tTextures[3].BindTexture();
	FOR(j, 2) FOR(i, 16) {
		spMain.SetUniform("vColor", glm::vec4(1.0f, 1.0f, 1.0f, 1 - i / 16.0f));
		TransformR(glm::vec3(30.0f, 4.0f + 8.0f*j, 0.0f), glm::vec3(8.0f), glm::vec3(0.0f, 1.0f, 0.0f), PI / 8 * i + PI / 16 * j);
		glDrawArrays(GL_TRIANGLES, 6, 36);
	}
	spMain.SetUniform("vColor", glm::vec4(1.0f));

	cCamera.Update();

	if (Keys::Onekey('Q')) {
		sunRotationSpeed = sunRotationSpeed > 0 ? 0.0f : 0.1f;
		sunRotationAngle = 30;
	}
	sunRotationAngle += sunRotationSpeed;
	float angleRad = sunRotationAngle * PI / 180;
	float radius = 1;
	dlSun.vDirection = glm::vec3(radius * cos(angleRad), radius * sin(angleRad), 1.0f);
	if (glm::abs(dlSun.vDirection.y) < 0.05f) {
		slFlashLight[0].bOn = 1 - slFlashLight[0].bOn;
		slFlashLight[1].bOn = 1 - slFlashLight[1].bOn;
	}
	slFlashLight[1].vDirection = glm::vec3(radius * cos(angleRad),-0.8f, radius * sin(angleRad));

	if (Keys::Onekey('E')) slFlashLight[0].bOn = 1 - slFlashLight[0].bOn;
	if (Keys::Onekey('R')) slFlashLight[1].bOn = 1 - slFlashLight[1].bOn;
	if (Keys::Onekey('T')) plLight[0].vColor = plLight[0].vColor == glm::vec3(0) ? glm::vec3(0, 0, 1.0f) : glm::vec3(0);
	if (Keys::Onekey('Y')) plLight[1].vColor = plLight[1].vColor == glm::vec3(0) ? glm::vec3(1.0f, 0, 0) : glm::vec3(0);

	glEnable(GL_DEPTH_TEST);
	if(Keys::Onekey(VK_ESCAPE))PostQuitMessage(0);
	fGlobalAngle += appMain.sof(1.0f);
	oglControl->SwapBuffers();
}

// Releases OpenGL scene.
// lpParam - Pointer to anything you want.
void ReleaseScene(LPVOID lpParam) {
	FOR(i, NUMTEXTURES)tTextures[i].DeleteTexture();
	FOR(i, NUMSHADERS)shShaders[i].DeleteShader();
	glDeleteVertexArrays(2, uiVAOs);
	vboSceneObjects.DeleteVBO();
	vboSphere.DeleteVBO();
	vboSphereInd.DeleteVBO();
	spMain.DeleteProgram();
}
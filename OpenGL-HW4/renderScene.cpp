// Mikhail Shubov BSE164
// Models of objects (tree, tower, Earth, plant) are loaded using Assimp
// Every parameter of the particles generator can be changed by Q(-) and E(+) buttons
// To change a particular parameter a Control Mode needs to be set (0-9,Z,X,C,V)
// Rotation Speed can be changed by buttons
#include "common_header.h"
#include "win_OpenGLApp.h"
#include "shaders.h"
#include "texture.h"
#include "vertexBufferObject.h"
#include "flyingCamera.h"
#include "freeTypeFont.h"
#include "skybox.h"
#include "dirLight.h"
#include "material.h"
#include "assimp_model.h"
#include "heightmap.h"
#include "static_geometry.h"
#include "particle_system_tf.h"

CVertexBufferObject vboSceneObjects;
UINT uiVAOSceneObjects;
CFreeTypeFont ftFont;
CSkybox sbMainSkybox;
CFlyingCamera cCamera;
CDirectionalLight dlSun;
CMaterial matShiny;
CAssimpModel amModels[6];
CMultiLayeredHeightmap hmWorld;
CParticleSystemTransformFeedback psMainParticleSystem;
int iTorusFaces;
bool bDisplayNormals = false; // Do not display normals by default
float rotationSpeed = 0.5f;

char controlMode;
// Particle params
float minVerticalVelocity = 40.0f;
float minHorizontalVelocity = -5.0f;
float maxVerticalVelocity = 80.0f;
float maxHorizontalVelocity = 5.0f;
float gravity = -9.8f;
float Red = 0.8f;
float Green = 0.6f;
float Blue = 0.6f;
float MinLifeTime = 6.0f;
float MaxLifeTime = 8.0f;
float RenderedSize = 0.75f;
float SpawnPeriod = 0.01f;
int Quantity = 30;

/*-----------------------------------------------
Name:    InitScene

Params:  lpParam - Pointer to anything you want.

Result:  Initializes OpenGL features that will
         be used.

/*---------------------------------------------*/

void InitScene(LPVOID lpParam)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	if(!PrepareShaderPrograms())
	{
		PostQuitMessage(0);
		return;
	}
	
	LoadAllTextures();

	vboSceneObjects.CreateVBO();
	glGenVertexArrays(1, &uiVAOSceneObjects); // Create one VAO
	glBindVertexArray(uiVAOSceneObjects);

	vboSceneObjects.BindVBO();

	iTorusFaces = GenerateTorus(vboSceneObjects, 7.0f, 2.0f, 20, 20);
	vboSceneObjects.UploadDataToGPU(GL_STATIC_DRAW);

	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3)+sizeof(glm::vec2), 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3)+sizeof(glm::vec2), (void*)sizeof(glm::vec3));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3)+sizeof(glm::vec2), (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)));


	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);

	// Here we load font with pixel size 32 - this means that if we print with size above 32, the quality will be low
	ftFont.LoadSystemFont("arial.ttf", 32);
	ftFont.SetShaderProgram(&spFont2D);
	
	cCamera = CFlyingCamera(glm::vec3(0.0f, 30.0f, 100.0f), glm::vec3(0.0f, 30.0f, 99.0f), glm::vec3(0.0f, 1.0f, 0.0f), 25.0f, 0.001f);
	cCamera.SetMovingKeys('W', 'S', 'A', 'D');

	sbMainSkybox.LoadSkybox("data\\skyboxes\\bluefreeze\\", "bluefreeze_front.jpg", "bluefreeze_back.jpg", "bluefreeze_right.jpg", "bluefreeze_left.jpg", "bluefreeze_top.jpg", "bluefreeze_top.jpg");

	dlSun = CDirectionalLight(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(sqrt(2.0f)/2, -sqrt(2.0f)/2, 0), 0.5f, 0);

	amModels[0].LoadModelFromFile("data\\models\\house\\house.3ds");
	amModels[1].LoadModelFromFile("data\\models\\treasure_chest_obj\\treasure_chest.obj");
	amModels[2].LoadModelFromFile("data\\models\\earth\\earth.3ds");
	amModels[3].LoadModelFromFile("data\\models\\tree\\tree.3ds");
	amModels[4].LoadModelFromFile("data\\models\\tower\\tower.obj");
	amModels[5].LoadModelFromFile("data\\models\\plant\\plant.obj");
	

	CAssimpModel::FinalizeVBO();
	CMultiLayeredHeightmap::LoadTerrainShaderProgram();
	hmWorld.LoadHeightMapFromImage("data\\worlds\\world_like_in_21th.bmp");

	matShiny = CMaterial(1.0f, 32.0f);
	psMainParticleSystem.InitalizeParticleSystem();
}

/*-----------------------------------------------

Name:    RenderScene

Params:  lpParam - Pointer to anything you want.

Result:  Renders whole scene.

/*---------------------------------------------*/

void RenderScene(LPVOID lpParam)
{
	// Typecast lpParam to COpenGLControl pointer
	COpenGLControl* oglControl = (COpenGLControl*)lpParam;
	oglControl->ResizeOpenGLViewportFull();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	spMain.UseProgram();

	spMain.SetUniform("matrices.projMatrix", oglControl->GetProjectionMatrix());
	spMain.SetUniform("matrices.viewMatrix", cCamera.Look());

	spMain.SetUniform("gSampler", 0);

	spMain.SetUniform("matrices.modelMatrix", glm::mat4(1.0));
	spMain.SetUniform("matrices.normalMatrix", glm::mat4(1.0));
	spMain.SetUniform("vColor", glm::vec4(1, 1, 1, 1));

	psMainParticleSystem.SetGeneratorProperties(
		glm::vec3(-10.0f, 17.5f, 0.0f), // Where the particles are generated
		glm::vec3(-minHorizontalVelocity, minVerticalVelocity, -minHorizontalVelocity), // Minimal velocity
		glm::vec3(-maxHorizontalVelocity, maxVerticalVelocity, -maxHorizontalVelocity), // Maximal velocity
		glm::vec3(0, gravity, 0), // Gravity force applied to particles
		glm::vec3(Red, Green, Blue), // Color
		MinLifeTime, // Minimum lifetime in seconds
		MaxLifeTime, // Maximum lifetime in seconds
		RenderedSize, // Rendered size
		SpawnPeriod, // Spawn every 0.05 seconds
		Quantity); // And spawn 30 particles

	// This values will set the darkness of whole scene, that's why such name of variable :D
	static float fAngleOfDarkness = 45.0f;
	// You can play with direction of light with '+' and '-' key
	if(Keys::Key(VK_ADD))fAngleOfDarkness += appMain.sof(90);
	if(Keys::Key(VK_SUBTRACT))fAngleOfDarkness -= appMain.sof(90);
	// Set the directional vector of light
	dlSun.vDirection = glm::vec3(-sin(fAngleOfDarkness*3.1415f/180.0f), -cos(fAngleOfDarkness*3.1415f/180.0f), 0.0f);
	
	dlSun.iSkybox = 1;
	dlSun.SetUniformData(&spMain, "sunLight");

	spMain.SetUniform("matrices.modelMatrix", glm::translate(glm::mat4(1.0), cCamera.vEye));
	sbMainSkybox.RenderSkybox();

	dlSun.iSkybox = 0;
	dlSun.SetUniformData(&spMain, "sunLight");

	spMain.SetUniform("matrices.modelMatrix", glm::mat4(1.0));

	spMain.SetUniform("vEyePosition", cCamera.vEye);
	matShiny.SetUniformData(&spMain, "matActive");
	
	CAssimpModel::BindModelsVAO();

	// Render a house
	glm::mat4 mModel = glm::translate(glm::mat4(1.0), glm::vec3(40.0f, 17.0f, 0));
	mModel = glm::scale(mModel, glm::vec3(8.0f));
	spMain.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
	amModels[0].RenderModel();

	// ... and a treasure chest
	mModel = glm::translate(glm::mat4(1.0), glm::vec3(-10.0f, 17.5f, 0));
	mModel = glm::scale(mModel, glm::vec3(0.5f, 0.5f, 0.5f));
	spMain.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
	amModels[1].RenderModel();

	// EARTH
	mModel = glm::translate(glm::mat4(1.0f), glm::vec3(100.0f, 37.5f, 100.0f));
	mModel = glm::scale(mModel, glm::vec3(0.1f));
	spMain.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
	amModels[2].RenderModel();

	// TREE
	mModel = glm::translate(glm::mat4(1.0), glm::vec3(100.0f, 17.5f, 0));
	mModel = glm::scale(mModel, glm::vec3(0.03f));
	spMain.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
	amModels[3].RenderModel();

	// TOWER
	mModel = glm::translate(glm::mat4(1.0), glm::vec3(130.0f, 10.0f, 40.0f));
	mModel = glm::scale(mModel, glm::vec3(7.0f));
	spMain.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
	amModels[4].RenderModel();

	// PLANT
	mModel = glm::translate(glm::mat4(1.0), glm::vec3(10.0f, 17.5f, 0));
	spMain.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
	amModels[5].RenderModel();

	// Render 3 rotated tori to create interesting object
	tTextures[5].BindTexture();
	glBindVertexArray(uiVAOSceneObjects);
	static float fGlobalAngle = 0.0f;

	FOR(i, 2)
	{
		glm::vec3 vCenter = glm::vec3(-40+i*40, 30, -20);
		mModel = glm::translate(glm::mat4(1.0), vCenter);
		if(i == 0)mModel = glm::rotate(mModel, fGlobalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		spMain.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
		glDrawArrays(GL_TRIANGLES, 0, iTorusFaces*3);

		mModel = glm::translate(glm::mat4(1.0), vCenter+glm::vec3(0.01f, 0.0f, 0.0f));
		if(i == 0)mModel = glm::rotate(mModel, fGlobalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		mModel = glm::rotate(mModel, 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		spMain.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
		glDrawArrays(GL_TRIANGLES, 0, iTorusFaces*3);

		mModel = glm::translate(glm::mat4(1.0), vCenter+glm::vec3(0.00f, 0.01f, 0.0f));

		if(i == 0)mModel = glm::rotate(mModel, fGlobalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		mModel = glm::rotate(mModel, 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		spMain.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
		glDrawArrays(GL_TRIANGLES, 0, iTorusFaces*3);
	}

	fGlobalAngle += appMain.sof(rotationSpeed);

	// Now we're going to render terrain

	hmWorld.SetRenderSize(300.0f, 35.0f, 300.0f);
	CShaderProgram* spTerrain = CMultiLayeredHeightmap::GetShaderProgram();

	spTerrain->UseProgram();

	spTerrain->SetUniform("matrices.projMatrix", oglControl->GetProjectionMatrix());
	spTerrain->SetUniform("matrices.viewMatrix", cCamera.Look());

	spTerrain->SetUniform("vEyePosition", cCamera.vEye);
	matShiny.SetUniformData(spTerrain, "matActive");

	// We bind all 5 textures - 3 of them are textures for layers, 1 texture is a "path" texture, and last one is
	// the places in heightmap where path should be and how intense should it be
	FOR(i, 5)
	{
		char sSamplerName[256];
		sprintf_s(sSamplerName, "gSampler[%d]", i);//_s
		tTextures[i].BindTexture(i);
		spTerrain->SetUniform(sSamplerName, i);
	}

	// ... set some uniforms
	spTerrain->SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", glm::mat4(1.0));
	spTerrain->SetUniform("vColor", glm::vec4(1, 1, 1, 1));

	dlSun.SetUniformData(spTerrain, "sunLight");

	// ... and finally render heightmap
	hmWorld.RenderHeightmap();

	if(bDisplayNormals)
	{
		spNormalDisplayer.UseProgram();
		spNormalDisplayer.SetUniform("fNormalLength", 1.0f);
		spNormalDisplayer.SetUniform("matrices.projMatrix", oglControl->GetProjectionMatrix());
		spNormalDisplayer.SetUniform("matrices.viewMatrix", cCamera.Look());

		CAssimpModel::BindModelsVAO();

		// ... Render the house again

		glm::mat4 mModel = glm::translate(glm::mat4(1.0), glm::vec3(40.0f, 17.0f, 0));
		mModel = glm::scale(mModel, glm::vec3(8));

		spNormalDisplayer.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
		amModels[0].RenderModel(GL_POINTS);

		// ... and the treasure chest again

		mModel = glm::translate(glm::mat4(1.0), glm::vec3(-10.0f, 17.5f, 0));
		mModel = glm::scale(mModel, glm::vec3(0.5f, 0.5f, 0.5f));

		spNormalDisplayer.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
		amModels[1].RenderModel(GL_POINTS);

		glBindVertexArray(uiVAOSceneObjects);

		FOR(i, 2)
		{
			glm::vec3 vCenter = glm::vec3(-40+i*40, 30, -20);
			mModel = glm::translate(glm::mat4(1.0), vCenter);
			if(i == 0)mModel = glm::rotate(mModel, fGlobalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
			spNormalDisplayer.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
			glDrawArrays(GL_POINTS, 0, iTorusFaces*3);

			mModel = glm::translate(glm::mat4(1.0), vCenter+glm::vec3(0.01f, 0.0f, 0.0f));
			if(i == 0)mModel = glm::rotate(mModel, fGlobalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
			mModel = glm::rotate(mModel, 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
			spNormalDisplayer.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
			glDrawArrays(GL_POINTS, 0, iTorusFaces*3);

			mModel = glm::translate(glm::mat4(1.0), vCenter+glm::vec3(0.00f, 0.01f, 0.0f));

			if(i == 0)mModel = glm::rotate(mModel, fGlobalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
			mModel = glm::rotate(mModel, 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
			spNormalDisplayer.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
			glDrawArrays(GL_POINTS, 0, iTorusFaces*3);
		}

		spNormalDisplayer.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", hmWorld.GetScaleMatrix());
		hmWorld.RenderHeightmapForNormals();
	}

	tTextures[6].BindTexture(); 

	psMainParticleSystem.SetMatrices(oglControl->GetProjectionMatrix(), cCamera.vEye, cCamera.vView, cCamera.vUp);

	psMainParticleSystem.UpdateParticles(appMain.sof(1.0f));
	psMainParticleSystem.RenderParticles();

	cCamera.Update();

	// Print something over scene
	
	spFont2D.UseProgram();
	glDisable(GL_DEPTH_TEST);
	spFont2D.SetUniform("matrices.projMatrix", oglControl->GetOrthoMatrix());

	int w = oglControl->GetViewportWidth(), 
		h = oglControl->GetViewportHeight();
	spFont2D.SetUniform("vColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	ftFont.Print("OpenGL_4 2020 Mikhail Shubov", 20, 20, 24);
	ftFont.PrintFormatted(20, h - 30 * 1, 20, "FPS: %d", oglControl->GetFPS());
	ftFont.PrintFormatted(20, h - 30 * 2, 20, "Particles: %d", psMainParticleSystem.GetNumParticles());
	ftFont.PrintFormatted(20, h - 30 * 3, 20, "ControlMode %c (Press 'Q' and 'E' to change)", controlMode);
	ftFont.PrintFormatted(20, h - 30 * 4, 20, "0 MinVerticalVelocity: %.2f", minVerticalVelocity);
	ftFont.PrintFormatted(20, h - 30 * 5, 20, "1 MinHorizontalVelocity: %.2f", minHorizontalVelocity);
	ftFont.PrintFormatted(20, h - 30 * 6, 20, "2 MaxVerticalVelocity: %.2f", maxVerticalVelocity);
	ftFont.PrintFormatted(20, h - 30 * 7, 20, "3 MaxHorizontalVelocity: %.2f", maxHorizontalVelocity);
	ftFont.PrintFormatted(20, h - 30 * 8, 20, "4 Gravity: %.2f", gravity);
	ftFont.PrintFormatted(20, h - 30 * 9, 20, "5 Red: %.2f", Red);
	ftFont.PrintFormatted(20, h - 30 * 10, 20, "6 Gren: %.2f", Green);
	ftFont.PrintFormatted(20, h - 30 * 11, 20, "7 Blue: %.2f", Blue);
	ftFont.PrintFormatted(20, h - 30 * 12, 20, "8 MinLifeTime: %.2f", MinLifeTime);
	ftFont.PrintFormatted(20, h - 30 * 13, 20, "9 MaxLifeTime: %.2f", MaxLifeTime);
	ftFont.PrintFormatted(20, h - 30 * 14, 20, "Z RenderedSize: %.2f", RenderedSize);
	ftFont.PrintFormatted(20, h - 30 * 15, 20, "X SpawnPeriod: %.2f", SpawnPeriod);
	ftFont.PrintFormatted(20, h - 30 * 16, 20, "C Quantity: %d", Quantity);
	ftFont.PrintFormatted(20, h - 30 * 17, 20, "V Rotation Speed: %.2f", rotationSpeed);
	ftFont.PrintFormatted(20, h - 30 * 18, 20, "Displaying Normals: %s (Press 'N' to toggle)", bDisplayNormals ? "Yes" : "Nope");
	
	if (Keys::Key('0')) controlMode = '0';
	if (Keys::Key('1')) controlMode = '1';
	if (Keys::Key('2')) controlMode = '2';
	if (Keys::Key('3')) controlMode = '3';
	if (Keys::Key('4')) controlMode = '4';
	if (Keys::Key('5')) controlMode = '5';
	if (Keys::Key('6')) controlMode = '6';
	if (Keys::Key('7')) controlMode = '7';
	if (Keys::Key('8')) controlMode = '8';
	if (Keys::Key('9')) controlMode = '9';
	if (Keys::Key('Z')) controlMode = 'Z';
	if (Keys::Key('X')) controlMode = 'X';
	if (Keys::Key('C')) controlMode = 'C';
	if (Keys::Key('V')) controlMode = 'V';
	if (Keys::Onekey('N')) bDisplayNormals = !bDisplayNormals;
	if (Keys::Key('Q'))
	{
		switch (controlMode)
		{
		case '0':
			minVerticalVelocity     -= appMain.sof(10.0f);
			break;
		case '1':
			minHorizontalVelocity   -= appMain.sof(5.0f);
			break;
		case '2':
			maxVerticalVelocity     -= appMain.sof(10.0f);
			break;
		case '3':
			maxHorizontalVelocity   -= appMain.sof(5.0f);
			break;
		case '4':
			gravity                 -= appMain.sof(1.0f);
			break;
		case '5':
			Red = glm::clamp(Red - appMain.sof(0.1f), 0.0f, 1.0f);
			break;
		case '6':
			Green = glm::clamp(Green - appMain.sof(0.1f), 0.0f, 1.0f);
			break;
		case '7':
			Blue = glm::clamp(Blue - appMain.sof(0.1f), 0.0f, 1.0f);
			break;
		case '8':
			MinLifeTime             -= appMain.sof(1.0f);
			break;
		case '9':
			MaxLifeTime             -= appMain.sof(1.0f);
			break;
		case 'Z':
			RenderedSize            -= appMain.sof(0.05f);
			break;
		case 'X':
			SpawnPeriod             -= appMain.sof(0.01f);
			break;
		case 'C':
			Quantity                -= 1;
			break;
		case 'V':
			rotationSpeed -= appMain.sof(0.5f);
			break;
		default:
			break;
		}
	}
	if (Keys::Key('E')) 
	{
		switch (controlMode)
		{
		case '0':
			minVerticalVelocity		+= appMain.sof(10.0f);
			break;
		case '1':
			minHorizontalVelocity	+= appMain.sof(5.0f);
			break;
		case '2':
			maxVerticalVelocity		+= appMain.sof(10.0f);
			break;
		case '3':
			maxHorizontalVelocity	+= appMain.sof(5.0f);
			break;
		case '4':
			gravity					+= appMain.sof(1.0f);
			break;
		case '5':
			Red						= glm::clamp(Red + appMain.sof(0.1f), 0.0f, 1.0f);
			break;
		case '6':
			Green					= glm::clamp(Green + appMain.sof(0.1f), 0.0f, 1.0f);
			break;
		case '7':
			Blue					= glm::clamp(Blue + appMain.sof(0.1f), 0.0f, 1.0f);
			break;
		case '8':
			MinLifeTime				+= appMain.sof(1.0f);
			break;
		case '9':
			MaxLifeTime				+= appMain.sof(1.0f);
			break;
		case 'Z':
			RenderedSize			+= appMain.sof(0.05f);
			break;
		case 'X':
			SpawnPeriod				+= appMain.sof(0.01f);
			break;
		case 'C':
			Quantity				+= 1;
			break;
		case 'V':
			rotationSpeed += appMain.sof(0.5f); 
			break;
		default:
			break;
		}
	}
	
	//toggle mouse rotation
	glEnable(GL_DEPTH_TEST);	
	if (Keys::Onekey(VK_ESCAPE))PostQuitMessage(0);

	oglControl->SwapBuffers();
}

/*-----------------------------------------------

Name:    ReleaseScene

Params:  lpParam - Pointer to anything you want.

Result:  Releases OpenGL scene.

/*---------------------------------------------*/

void ReleaseScene(LPVOID lpParam)
{
	FOR(i, NUMTEXTURES)tTextures[i].DeleteTexture();
	sbMainSkybox.DeleteSkybox();
	spMain.DeleteProgram();
	spOrtho2D.DeleteProgram();
	spFont2D.DeleteProgram();
	FOR(i, NUMSHADERS)shShaders[i].DeleteShader();
	ftFont.DeleteFont();

	glDeleteVertexArrays(1, &uiVAOSceneObjects);
	vboSceneObjects.DeleteVBO();

	hmWorld.ReleaseHeightmap();
	CMultiLayeredHeightmap::ReleaseTerrainShaderProgram();
}
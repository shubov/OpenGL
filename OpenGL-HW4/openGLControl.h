#pragma once

#define SIMPLE_OPENGL_CLASS_NAME "simple_openGL_class_name"

/********************************
Class:	COpenGLControl

Purpose: Provides convenient usage
		 of OpenGL.

********************************/

class COpenGLControl
{
public:
	bool InitOpenGL(HINSTANCE hInstance, HWND* a_hWnd, int iMajorVersion, int iMinorVersion, void (*a_ptrInitScene)(LPVOID), void (*a_ptrRenderScene)(LPVOID), void(*a_ptrReleaseScene)(LPVOID), LPVOID lpParam);
	
	void ResizeOpenGLViewportFull();
	void SetProjection3D(float fFOV, float fAspectRatio, float fNear, float fFar);
	void SetOrtho2D(int width, int height);

	glm::mat4* GetProjectionMatrix();
	glm::mat4* GetOrthoMatrix();

	void Render(LPVOID lpParam);
	void ReleaseOpenGLControl(LPVOID lpParam);

	static void RegisterSimpleOpenGLClass(HINSTANCE hInstance);
	static void UnregisterSimpleOpenGLClass(HINSTANCE hInstance);

	void MakeCurrent();
	void SwapBuffers();

	bool SetVerticalSynchronization(bool bEnabled);

	int GetFPS();

	int GetViewportWidth();
	int GetViewportHeight();

	COpenGLControl();

private:
	bool InitGLEW(HINSTANCE hInstance);

	HDC hDC;
	HWND* hWnd;
	HGLRC hRC;
	static bool bClassRegistered;
	static bool bGlewInitialized;
	int iMajorVersion, iMinorVersion;

	// Used for FPS calculation
	int iFPSCount, iCurrentFPS;
	clock_t tLastSecond;

	// Matrix for perspective projection
	glm::mat4 mProjection;
	// Matrix for orthographic 2D projection
	glm::mat4 mOrtho;

	// Viewport parameters
	int iViewportWidth, iViewportHeight;

	void (*ptrInitScene)(LPVOID lpParam), (*ptrRenderScene)(LPVOID lpParam), (*ptrReleaseScene)(LPVOID lpParam);
};

LRESULT CALLBACK msgHandlerSimpleOpenGLClass(HWND, UINT, WPARAM, LPARAM);
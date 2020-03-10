#pragma once

#define SIMPLE_OPENGL_CLASS_NAME "simple_openGL_class_name"

// Provides convenient usage of OpenGL.
class COpenGLControl
{
public:
	bool InitOpenGL(HINSTANCE hInstance, HWND* a_hWnd, void (*a_ptrInitScene)(LPVOID), void (*a_ptrRenderScene)(LPVOID), void(*a_ptrReleaseScene)(LPVOID), LPVOID lpParam);
	
	void ResizeOpenGLViewportFull();
	void SetProjection3D(float fFOV, float fAspectRatio, float fNear, float fFar);

	glm::mat4* GetProjectionMatrix();

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

	// Used for FPS calculation
	int iFPSCount, iCurrentFPS;
	clock_t tLastSecond;

	// Matrix for perspective projection
	glm::mat4 mProjection;

	// Viewport parameters
	int iViewportWidth, iViewportHeight;

	void (*ptrInitScene)(LPVOID lpParam), (*ptrRenderScene)(LPVOID lpParam), (*ptrReleaseScene)(LPVOID lpParam);
};

LRESULT CALLBACK msgHandlerSimpleOpenGLClass(HWND, UINT, WPARAM, LPARAM);
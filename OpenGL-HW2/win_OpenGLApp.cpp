#include "common_header.h"

#include "win_OpenGLApp.h"

COpenGLWinApp appMain;

char Keys::kp[256] = {0};

// Return true if Key is pressed.
// iKey - virtual Key code
int Keys::Key(int iKey)
{
	return (GetAsyncKeyState(iKey)>>15)&1;
}

// Return true if Key was pressed, but only once(the Key must be released).
// iKey - virtual Key code
int Keys::Onekey(int iKey)
{
	if(Key(iKey) && !kp[iKey]){kp[iKey] = 1; return 1;}
	if(!Key(iKey))kp[iKey] = 0;
	return 0;
}

// Resets application timer (for example after re-activation of application).
void COpenGLWinApp::ResetTimer()
{
	tLastFrame = clock();
	fFrameInterval = 0.0f;
}


// Updates application timer.
void COpenGLWinApp::UpdateTimer()
{
	clock_t tCur = clock();
	fFrameInterval = float(tCur-tLastFrame)/float(CLOCKS_PER_SEC);
	tLastFrame = tCur;
}

// Stands for speed optimized float.
float COpenGLWinApp::sof(float fVal)
{
	return fVal*fFrameInterval;
}

// Initializes app with specified (unique) application identifier.
bool COpenGLWinApp::InitializeApp(string a_sAppName)
{
	sAppName = a_sAppName;
	hMutex = CreateMutex(NULL, 1, sAppName.c_str());
	if(GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBox(NULL, "This application already runs!", "Multiple Instances Found.", MB_ICONINFORMATION | MB_OK);
		return 0;
	}
	return 1;
}

// Registers application window class.
// a_hInstance - application instance handle
LRESULT CALLBACK GlobalMessageHandler(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	return appMain.msgHandlerMain(hWnd, uiMsg, wParam, lParam);
}

void COpenGLWinApp::RegisterAppClass(HINSTANCE a_hInstance)
{
	WNDCLASSEX wcex;
	memset(&wcex, 0, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_OWNDC;

	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

	wcex.hIcon = LoadIcon(hInstance, IDI_WINLOGO);
	wcex.hIconSm = LoadIcon(hInstance, IDI_WINLOGO);
	wcex.hCursor = LoadCursor(hInstance, IDC_ARROW);
	wcex.hInstance = a_hInstance;
	hInstance = a_hInstance;

	wcex.lpfnWndProc = GlobalMessageHandler;
	wcex.lpszClassName = sAppName.c_str();

	wcex.lpszMenuName = NULL;

	RegisterClassEx(&wcex);
}

// Creates main application window.
// sTitle - title of created window
bool COpenGLWinApp::CreateAppWindow(string sTitle)
{
	if(MessageBox(NULL, "Would you like to run in fullscreen?", "Fullscreen", MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
		DEVMODE dmSettings = {0};
		EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmSettings); // Get current display settings

		hWnd = CreateWindowEx(0, sAppName.c_str(), sTitle.c_str(), WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, // This is the commonly used style for fullscreen
		0, 0, dmSettings.dmPelsWidth, dmSettings.dmPelsHeight, NULL,
		NULL, hInstance, NULL);
	}
	else hWnd = CreateWindowEx(0, sAppName.c_str(), sTitle.c_str(), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		0, 0, CW_USEDEFAULT, CW_USEDEFAULT, NULL,
		NULL, hInstance, NULL);

	if(!oglControl.InitOpenGL(hInstance, &hWnd, InitScene, RenderScene, ReleaseScene, &oglControl))return false;

	ShowWindow(hWnd, SW_SHOW);

	// Just to send WM_SIZE message again
	ShowWindow(hWnd, SW_SHOWMAXIMIZED);
	UpdateWindow(hWnd);

	ShowCursor(FALSE);

	return true;
}

// Main application body infinite loop.
void COpenGLWinApp::AppBody()
{
	MSG msg;
	while(1)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)break; // If the message was WM_QUIT then exit application
			else
			{
				TranslateMessage(&msg); // Otherwise send message to appropriate window
				DispatchMessage(&msg);
			}
		}
		else if(bAppActive)
		{
			UpdateTimer();
			oglControl.Render(&oglControl);
		}
		else Sleep(200); // Do not consume processor power if application isn't active
	}
}

// Shuts down application and releases used memory.
void COpenGLWinApp::Shutdown()
{
	oglControl.ReleaseOpenGLControl(&oglControl);

	DestroyWindow(hWnd);
	UnregisterClass(sAppName.c_str(), hInstance);
	COpenGLControl::UnregisterSimpleOpenGLClass(hInstance);
	ReleaseMutex(hMutex);
}

// Application messages handler.
LRESULT CALLBACK COpenGLWinApp::msgHandlerMain(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;

	switch(uiMsg)
	{
		case WM_PAINT:
			BeginPaint(hWnd, &ps);					
			EndPaint(hWnd, &ps);
			break;

		case WM_CLOSE:
			PostQuitMessage(0);
			break;

		case WM_ACTIVATE:
		{
			switch(LOWORD(wParam))
			{
				case WA_ACTIVE:
				case WA_CLICKACTIVE:
					bAppActive = true;
					ResetTimer();
					break;
				case WA_INACTIVE:
					bAppActive = false;
					break;
			}
			break;
		}

		case WM_SIZE:
			oglControl.ResizeOpenGLViewportFull();
			oglControl.SetProjection3D(45.0f, float(LOWORD(lParam))/float(HIWORD(lParam)), 0.5f, 1000.0f);
			break;

		default:
			return DefWindowProc(hWnd, uiMsg, wParam, lParam);
	}
	return 0;
}

// Returns application instance.
HINSTANCE COpenGLWinApp::GetInstance()
{
	return hInstance;
}

// Application entry point.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR sCmdLine, int iShow)
{
	if(!appMain.InitializeApp("Семинар 8.  2018-2019"))
		return 0;
	appMain.RegisterAppClass(hInstance);

	if(!appMain.CreateAppWindow("Семинар 8. 2018-2019. Индексное рисование, Skybox, Мультитекстурирование"))
		return 0;
	appMain.ResetTimer();

	appMain.AppBody();
	appMain.Shutdown();

	return 0;
}


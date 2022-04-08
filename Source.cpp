#include <Windows.h>
#include <iostream>
#include <GL/gl.h>

#pragma comment(lib, "opengl32.lib")

#define MAX_NAME_LENGTH 256
#define HInstance() GetModuleHandle(NULL)

#define MOUSE_LBUTTON 0
#define MOUSE_RBUTTON 1
#define MOUSE_WHEEL 3

#define BLOCK_SIZE 1.0f
#define PLAYER_HEIGHT 4.0f

#define OUTPUT_WIDTH 50
#define OUTPUT_HEIGHT 50

struct MouseState
{
	bool bPressed;
	bool bReleased;
	bool bDblClick;

	float fWheelDelta;
};

struct KeyState
{
	bool bPressed;
	bool bReleased;
};

struct vf3d
{
	float x;
	float y;
	float z;
};

struct sCamera
{
	vf3d pos;
	vf3d rot;

	float speed;
	float vel;

	bool jumpActive;
};

sCamera vCamera = {
	{ 5.0f, 5.0f, 20.0f }, 
	{ 70.0f, 0.0f, -40.0f },
	0.0f, 0.0f, false
};

const double PI = 2.0 * acos(0.0);

int nScreenWidth = 1024;
int nScreenHeight = 768;

float fMouseX = 0.0f;
float fMouseY = 0.0f;

bool bShowCursor = true;

MouseState mouse[5];
KeyState keys[256];

float fPerlinNoise2D[OUTPUT_WIDTH * OUTPUT_HEIGHT];
float fPerlinSeed2D[OUTPUT_WIDTH * OUTPUT_HEIGHT];

float fScaleBias = 2.0f;
float fOctaves = 12;

vf3d vMap[1024 * 768];

LRESULT CALLBACK WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	POINT cursor;
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		DestroyWindow(hWnd);
		return 0;

	case WM_MOUSEMOVE:
		GetCursorPos(&cursor);
		fMouseX = cursor.x;
		fMouseY = cursor.y;
		ShowCursor(bShowCursor);
		return 0;

	case WM_LBUTTONDOWN:
		mouse[0].bPressed = true;
		mouse[0].bReleased = false;
		return 0;

	case WM_LBUTTONUP:
		mouse[0].bPressed = false;
		mouse[0].bReleased = true;
		return 0;

	case WM_RBUTTONDOWN:
		mouse[1].bPressed = true;
		mouse[1].bReleased = false;
		return 0;

	case WM_RBUTTONUP:
		mouse[1].bPressed = false;
		mouse[1].bReleased = true;
		return 0;

	case WM_MOUSEWHEEL:
		mouse[3].fWheelDelta = HIWORD(wParam);
		return 0;

	case WM_KEYDOWN:
		keys[wParam].bPressed = true;
		keys[wParam].bReleased = false;
		return 0;

	case WM_KEYUP:
		keys[wParam].bPressed = false;
		keys[wParam].bReleased = true;
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void RotateCamera(HWND* hWnd)
{
	auto rotate = [&](float xAngle, float zAngle)
	{
		vCamera.rot.x += xAngle;
		vCamera.rot.z += zAngle;

		if (vCamera.rot.x < 0.0f) vCamera.rot.x = 0.0f;
		if (vCamera.rot.x > 180.0f) vCamera.rot.z = 180.0f;

		if (vCamera.rot.z < 0.0f) vCamera.rot.z += 360.0f;
		if (vCamera.rot.z > 360.0f) vCamera.rot.z -= 360.0f;		
	};

	if (GetForegroundWindow() != *hWnd) return;

	float fAngle = -vCamera.rot.z / 180.0f * PI;
	float fSpeed = 0.0f;

	if (keys[L'W'].bPressed) fSpeed = 0.1f + vCamera.speed;
	if (keys[L'S'].bPressed) fSpeed = -0.1f;
	if (keys[L'A'].bPressed) { fSpeed = 0.1f; fAngle -= PI / 2.0f; }
	if (keys[L'D'].bPressed) { fSpeed = 0.1f; fAngle += PI / 2.0f; }
	
	if (keys[VK_UP].bPressed) vCamera.pos.z += 0.1f;
	if (keys[VK_DOWN].bPressed) vCamera.pos.z -= 0.1f;

	if (fSpeed != 0.0f)
	{
		vCamera.pos.x += sinf(fAngle) * fSpeed;
		vCamera.pos.y += cosf(fAngle) * fSpeed;
	}

	static POINT base = { nScreenWidth / 2, nScreenHeight / 2 };
	rotate((base.y - fMouseY) / 5.0f, (base.x - fMouseX) / 5.0f);
	SetCursorPos(base.x, base.y);
}

void DrawCube(float x, float y, float z)
{
	glTranslatef(x, y, z);

	glBegin(GL_TRIANGLES);

	glColor3f(0.5f, 0.5f, 0.5f);

	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 1);
	glVertex3f(1, 0, 0);

	glVertex3f(0, 0, 1);
	glVertex3f(1, 0, 1);
	glVertex3f(1, 0, 0);

	glColor3f(0.1f, 0.5f, 0.5f);

	glVertex3f(0, 1, 0);
	glVertex3f(0, 1, 1);
	glVertex3f(1, 1, 0);

	glVertex3f(0, 1, 1);
	glVertex3f(1, 1, 1);
	glVertex3f(1, 1, 0);

	glColor3f(0.1f, 0.2f, 0.5f);

	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 1);
	glVertex3f(0, 1, 0);

	glVertex3f(0, 1, 0);
	glVertex3f(0, 1, 1);
	glVertex3f(0, 0, 1);

	glColor3f(0.3f, 0.2f, 0.1f);

	glVertex3f(1, 0, 0);
	glVertex3f(1, 0, 1);
	glVertex3f(1, 1, 0);

	glVertex3f(1, 1, 0);
	glVertex3f(1, 1, 1);
	glVertex3f(1, 0, 1);

	glColor3f(0.5f, 0.7f, 0.6f);

	glVertex3f(0, 0, 1);
	glVertex3f(0, 1, 1);
	glVertex3f(1, 0, 1);

	glVertex3f(1, 0, 1);
	glVertex3f(1, 1, 1);
	glVertex3f(0, 1, 1);

	glColor3f(0.75f, 0.1f, 0.3f);
	
	glVertex3f(0, 0, 0);
	glVertex3f(0, 1, 0);
	glVertex3f(1, 0, 0);

	glVertex3f(1, 0, 0);
	glVertex3f(1, 1, 0);
	glVertex3f(0, 1, 0);

	glEnd();

	glTranslatef(-x, -y, -z);
}

void DoPerlinNoise2D(int nWidth, int nHeight, float* fSeed, float fOctaves, float fBias, float* fOutput)
{
	for (int x = 0; x < nWidth; x++)
		for (int y = 0; y < nHeight; y++)
		{
			float fNoise = 0.0f;
			float fScale = 1.0f;
			float fScaleAccumulator = 0.0f;

			for (int o = 0; o < (int)fOctaves; o++)
			{
				int nPitch = nWidth >> o;

				if (nPitch != 0) {
					int nSampleX1 = (x / nPitch) * nPitch;
					int nSampleY1 = (y / nPitch) * nPitch;

					int nSampleX2 = (nSampleX1 + nPitch) % nWidth;
					int nSampleY2 = (nSampleY1 + nPitch) % nWidth;

					float fBlendX = (float)(x - nSampleX1) / (float)nPitch;
					float fBlendY = (float)(y - nSampleY1) / (float)nPitch;

					float fSampleT = (1.0f - fBlendX) * fSeed[nSampleY1 * nWidth + nSampleX1] + fBlendX * fSeed[nSampleY1 * nWidth + nSampleX2];
					float fSampleB = (1.0f - fBlendX) * fSeed[nSampleY2 * nWidth + nSampleX1] + fBlendX * fSeed[nSampleY2 * nWidth + nSampleX2];

					fNoise += (fBlendY * (fSampleB - fSampleT) + fSampleT) * fScale;
					fScaleAccumulator += fScale;
					fScale /= fBias;
				}
			}

			fOutput[y * nWidth + x] = fNoise / fScaleAccumulator;
		}
}

bool OnUserCreate()
{
	srand(time(NULL));

	for (int i = 0; i < OUTPUT_WIDTH * OUTPUT_HEIGHT; i++)
		fPerlinSeed2D[i] = (float)rand() / (float)RAND_MAX;

	bShowCursor = false;

	DoPerlinNoise2D(OUTPUT_WIDTH, OUTPUT_HEIGHT, fPerlinSeed2D, fOctaves, fScaleBias, fPerlinNoise2D);

	for (int i = 0; i < OUTPUT_WIDTH * OUTPUT_HEIGHT; i++)
		fPerlinNoise2D[i] = (10.0f - fPerlinNoise2D[i] * 16.0f) * 5.0f;

	return true;
}

bool OnUserUpdate(HWND* hWnd)
{
	glRotatef(-vCamera.rot.x, 1, 0, 0);
	glRotatef(-vCamera.rot.z, 0, 0, 1);
	glTranslatef(-vCamera.pos.x, -vCamera.pos.y, -vCamera.pos.z);

	RotateCamera(hWnd);

	DoPerlinNoise2D(OUTPUT_WIDTH, OUTPUT_HEIGHT, fPerlinSeed2D, fOctaves, fScaleBias, fPerlinNoise2D);

	if (vCamera.vel > 0.2f)
		vCamera.vel = 0.2f;
	
	if (!vCamera.jumpActive)
		vCamera.pos.z -= vCamera.vel;

	for (int x = 0; x < OUTPUT_WIDTH; x++)
		for (int y = 0; y < OUTPUT_HEIGHT; y++)
		{
			float z = fPerlinNoise2D[y * OUTPUT_WIDTH + x];

			if (x <= vCamera.pos.x && y <= vCamera.pos.y && vCamera.pos.x <= x + BLOCK_SIZE && vCamera.pos.y <= y + BLOCK_SIZE)
			{
				if (z + PLAYER_HEIGHT > vCamera.pos.z && vCamera.pos.z - PLAYER_HEIGHT / 2.0f > z && !vCamera.jumpActive)
				{
					vCamera.pos.z = z + PLAYER_HEIGHT;
					vCamera.vel = 0.0f;
				}

				if (vCamera.jumpActive)
				{
					vCamera.vel -= 0.07f;

					if (vCamera.vel < 0.0f)
						vCamera.jumpActive = false;
				}
			}

			DrawCube(x, y, z);
		}

	if (!vCamera.jumpActive)
		vCamera.vel += 0.03f;

	if (keys[VK_SPACE].bPressed)
		vCamera.jumpActive = true;

	vCamera.speed = keys[VK_SHIFT].bPressed ? 0.2f : 0.0f;
	
	return true;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
	PIXELFORMATDESCRIPTOR pfd;

	/* get the device context (DC) */
	*hDC = GetDC(hwnd);

	/* set the pixel format for the DC */
	ZeroMemory(&pfd, sizeof(pfd));

	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;

	SetPixelFormat(*hDC, ChoosePixelFormat(*hDC, &pfd), &pfd);

	/* create and enable the render context (RC) */
	*hRC = wglCreateContext(*hDC);

	wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hwnd, hDC);
}

int main(HINSTANCE, HINSTANCE, LPSTR, INT)
{
	std::wstring sTitle = L"Perlin Noise 3D";

	bool bFullScreen = false;

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;

	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);

	wc.hIcon = LoadIcon(HInstance(), nullptr);

	wc.lpszClassName = sTitle.c_str();
	wc.lpszMenuName = nullptr;

	wc.hInstance = HInstance();

	wc.lpfnWndProc = WinProc;

	RegisterClass(&wc);

	DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD dwStyle = WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_THICKFRAME;

	HDC hDC;
	HGLRC hRC;

	float fTopLeftX = CW_USEDEFAULT;
	float fTopLeftY = CW_USEDEFAULT;

	HWND hWnd = nullptr;

	if (bFullScreen)
	{
		dwExStyle = 0;
		dwStyle = WS_VISIBLE | WS_POPUP;

		HMONITOR hmon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };

		if (!GetMonitorInfo(hmon, &mi)) return 1;

		nScreenWidth = mi.rcMonitor.right;
		nScreenHeight = mi.rcMonitor.bottom;

		fTopLeftX = 0.0f;
		fTopLeftY = 0.0f;
	}

	hWnd = CreateWindowEx(dwExStyle, sTitle.c_str(), sTitle.c_str(), dwStyle,
		fTopLeftX, fTopLeftY, nScreenWidth, nScreenHeight, nullptr, nullptr, HInstance(), nullptr);

	if (!hWnd)
	{
		MessageBox(0, L"Failed To Create Window!", 0, 0);
		return false;
	}

	ShowWindow(hWnd, SW_SHOW);

	EnableOpenGL(hWnd, &hDC, &hRC);

	glEnable(GL_DEPTH_TEST);

	glFrustum(-1, 1, -1, 1, 2, 80);

	if (!OnUserCreate())
		return true;

	for (int i = 0; i < 5; i++)
		mouse[0] = { false, false, true, 0 };

	for (int i = 0; i < 256; i++)
		keys[i] = { false, true };

	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glPushMatrix();

			SetWindowText(hWnd, LPWSTR(sTitle.c_str()));

			if (!OnUserUpdate(&hWnd))
				return 0;

			glPopMatrix();

			SwapBuffers(hDC);

			Sleep(1);
		}
	}

	return 0;
}

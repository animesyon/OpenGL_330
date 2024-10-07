//    
//    Draw a Square
//     
#include "framework.h"
#include "square.h"
#include "matrix.h"
#include "shader.h"
#include "model.h"

const int MAX_LOADSTRING = 100;

// variable
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // main window class name

CMatrix matrix;
CShader shader;
CModel model;

// function
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

void OnSize(HWND hWnd, int width, int height);

void OnPaint(HDC hDC);
void OnCreate(HWND hWnd, HDC* hDC);
void OnDestroy(HWND hWnd, HDC hDC);

// main program
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SQUARE, szWindowClass, MAX_LOADSTRING);

    // registers the window class
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SQUARE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    RegisterClassExW(&wcex);

    // creates window
    int X, Y, nWidth, nHeight, Cx, Cy;

    Cx = 1280;
    Cy = 720;

    nWidth = Cx + 16;
    nHeight = Cy + 58;

    X = (GetSystemMetrics(SM_CXSCREEN) - nWidth) / 2;
    Y = (GetSystemMetrics(SM_CYSCREEN) - nHeight) / 4;

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        X, Y, nWidth, nHeight, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
        return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;

    // main message loop
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

// processes messages for the main window
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC hDC;

    switch (message)
    {
    case WM_SIZE:           OnSize(hWnd, LOWORD(lParam), HIWORD(lParam));                    break;
    case WM_PAINT:          OnPaint(hDC);                                                               break;
    case WM_CREATE:         OnCreate(hWnd, &hDC);                                                       break;
    case WM_DESTROY:        OnDestroy(hWnd, hDC);                                                       break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// set screen coordinate
void OnSize(HWND hWnd, int width, int height)
{
    if (width == 0) width = 1;
    if (height == 0) height = 1;

    float left, right, bottom, top, znear, zfar;
    float w, h;

    w = 100.0f;
    h = w * ((float)height / (float)width);

    left = -w / 2.0f;
    right = left + w;
    bottom = -h / 2.0f;
    top = bottom + h;
    znear = 0.0f;
    zfar = 1.0f;

    matrix.Orthographic(left, right, bottom, top, znear, zfar);

    glViewport(0, 0, width, height);
}

// render onto a screen
void OnPaint(HDC hDC)
{
    glClear(GL_COLOR_BUFFER_BIT);
    model.Draw(matrix);
    SwapBuffers(hDC);
}

// initialize opengl
void OnCreate(HWND hWnd, HDC* hDC)
{
    PIXELFORMATDESCRIPTOR pfd;
    int format;
    HGLRC hglrc;

    // create a pixel format
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);  // size ng data structure
    pfd.nVersion = 1;                           // version number
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |          // support window
        PFD_SUPPORT_OPENGL |                    // support OpenGL
        PFD_DOUBLEBUFFER |                      // double buffered
        PFD_GENERIC_ACCELERATED;                // support device driver that accelerates the generic implementation
    pfd.iPixelType = PFD_TYPE_RGBA;             // RGBA pixels
    pfd.cColorBits = 24;                        //  24-bit color
    pfd.cRedBits = 0;                           // color bits ignored
    pfd.cGreenBits = 0;
    pfd.cBlueBits = 0;
    pfd.cAlphaBits = 0;
    pfd.cRedShift = 0;                          // shift bit ignored
    pfd.cGreenShift = 0;
    pfd.cBlueShift = 0;
    pfd.cAlphaShift = 0;
    pfd.cAccumBits = 0;                         // no accumulation buffer
    pfd.cAccumRedBits = 0;
    pfd.cAccumGreenBits = 0;
    pfd.cAccumBlueBits = 0;
    pfd.cAccumAlphaBits = 0;
    pfd.cDepthBits = 32;                        // 32-bit z-buffer
    pfd.cStencilBits = 0;                       // no stencil buffer
    pfd.cAuxBuffers = 0;                        // no auxiliary buffer
    pfd.iLayerType = PFD_MAIN_PLANE;            // main layer
    pfd.bReserved = 0;
    pfd.dwLayerMask = 0;
    pfd.dwVisibleMask = 0;
    pfd.dwDamageMask = 0;

    *hDC = GetDC(hWnd);                         // get the device context for our window
    format = ChoosePixelFormat(*hDC, &pfd);     // get the best available match of pixel format for the device context
    SetPixelFormat(*hDC, format, &pfd);         // make that the pixel format of the device context
    hglrc = wglCreateContext(*hDC);             // create an OpenGL rendering context
    wglMakeCurrent(*hDC, hglrc);                // make it the current rendering context

    // load OpenGL functions
    LoadOpenGLFunctions();

    // get shader path name
    wchar_t pathname[MAX_PATH], filename1[MAX_PATH], filename2[MAX_PATH];

    GetCurrentDirectory(MAX_PATH, pathname);

    wcscpy_s(filename1, MAX_PATH, pathname);
    wcscat_s(filename1, MAX_PATH, L"\\shader\\vertex.txt");

    wcscpy_s(filename2, MAX_PATH, pathname);
    wcscat_s(filename2, MAX_PATH, L"\\shader\\fragment.txt");

    OutputDebugString(L"\n");
    OutputDebugString(filename1); OutputDebugString(L"\n");
    OutputDebugString(filename2); OutputDebugString(L"\n");
    OutputDebugString(L"\n");

    // create objects
    shader.Create(filename1, filename2);
    shader.Use();
    model.Create(shader.Get());

    // clear window to black
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // opengl version info
    char str[100];
    OutputDebugStringA("-----------------------------------------------------------------------------\n");
    sprintf_s(str, 100, "OpenGL Version :%s\n", glGetString(GL_VERSION));                   OutputDebugStringA(str);
    sprintf_s(str, 100, "GLES Version   :%s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));  OutputDebugStringA(str);
    sprintf_s(str, 100, "GLU Version    :%s\n", glGetString(GLU_VERSION));                  OutputDebugStringA(str);
    sprintf_s(str, 100, "Vendor         :%s\n", glGetString(GL_VENDOR));                    OutputDebugStringA(str);
    sprintf_s(str, 100, "Renderer       :%s\n", glGetString(GL_RENDERER));                  OutputDebugStringA(str);
    OutputDebugStringA("-----------------------------------------------------------------------------\n");
}

//
void OnDestroy(HWND hWnd, HDC hDC)
{
    // release ojects
    shader.Destroy();
    model.Destroy();

    HGLRC hglrc;

    wglMakeCurrent(hDC, NULL);      // get current OpenGL rendering context
    hglrc = wglGetCurrentContext(); // make the rendering context not current
    wglDeleteContext(hglrc);        // delete the rendering context
    ReleaseDC(hWnd, hDC);           // releases a device context

    PostQuitMessage(0);             // close the program
}

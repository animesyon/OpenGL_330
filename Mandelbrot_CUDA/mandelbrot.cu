
// Mandelbrot
// CUDA OpenGL Interoperability
//
// This program requires nVidia GPU and CUDA Toollkit

#include "framework.h"
#include "matrix.h"
#include "shader.h"
#include "square1.h"
#include "square2.h"
#include "stack.h"

const UINT_PTR IDM_OPEN = 111;
const UINT_PTR IDM_SAVE_AS = 112;
const UINT_PTR IDM_EXIT = 113;

const int MAX_ITERATION_COUNT = 32768;

const int PARAM_1_COUNT = 4;
const int PARAM_2_COUNT = 3;
const int PARAM_3_COUNT = 3 * MAX_ITERATION_COUNT;

const unsigned long long PARAM_1_SIZE = PARAM_1_COUNT * sizeof(double);
const unsigned long long PARAM_2_SIZE = PARAM_2_COUNT * sizeof(int);
const unsigned long long PARAM_3_SIZE = PARAM_3_COUNT * sizeof(unsigned char);

const UINT_PTR IDM_FRACTAL = 121;
const UINT_PTR IDM_GO_BACK = 122;

const int MAX_LOADSTRING = 100;

const int FRACTAL_WIDTH = 1280;
const int FRACTAL_HEIGHT = 720;

// Global Variables:
HINSTANCE hInst;                                // current instance
CHAR szTitle[MAX_LOADSTRING];                  // The title bar text
CHAR szWindowClass[MAX_LOADSTRING];            // main window class name
HMENU hMenu;

CMatrix matrix;
CShader shader1, shader2;
CSquare1 square1;
CSquare2 square2;
CStack stack;
GLuint textures;
double* Param1;
int* Param2;
unsigned char* Param3;
int height, px, py, px1, py1, px2, py2;
bool is_dragging, show_selection;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK    WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void OnAfterWindowDisplayed(HWND hWnd, WPARAM wParam, LPARAM lParam);

HMENU CreateAppMenu(HWND hWnd);
void DestroyAppMenu(HMENU hMenu);

void GetUniqueName(wchar_t* filename, DWORD size);
void CreateColorGradient(unsigned char* gradient, int count);
__global__ void DoFractal(double* param1, int* param2, unsigned char* param3, cudaSurfaceObject_t object);
void DoParallelComputing(double* Param1, size_t size1, int* Param2, size_t size2, unsigned char* Param3, size_t size3);

void OnLButtonDown(HWND hWnd, WORD Key, int x, int y);
void OnLButtonUp(HWND hWnd, WORD Key, int x, int y);
void OnMouseMove(HWND hWnd, WORD Key, int x, int y);
void OnSize(HWND hWnd, int width, int height);

void OnPaint(HDC hDC);
void OnCreate(HWND hWnd, HDC* hDC);
void OnDestroy(HWND hWnd, HDC hDC);

void OnFileOpen(HWND hWnd);
void OnFileSaveAs(HWND hWnd);
void OnFileExit(HWND hWnd);

void OnViewFractal(HWND hWnd);
void OnViewGoBack(HWND hWnd);

int main()
{
    HINSTANCE hInstance;
    WNDCLASSEXA wcex;
    HWND hWnd;
    MSG msg;
    int X, Y, nWidth, nHeight, Cx, Cy;

    strcpy_s(szTitle, "Mandelbrot");
    strcpy_s(szWindowClass, "MandelbrotClass");

    hInst = hInstance = GetModuleHandle(NULL);

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;

    if (!RegisterClassExA(&wcex)) return 0;

    X = 200;
    Y = 100;

    Cx = FRACTAL_WIDTH;
    Cy = FRACTAL_HEIGHT;

    nWidth = Cx + 16;
    nHeight = Cy + 59;

    hWnd = CreateWindowExA(NULL,
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        X, Y,
        nWidth, nHeight,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (!hWnd) return 0;

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

// Processes messages for the main window.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC hDC;

    switch (message)
    {
    case WM_AFTERWINDOWDISPLAYED: OnAfterWindowDisplayed(hWnd, wParam, lParam);  break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_OPEN:	            OnFileOpen(hWnd);		                    break;
        case IDM_SAVE_AS:	        OnFileSaveAs(hWnd);		                    break;
        case IDM_EXIT:	            OnFileExit(hWnd);		                    break;
        case IDM_FRACTAL:           OnViewFractal(hWnd);        break;
        case IDM_GO_BACK:           OnViewGoBack(hWnd);         break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_LBUTTONDOWN:    OnLButtonDown(hWnd, WORD(wParam), LOWORD(lParam), HIWORD(lParam));  break;
    case WM_LBUTTONUP:      OnLButtonUp(hWnd, WORD(wParam), LOWORD(lParam), HIWORD(lParam));    break;
    case WM_MOUSEMOVE:      OnMouseMove(hWnd, WORD(wParam), LOWORD(lParam), HIWORD(lParam));    break;
    case WM_SIZE:           OnSize(hWnd, LOWORD(lParam), HIWORD(lParam));                    break;
    case WM_PAINT:          OnPaint(hDC);                                                       break;
    case WM_CREATE:         OnCreate(hWnd, &hDC);                                               break;
    case WM_DESTROY:        OnDestroy(hWnd, hDC);                                               break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void OnAfterWindowDisplayed(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    DoParallelComputing(Param1, PARAM_1_SIZE, Param2, PARAM_2_SIZE, Param3, PARAM_3_SIZE);
}

// Adding Lines and Graphs to a Menu
// https://learn.microsoft.com/en-us/windows/win32/menurc/using-menus
HMENU CreateAppMenu(HWND hWnd)
{
    HMENU hMenu, hFile, hView;

    hMenu = CreateMenu();
    hFile = CreatePopupMenu();
    hView = CreatePopupMenu();

    AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hFile, "File");
    AppendMenu(hFile, MF_STRING, IDM_OPEN, "Open...");
    AppendMenu(hFile, MF_STRING, IDM_SAVE_AS, "Save As...");
    AppendMenu(hFile, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFile, MF_STRING, IDM_EXIT, "Exit");

    AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hView, "View");
    AppendMenu(hView, MF_STRING, IDM_FRACTAL, "Fractal");
    AppendMenu(hView, MF_STRING, IDM_GO_BACK, "Go Back");

    SetMenu(hWnd, hMenu);

    return hMenu;
}

void DestroyAppMenu(HMENU hMenu)
{
    DestroyMenu(hMenu);
}

void GetUniqueName(wchar_t* filename, DWORD size)
{
    time_t ltime;
    struct tm a;

    time(&ltime);
    _localtime64_s(&a, &ltime);

    swprintf_s(filename, size, L"%d%02d%02d%02d%02d%02d.mdl", a.tm_year + 1900, a.tm_mon + 1, a.tm_mday, a.tm_hour, a.tm_min, a.tm_sec);
}

// gradient is a series of rgb value
void CreateColorGradient(unsigned char* gradient, int count)
{
    int i, k, quo, c1, c2, c3;

    k = 0;

    for (i = 0; i < count; i++) {

        quo = i / 32;

        c3 = i % 32;
        c2 = quo % 32;
        c1 = quo / 32;

        gradient[k++] = (unsigned char)(255.0 * ((double)c1 / 31.0));
        gradient[k++] = (unsigned char)(255.0 * ((double)c2 / 31.0));
        gradient[k++] = (unsigned char)(255.0 * ((double)c3 / 31.0));
    }
}

//  param1[0] - cx
//  param1[1] - cy
//  param1[2] - ox
//  param1[3] - oy
//
//  param2[0] - FRACTAL_WIDTH
//  param2[1] - FRACTAL_HEIGHT
//  param2[2] - MAX_ITERATION_COUNT
//
//  param3[0] - color gradient

__global__ void DoFractal(double* param1, int* param2, unsigned char* param3, cudaSurfaceObject_t object)
{
    unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

    int i, j, k;
    double ax, ay, bx, by, rx, ry;
    unsigned char r, g, b;

    if (y < param2[1]) {

        bx = ((double)x / (double)(param2[0] - 1)) * param1[0] + param1[2];
        by = ((double)y / (double)(param2[1] - 1)) * param1[1] + param1[3];

        ax = 0.0;
        ay = 0.0;

        j = 0;

        for (i = 0; i < param2[2]; i++) {

            rx = ax * ax - ay * ay + bx;
            ry = 2.0 * ax * ay + by;

            if ((rx * rx + ry * ry) > 4.0) {
                j = i;
                break;
            }

            ax = rx;
            ay = ry;
        }

        k = 3 * j;
        
        r = param3[k];
        g = param3[k + 1];
        b = param3[k + 2];

        surf2Dwrite(make_uchar4(r, g, b, 0), object, x * sizeof(uchar4), y);
    }
}

void DoParallelComputing(double* Param1, size_t size1, int* Param2, size_t size2, unsigned char* Param3, size_t size3)
{
    LARGE_INTEGER freq, t1, t2;
    LONGLONG tmi, quo, ms, ss, mm, hh;
    cudaGraphicsResource* resource;
    cudaError_t result;
    cudaArray_t pointer;
    cudaResourceDesc descriptor;
    cudaSurfaceObject_t object;
    double* param1;
    int* param2;
    unsigned char* param3;

    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t1);

    // 1280 / 32 = 40   -> 40
    //  720 / 32 = 22.5 -> 23 -> check for out of index

    dim3 grid_size(40, 23);     // the number of thread blocks in the grid
    dim3 block_size(32, 32);    // the number of threads in a thread block

    result = cudaGraphicsGLRegisterImage(&resource, textures, GL_TEXTURE_2D, 0);

    if (result == cudaSuccess) {

        result = cudaGraphicsMapResources(1, &resource, 0);

        if (result == cudaSuccess) {

            result = cudaGraphicsSubResourceGetMappedArray(&pointer, resource, 0, 0);

            if (result == cudaSuccess) {

                ZeroMemory(&descriptor, sizeof(descriptor));

                descriptor.resType = cudaResourceTypeArray;
                descriptor.res.array.array = pointer;

                result = cudaCreateSurfaceObject(&object, &descriptor);

                if (result == cudaSuccess) {

                    cudaMalloc(&param1, size1);
                    cudaMalloc(&param2, size2);
                    cudaMalloc(&param3, size3);

                    cudaMemcpy(param1, Param1, size1, cudaMemcpyHostToDevice);
                    cudaMemcpy(param2, Param2, size2, cudaMemcpyHostToDevice);
                    cudaMemcpy(param3, Param3, size3, cudaMemcpyHostToDevice);

                    DoFractal << < grid_size, block_size >> > (param1, param2, param3, object);

                    cudaFree(param1);
                    cudaFree(param2);
                    cudaFree(param3);

                    cudaDestroySurfaceObject(object);

                    result = cudaDeviceSynchronize();

                    if (result != cudaSuccess) {
                        OutputDebugStringA("cudaDeviceSynchronize error - ");
                        OutputDebugStringA(cudaGetErrorName(cudaGetLastError()));
                        OutputDebugStringA("\n");
                    }
                }
                else {
                    OutputDebugStringA("cudaCreateSurfaceObject error - ");
                    OutputDebugStringA(cudaGetErrorName(cudaGetLastError()));
                    OutputDebugStringA("\n");
                }
            }
            else {
                OutputDebugStringA("cudaGraphicsSubResourceGetMappedArray error - ");
                OutputDebugStringA(cudaGetErrorName(cudaGetLastError()));
                OutputDebugStringA("\n");
            }

            cudaGraphicsUnmapResources(1, &resource, 0);
        }
        else {
            OutputDebugStringA("cudaGraphicsMapResources error - ");
            OutputDebugStringA(cudaGetErrorName(cudaGetLastError()));
            OutputDebugStringA("\n");
        }
    }
    else {
        OutputDebugStringA("cudaGraphicsGLRegisterImage error - ");
        OutputDebugStringA(cudaGetErrorName(cudaGetLastError()));
        OutputDebugStringA("\n");
    }

    QueryPerformanceCounter(&t2);
    tmi = ((t2.QuadPart - t1.QuadPart) * 1000LL) / freq.QuadPart;

    quo = tmi / 1000;
    ms = tmi % 1000;

    tmi = quo;

    quo = tmi / 60;
    ss = tmi % 60;

    tmi = quo;

    hh = tmi / 60;
    mm = tmi % 60;

    printf("%lld:%02lld:%02lld:%03lld\n", hh, mm, ss, ms);
}


void OnLButtonDown(HWND hWnd, WORD Key, int x, int y)
{
    EnableMenuItem(hMenu, IDM_FRACTAL, MF_BYCOMMAND | MF_DISABLED);

    is_dragging = true;
    show_selection = false;

    px = x;
    py = y;
}

void OnLButtonUp(HWND hWnd, WORD Key, int x, int y)
{
    is_dragging = false;
}

void OnMouseMove(HWND hWnd, WORD Key, int x, int y)
{
    if (is_dragging) {

        // arrange coordinate in ascending order
        if (px > x) {
            px1 = x;
            px2 = px;
        }
        else {
            px1 = px;
            px2 = x;
        }

        if (py > y) {
            py2 = height - y;
            py1 = height - py;
        }
        else {
            py2 = height - py;
            py1 = height - y;
        }

        // enable this menu if there is a selection
        if ((px2 - px1) > 2 && (py2 - py1) > 2) {

            EnableMenuItem(hMenu, IDM_FRACTAL, MF_BYCOMMAND | MF_ENABLED);

            show_selection = true;
            square2.Update((float)px1, (float)py1, (float)(px2), (float)py2);
        }
    }
}

void OnSize(HWND hWnd, int width, int height)
{
    printf("%10d%10d\n", width, height);

    ::height = height;

    float left, right, bottom, top, znear, zfar;

    left = 0.0f;
    right = (float)width;

    bottom = 0.0f;
    top = (float)height;

    znear = 0.0f;
    zfar = 1.0f;

    matrix.Orthographic(left, right, bottom, top, znear, zfar);

    glViewport(0, 0, width, height);
}

void OnPaint(HDC hDC)
{
    glClear(GL_COLOR_BUFFER_BIT);

    shader1.Use();
    square1.Draw(matrix);

    if (show_selection) {
        shader2.Use();
        square2.Draw(matrix);
    }

    SwapBuffers(hDC);
}

void OnCreate(HWND hWnd, HDC* hDC)
{
    PIXELFORMATDESCRIPTOR pfd;
    int format;
    HGLRC hglrc;

    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);  // size of structured data
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
    format = ChoosePixelFormat(*hDC, &pfd); // get the best available match of pixel format for the device context
    SetPixelFormat(*hDC, format, &pfd);     // make that the pixel format of the device context
    hglrc = wglCreateContext(*hDC);             // create an OpenGL rendering context
    wglMakeCurrent(*hDC, hglrc);                // make it the current rendering context

    // Load OpenGL functions.
    LoadOpenGLFunctions();

    printf("OpenGL Version :%s\n", glGetString(GL_VERSION));
    printf("GLES Version   :%s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("GLU Version    :%s\n", glGetString(GLU_VERSION));
    printf("Vendor         :%s\n", glGetString(GL_VENDOR));
    printf("Renderer       :%s\n", glGetString(GL_RENDERER));

    //   +-----------------------------------------+
    //   |                  TEXTURE                |
    //   +-----------------------------------------+

    char source1[] = "#version 330\n"
        "precision mediump float;\n"
        "in vec3 v_vertex;\n"
        "in vec2 v_texture;\n"
        "out vec2 st;\n"
        "uniform mat4 m_matrix;\n"
        "void main()\n"
        "{\n"
        "st = v_texture;\n"
        "gl_Position = m_matrix * vec4(v_vertex, 1.0);\n"
        " }\n";

    char source2[] = "#version 330\n"
        "precision mediump float;\n"
        "in vec2 st;\n"
        "out vec4 FragColor;\n"
        "uniform sampler2D sampler;\n"
        "void main()\n"
        "{\n"
        "FragColor = texture(sampler, st);\n"
        "}\n";

    shader1.Create(source1, source2);
    square1.Create(shader1.Get(), (float)FRACTAL_WIDTH, (float)FRACTAL_HEIGHT);

    //   +-----------------------------------------+
    //   |          SELECTION RECTANGLE            |
    //   +-----------------------------------------+

    char source3[] = "#version 330\n"
        "precision mediump float;\n"
        "in vec3 v_vertex;\n"
        "uniform mat4 m_matrix;\n"
        "void main()\n"
        "{\n"
        "gl_Position = m_matrix * vec4(v_vertex, 1.0);\n"
        "}\n";

    char source4[] = "#version 330\n"
        "precision mediump float;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "FragColor = vec4(1.0, 1.0, 0.0, 1.0);\n"
        "}\n";

    shader2.Create(source3, source4);
    square2.Create(shader2.Get());

    //   +-----------------------------------------+
    //   |      PARAMETER INITIALIZATION           |
    //   +-----------------------------------------+

    hMenu = CreateAppMenu(hWnd);
    EnableMenuItem(hMenu, IDM_FRACTAL, MF_BYCOMMAND | MF_DISABLED);
    EnableMenuItem(hMenu, IDM_GO_BACK, MF_BYCOMMAND | MF_DISABLED);

    show_selection = is_dragging = false;

    Param1 = new double[PARAM_1_COUNT];
    Param2 = new int[PARAM_2_COUNT];
    Param3 = new unsigned char[PARAM_3_COUNT];

    Param2[0] = FRACTAL_WIDTH;
    Param2[1] = FRACTAL_HEIGHT;
    Param2[2] = MAX_ITERATION_COUNT;

    Param1[0] = 4.5;                                                    // cx
    Param1[1] = Param1[0] * ((double)Param2[1] / (double)Param2[0]);    // cy
    Param1[2] = -Param1[0] * 0.6;                                       // ox
    Param1[3] = -Param1[1] * 0.5;                                       // oy

    CreateColorGradient(Param3, MAX_ITERATION_COUNT);

    glGenTextures(1, &textures);
    glBindTexture(GL_TEXTURE_2D, textures);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FRACTAL_WIDTH, FRACTAL_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glGenerateMipmap(GL_TEXTURE_2D);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // do parallel computing after window is displayed
    PostMessage(hWnd, WM_AFTERWINDOWDISPLAYED, 0, 0);
}

void OnDestroy(HWND hWnd, HDC hDC)
{
    // release these objects.
    square1.Destroy();
    square2.Destroy();

    shader1.Destroy();
    shader2.Destroy();

    delete[] Param1;
    delete[] Param2;
    delete[] Param3;

    glDeleteTextures(1, &textures);

    DestroyAppMenu(hMenu);

    HGLRC hglrc;

    wglMakeCurrent(hDC, NULL);      // get current OpenGL rendering context
    hglrc = wglGetCurrentContext(); // make the rendering context not current
    wglDeleteContext(hglrc);        // delete the rendering context
    ReleaseDC(hWnd, hDC);           // releases a device context

    // close the program.
    PostQuitMessage(0);
}

void OnFileOpen(HWND hWnd)
{
    HRESULT hr;
    IFileOpenDialog* pFileOpen;
    FILEOPENDIALOGOPTIONS options;
    COMDLG_FILTERSPEC fs[2];
    IShellItem* pItem;
    PWSTR pszFile;
    wchar_t filename[MAX_PATH];
    wchar_t name[2][20], spec[2][6];
    char str[MAX_PATH];
    bool cancel;

    cancel = true;

    // Initialize COM.
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (SUCCEEDED(hr)) {

        // Create the FileOpenDialog object.
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr)) {

            // Filter.
            wcscpy_s(name[0], 20, L"Mandelbrot");
            wcscpy_s(spec[0], 6, L"*.mdl");

            fs[0].pszName = name[0];
            fs[0].pszSpec = spec[0];

            wcscpy_s(name[1], 20, L"All Files");
            wcscpy_s(spec[1], 6, L"*.*");

            fs[1].pszName = name[1];
            fs[1].pszSpec = spec[1];

            pFileOpen->SetFileTypes(2, fs);
            hr = pFileOpen->GetOptions(&options);

            if (SUCCEEDED(hr)) {
                options |= FOS_STRICTFILETYPES;
                pFileOpen->SetOptions(options);
            }

            // Show the Save dialog box.
            hr = pFileOpen->Show(hWnd);

            if (SUCCEEDED(hr)) {

                // Get the result object.
                hr = pFileOpen->GetResult(&pItem);

                if (SUCCEEDED(hr)) {

                    // Gets the filename that the user made in the dialog.
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFile);

                    // Copy the file name.
                    if (SUCCEEDED(hr)) {

                        cancel = false;

                        wcscpy_s(filename, MAX_PATH, pszFile);

                        // Release memory.
                        CoTaskMemFree(pszFile);
                    }

                    // Release result object.
                    pItem->Release();
                }
            }

            // Release FileOpenDialog object.
            pFileOpen->Release();
        }

        // Release COM.
        CoUninitialize();
    }

    if (cancel) return;

    if (!stack.Open(filename)) return;

    sprintf_s(str, MAX_PATH, "%s - %ws", szTitle, filename);
    SetWindowText(hWnd, str);

    EnableMenuItem(hMenu, IDM_GO_BACK, MF_BYCOMMAND | MF_ENABLED);
    OnViewGoBack(hWnd);
}

void OnFileSaveAs(HWND hWnd)
{
    HRESULT hr;
    IFileSaveDialog* pFileSave;
    FILEOPENDIALOGOPTIONS options;
    COMDLG_FILTERSPEC fs[2];
    IShellItem* pItem;
    PWSTR pszFile;
    UINT filetype;
    wchar_t filename[MAX_PATH];
    wchar_t name[2][20], spec[2][6];
    char str[MAX_PATH];
    bool cancel;

    cancel = true;

    // Initialize COM.
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (SUCCEEDED(hr)) {

        // Create the FileSaveDialog object.
        hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));

        if (SUCCEEDED(hr)) {

            // Set default filename extension
            pFileSave->SetDefaultExtension(L"mdl");

            // Set default filename.
            GetUniqueName(filename, MAX_PATH);
            pFileSave->SetFileName(filename);

            // Filter
            // 0
            wcscpy_s(name[0], 20, L"Mandelbrot");
            wcscpy_s(spec[0], 6, L"*.mdl");

            fs[0].pszName = name[0];
            fs[0].pszSpec = spec[0];

            // 1
            wcscpy_s(name[1], 20, L"All Files");
            wcscpy_s(spec[1], 6, L"*.*");

            fs[1].pszName = name[1];
            fs[1].pszSpec = spec[1];

            pFileSave->SetFileTypes(2, fs);
            hr = pFileSave->GetOptions(&options);

            if (SUCCEEDED(hr)) {
                options |= FOS_STRICTFILETYPES;
                pFileSave->SetOptions(options);
            }

            // Show the Save dialog box.
            hr = pFileSave->Show(hWnd);

            if (SUCCEEDED(hr)) {

                // Get the result object.
                hr = pFileSave->GetResult(&pItem);

                if (SUCCEEDED(hr)) {

                    // Gets the filename that the user made in the dialog.
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFile);

                    // Copy the file name.
                    if (SUCCEEDED(hr)) {

                        cancel = false;

                        wcscpy_s(filename, MAX_PATH, pszFile);

                        pFileSave->GetFileTypeIndex(&filetype);

                        // Release memory.
                        CoTaskMemFree(pszFile);
                    }

                    // Release result object.
                    pItem->Release();
                }
            }

            // Release FileSaveDialog object.
            pFileSave->Release();
        }

        // Release COM.
        CoUninitialize();
    }

    if (cancel) return;

    if (!stack.Save(filename, Param1[0], Param1[1], Param1[2], Param1[3])) return;

    sprintf_s(str, MAX_PATH, "%s - %ws", szTitle, filename);
    SetWindowText(hWnd, str);
}

void OnFileExit(HWND hWnd)
{
    DestroyWindow(hWnd);
}

void OnViewFractal(HWND hWnd)
{
    double x1, y1, x2, y2, sx, sy, cx, cy, ox, oy;

    EnableMenuItem(hMenu, IDM_FRACTAL, MF_BYCOMMAND | MF_DISABLED);
    EnableMenuItem(hMenu, IDM_GO_BACK, MF_BYCOMMAND | MF_ENABLED);

    show_selection = false;

    stack.Push(Param1[0], Param1[1], Param1[2], Param1[3]);

    x1 = ((double)px1 / (double)(Param2[0] - 1)) * Param1[0] + Param1[2];
    x2 = ((double)px2 / (double)(Param2[0] - 1)) * Param1[0] + Param1[2];

    y2 = ((double)py2 / (double)(Param2[1] - 1)) * Param1[1] + Param1[3];
    y1 = ((double)py1 / (double)(Param2[1] - 1)) * Param1[1] + Param1[3];

    sx = x2 - x1;
    sy = y2 - y1;

    cx = sx;
    cy = ((double)Param2[1] / (double)Param2[0]) * cx;

    if (cy < sy) {
        cy = sy;
        cx = ((double)Param2[0] / (double)Param2[1]) * cy;
    }

    ox = (x1 + x2 - cx) / 2.0;
    oy = (y1 + y2 - cy) / 2.0;

    Param1[0] = cx;
    Param1[1] = cy;
    Param1[2] = ox;
    Param1[3] = oy;

    DoParallelComputing(Param1, PARAM_1_SIZE, Param2, PARAM_2_SIZE, Param3, PARAM_3_SIZE);
}

void OnViewGoBack(HWND hWnd)
{
    stack.Pop(&Param1[0], &Param1[1], &Param1[2], &Param1[3]);

    show_selection = false;

    if (stack.IsEmpty())
        EnableMenuItem(hMenu, IDM_GO_BACK, MF_BYCOMMAND | MF_DISABLED);
    else
        EnableMenuItem(hMenu, IDM_GO_BACK, MF_BYCOMMAND | MF_ENABLED);

    DoParallelComputing(Param1, PARAM_1_SIZE, Param2, PARAM_2_SIZE, Param3, PARAM_3_SIZE);
}

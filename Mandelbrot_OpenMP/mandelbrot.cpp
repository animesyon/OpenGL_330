
// Mandelbrot
// This program use CPU core (OpenMP).

#include "framework.h"
#include "mandelbrot.h"
#include "matrix.h"
#include "shader.h"
#include "square1.h"
#include "square2.h"
#include "stack.h"

typedef struct {
    HWND hWnd;
    DWORD id;
    int width, height, max_iteration;
    double cx, cy, ox, oy;
    unsigned char* gradient;
    unsigned char* pixel;
} Fractal_Data;

const int MAX_LOADSTRING = 100;

const int MAX_ITERATION = 32768;

const int FRACTAL_WIDTH = 1280;
const int FRACTAL_HEIGHT = 720;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // main window class name

CMatrix matrix;
CShader shader1, shader2;
CSquare1 square1;
CSquare2 square2;
CStack stack;
GLuint textures;
Fractal_Data data;
int height, px, py, px1, py1, px2, py2;
bool is_dragging, show_selection;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK    WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void OnAfterWindowDisplayed(HWND hWnd, WPARAM wParam, LPARAM lParam);

void GetUniqueName(wchar_t* filename, DWORD size);
void CreateColorGradient(unsigned char* gradient, int count);
void DoFractal(int i1, int i2, Fractal_Data* data);
void DoParallelComputing(Fractal_Data* data);

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

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MANDELBROT, szWindowClass, MAX_LOADSTRING);

    // Registers the window class.
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MANDELBROT));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MANDELBROT);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    RegisterClassExW(&wcex);

    // Store instance handle in our global variable.
    hInst = hInstance;

    // Creates main window.
    int X, Y, nWidth, nHeight, CX, CY;

    CX = FRACTAL_WIDTH;
    CY = FRACTAL_HEIGHT;

    nWidth = CX + 16;
    nHeight = CY + 59;

    X = (GetSystemMetrics(SM_CXSCREEN) - nWidth) / 3;
    Y = (GetSystemMetrics(SM_CYSCREEN) - nHeight) / 3;

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
    Fractal_Data* param = (Fractal_Data*)lParam;
    DoParallelComputing(param);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data.width, data.height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.pixel);
}

void GetUniqueName(wchar_t* filename, DWORD size)
{
    time_t ltime;
    struct tm a;

    time(&ltime);
    _localtime64_s(&a, &ltime);

    swprintf_s(filename, size, L"%d%02d%02d%02d%02d%02d.mdl", a.tm_year + 1900, a.tm_mon + 1, a.tm_mday, a.tm_hour, a.tm_min, a.tm_sec);
}

// pixel is a series of rgb value
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

// i1 - the horizontal pixel position
// i2 - the vertical pixel position
void DoFractal(int i1, int i2, Fractal_Data* data)
{
    double ax, ay, bx, by, rx, ry;
    int i, j, k, k1, k2;

    for (i = i1; i < i2; i++) {

        for (j = 0; j < data->width; j++) {

            bx = ((double)j / (double)(data->width - 1)) * data->cx + data->ox;
            by = ((double)i / (double)(data->height - 1)) * data->cy + data->oy;

            ax = 0.0;
            ay = 0.0;

            k2 = 0;

            for (k = 0; k < data->max_iteration; k++) {

                rx = ax * ax - ay * ay + bx;
                ry = 2.0 * ax * ay + by;

                if ((rx * rx + ry * ry) > 4.0) {
                    k2 = k;
                    break;
                }

                ax = rx;
                ay = ry;
            }

            k1 = 3 * (i * data->width + j);
            k2 *= 3;

            data->pixel[k1] = data->gradient[k2];
            data->pixel[k1 + 1] = data->gradient[k2 + 1];
            data->pixel[k1 + 2] = data->gradient[k2 + 2];
        }
    }
}

void DoParallelComputing(Fractal_Data* data)
{
    int *d, i, *i1, *i2, k, n, quo, rem;

    // divide the process to be executed in cpu core
    n = omp_get_num_procs();
    quo = data->height / n;
    rem = data->height % n;

    d = new int[n];
    i1 = new int[n];
    i2 = new int[n];

    for (i = 0; i < n; i++)
        d[i] = quo;

    if (rem > 0)
        for (i = 0; i < rem; i++)
            ++d[i];

    k = 0;

    for (i = 0; i < n; i++) {

        i1[i] = k;
        i2[i] = k + d[i];

        k += d[i];
    }

    delete[] d;

    // execute function DoFractal in parallel
    #pragma omp parallel for default(none) shared(i1, i2, data)

    for (i = 0; i < n; i++) {
        DoFractal(i1[i], i2[i], data);
    }

    delete[] i1;
    delete[] i2;
}

void OnLButtonDown(HWND hWnd, WORD Key, int x, int y)
{
    HMENU hMenu = GetMenu(hWnd);
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

            HMENU hMenu = GetMenu(hWnd);
            EnableMenuItem(hMenu, IDM_FRACTAL, MF_BYCOMMAND | MF_ENABLED);

            show_selection = true;
            square2.Update((float)px1, (float)py1, (float)(px2), (float)py2);
        }
    }
}

void OnSize(HWND hWnd, int width, int height)
{
    char str[100];
    sprintf_s(str, 100, "%10d%10d\n", width, height);
    OutputDebugStringA(str);

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

    char str[100];
    OutputDebugStringA("-----------------------------------------------------------------------------\n");
    sprintf_s(str, 100, "OpenGL Version :%s\n", glGetString(GL_VERSION));   OutputDebugStringA(str);
    sprintf_s(str, 100, "GLES Version   :%s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));  OutputDebugStringA(str);
    sprintf_s(str, 100, "GLU Version    :%s\n", glGetString(GLU_VERSION));  OutputDebugStringA(str);
    sprintf_s(str, 100, "Vendor         :%s\n", glGetString(GL_VENDOR));    OutputDebugStringA(str);
    sprintf_s(str, 100, "Renderer       :%s\n", glGetString(GL_RENDERER));  OutputDebugStringA(str);
    OutputDebugStringA("-----------------------------------------------------------------------------\n");

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

    HMENU hMenu = GetMenu(hWnd);
    EnableMenuItem(hMenu, IDM_FRACTAL, MF_BYCOMMAND | MF_DISABLED);
    EnableMenuItem(hMenu, IDM_GO_BACK, MF_BYCOMMAND | MF_DISABLED);

    is_dragging = show_selection = false;

    data.gradient = new unsigned char[3 * MAX_ITERATION];
    data.pixel = new unsigned char[3 * FRACTAL_WIDTH * FRACTAL_HEIGHT];

    data.hWnd = hWnd;

    data.max_iteration = MAX_ITERATION;

    data.width = FRACTAL_WIDTH;
    data.height = FRACTAL_HEIGHT;

    data.cx = 4.5;
    data.cy = data.cx * ((double)data.height / (double)data.width);

    data.ox = -data.cx * 0.65;
    data.oy = -data.cy * 0.5;

    CreateColorGradient(data.gradient, MAX_ITERATION);

    glGenTextures(1, &textures);
    glBindTexture(GL_TEXTURE_2D, textures);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // do parallel computing after window is displayed
    PostMessage(hWnd, WM_AFTERWINDOWDISPLAYED, 0, (LPARAM)&data);
}

void OnDestroy(HWND hWnd, HDC hDC)
{
    // release these objects.
    square1.Destroy();
    square2.Destroy();

    shader1.Destroy();
    shader2.Destroy();

    delete[] data.gradient;
    delete[] data.pixel;

    glDeleteTextures(1, &textures);

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
    wchar_t str[MAX_PATH], name[2][20], spec[2][6];
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

    swprintf_s(str, MAX_PATH, L"%s - %s", szTitle, filename);
    SetWindowText(hWnd, str);

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
    wchar_t name[2][20], spec[2][6], str[MAX_PATH];
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

    if (!stack.Save(filename, data.cx, data.cy, data.ox, data.oy)) return;

    swprintf_s(str, MAX_PATH, L"%s - %s", szTitle, filename);
    SetWindowText(hWnd, str);
}

void OnFileExit(HWND hWnd)
{
    DestroyWindow(hWnd);
}

void OnViewFractal(HWND hWnd)
{
    double x1, y1, x2, y2, sx, sy, cx, cy, ox, oy;

    HMENU hMenu = GetMenu(hWnd);
    EnableMenuItem(hMenu, IDM_FRACTAL, MF_BYCOMMAND | MF_DISABLED);
    EnableMenuItem(hMenu, IDM_GO_BACK, MF_BYCOMMAND | MF_ENABLED);

    show_selection = false;

    stack.Push(data.cx, data.cy, data.ox, data.oy);

    x1 = ((double)px1 / (double)(data.width - 1)) * data.cx + data.ox;
    x2 = ((double)px2 / (double)(data.width - 1)) * data.cx + data.ox;

    y2 = ((double)py2 / (double)(data.height - 1)) * data.cy + data.oy;
    y1 = ((double)py1 / (double)(data.height - 1)) * data.cy + data.oy;

    sx = x2 - x1;
    sy = y2 - y1;

    cx = sx;
    cy = ((double)data.height / (double)data.width) * cx;

    if (cy < sy) {
        cy = sy;
        cx = ((double)data.width / (double)data.height) * cy;
    }

    ox = (x1 + x2 - cx) / 2.0;
    oy = (y1 + y2 - cy) / 2.0;

    data.cx = cx;
    data.cy = cy;
    data.ox = ox;
    data.oy = oy;

    DoParallelComputing(&data);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data.width, data.height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.pixel);
}

void OnViewGoBack(HWND hWnd)
{
    stack.Pop(&data.cx, &data.cy, &data.ox, &data.oy);

    DoParallelComputing(&data);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data.width, data.height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.pixel);

    HMENU hMenu = GetMenu(hWnd);

    if (stack.IsEmpty())
        EnableMenuItem(hMenu, IDM_GO_BACK, MF_BYCOMMAND | MF_DISABLED);
    else
        EnableMenuItem(hMenu, IDM_GO_BACK, MF_BYCOMMAND | MF_ENABLED);
}

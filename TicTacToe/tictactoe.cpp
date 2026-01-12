
// Programming Tic Tac Toe using Minimax Algorithm.

#include "framework.h"
#include "tictactoe.h"
#include "matrix.h"
#include "shader.h"
#include "grid.h"
#include "circle.h"
#include "cross.h"
#include "line.h"

const int MAX_LOADSTRING = 100;

const int M = 3;
const int N = 3;

enum Player {
    EMPTY,
    AI,
    HUMAN
};

typedef struct {
    int i, j;
    bool found;
}TurnResult;

typedef struct {
    float x, y;
    Player player;
}Board;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // main window class name

CMatrix matrix1;
CShader shader;
CGrid grid;
CCircle circle;
CCross cross;
CLine line;
Board board[N][M];
int height;
float uppx, uppy, offx, offy;
bool game_over;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK    WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void OnAfterWindowDisplayed(HWND hWnd, WPARAM wParam, LPARAM lParam);

float toUnitX(int x, float uppx, float offx);
float toUnitY(int y, int height, float uppy, float offy);

int Minimax(Board board[N][M], Player player);
bool CheckForWin(Board board[N][M], int* score);
bool CheckForWin(Board board[N][M], CLine* line);
int GetScore(Player player);
Player TogglePlayer(Player player);

void EmptyBoard(Board board[N][M]);

TurnResult* HumanTurn(Board board[N][M], int x, int y);
void AITurn(Board board[N][M]);

void OnLButtonDown(HWND hWnd, int x, int y);
void OnRButtonDown(HWND hWnd, int x, int y);

void OnSize(HWND hWnd, int width, int height);

void OnPaint(HDC hDC);
void OnCreate(HWND hWnd, HDC* hDC);
void OnDestroy(HWND hWnd, HDC hDC);

void OnToolsReset(HWND hWnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TICTACTOE, szWindowClass, MAX_LOADSTRING);

    // Registers the window class.
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TICTACTOE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    RegisterClassExW(&wcex);

    // Store instance handle in our global variable.
    hInst = hInstance;

    // Creates main window.
    int X, Y, nWidth, nHeight, Cx, Cy;

    // 1080p: 1920 x 1080
    //  720p: 1280 x  720
    //  480p:  854 x  480

    Cx = 854;
    Cy = 480;

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

// Processes messages for the main window.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC hDC;

    switch (message)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_RESET:		OnToolsReset(hWnd);		break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_AFTERWINDOWDISPLAYED:   OnAfterWindowDisplayed(hWnd, wParam, lParam);             break;
    case WM_LBUTTONDOWN:            OnLButtonDown(hWnd, LOWORD(lParam), HIWORD(lParam));  break;
    case WM_RBUTTONDOWN:            OnRButtonDown(hWnd, LOWORD(lParam), HIWORD(lParam));  break;
    case WM_SIZE:                   OnSize(hWnd, LOWORD(lParam), HIWORD(lParam));  break;
    case WM_PAINT:                  OnPaint(hDC);                                             break;
    case WM_CREATE:                 OnCreate(hWnd, &hDC);                                     break;
    case WM_DESTROY:                OnDestroy(hWnd, hDC);                                     break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void OnAfterWindowDisplayed(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    EmptyBoard(board);
    AITurn(board);
}

float toUnitX(int x, float uppx, float offx)
{
    return ((float)x * uppx + offx);
}

// AI - maximizing player - pick positive score
// HUMAN - minimizing player - pick negative score
int Minimax(Board board[N][M], Player player)
{
    int i, j, score, best_score;
    bool result;

    result = CheckForWin(board, &score);
    if (result) return score;

    switch (player) {
    case AI:    best_score = INT_MIN; break;
    case HUMAN: best_score = INT_MAX; break;
    }

    for (i = 0; i < N; i++) {

        for (j = 0; j < M; j++) {

            if (board[i][j].player == EMPTY) {

                board[i][j].player = player;

                score = Minimax(board, TogglePlayer(player));

                board[i][j].player = EMPTY;

                switch (player) {
                case AI:    best_score = max(score, best_score); break; // maximize
                case HUMAN: best_score = min(score, best_score); break; // minimize
                }
            }
        }
    }

    return best_score;
}

bool CheckForWin(Board board[N][M], int* score)
{
    Player player[2];
    int i, j, k;
    bool result;

    player[0] = AI;
    player[1] = HUMAN;

    result = false;

    for (k = 0; k < 2; k++) {

        // horizontal line
        for (i = 0; i < N; i++)
            if (board[i][0].player == player[k] && board[i][1].player == player[k] && board[i][2].player == player[k]) {
                *score = GetScore(player[k]);
                result = true;
                goto dito;
            }

        // vertical line
        for (j = 0; j < M; j++)
            if (board[0][j].player == player[k] && board[1][j].player == player[k] && board[2][j].player == player[k]) {
                *score = GetScore(player[k]);
                result = true;
                goto dito;
            }

        // cross
        if (board[0][0].player == player[k] && board[1][1].player == player[k] && board[2][2].player == player[k]) {
            *score = GetScore(player[k]);
            result = true;
            goto dito;
        }

        if (board[2][0].player == player[k] && board[1][1].player == player[k] && board[0][2].player == player[k]) {
            *score = GetScore(player[k]);
            result = true;
            goto dito;
        }
    }

    if (!result) {

        for (i = 0; i < N; i++)
            for (j = 0; j < M; j++)
                if (board[i][j].player == EMPTY) {
                    goto dito;
                }

        *score = GetScore(EMPTY);
        result = true;
    }

dito:

    return result;
}

bool CheckForWin(Board board[N][M], CLine* line)
{
    Player player[2];
    float x1, y1, x2, y2;
    int i, j, k;
    bool result;

    player[0] = AI;
    player[1] = HUMAN;

    result = false;

    for (k = 0; k < 2; k++) {

        // horizontal line
        for (i = 0; i < N; i++)
            if (board[i][0].player == player[k] && board[i][1].player == player[k] && board[i][2].player == player[k]) {

                x1 = -0.45f;
                x2 = 0.45f;

                y1 = board[i][1].y - 0.01f;
                y2 = board[i][1].y + 0.01f;

                line->Update(x1, y1, x2, y2);

                result = true;
                goto dito;
            }

        // vertical line
        for (j = 0; j < M; j++)
            if (board[0][j].player == player[k] && board[1][j].player == player[k] && board[2][j].player == player[k]) {

                x1 = board[1][j].x - 0.01f;
                x2 = board[1][j].x + 0.01f;

                y1 = -0.45f;
                y2 = 0.45f;

                line->Update(x1, y1, x2, y2);

                result = true;
                goto dito;
            }

        // cross
        if (board[0][0].player == player[k] && board[1][1].player == player[k] && board[2][2].player == player[k]) {

            line->Update(-0.442929f, 0.457071f, 0.457071f, -0.442929f, 0.442929f, -0.457071f, -0.457071f, 0.442929f);

            result = true;
            goto dito;
        }

        // cross
        if (board[2][0].player == player[k] && board[1][1].player == player[k] && board[0][2].player == player[k]) {

            line->Update(0.457071f, 0.442929f, -0.442929f, -0.457071f, -0.457071f, -0.442929f, 0.442929f, 0.457071f);

            result = true;
            goto dito;
        }
    }

dito:

    return result;
}

// if AI wins, return a score of positive one
// if HUMAN wins, return a score of negative one
// if Tie, return a score of zero
int GetScore(Player player)
{
    int score;

    switch (player) {
    case AI:    score = 1;  break;
    case HUMAN: score = -1; break;
    default:  score = 0;
    }

    return score;
}

Player TogglePlayer(Player player)
{
    return (player == HUMAN ? AI : HUMAN);
}

float toUnitY(int y, int height, float uppy, float offy)
{
    return ((float)(height - y) * uppy + offy);
}

void EmptyBoard(Board board[N][M])
{
    int i, j;
    float x, y;

    y = -0.3f;

    for (i = (N - 1); i > -1; i--) {

        x = -0.3f;

        for (j = 0; j < M; j++) {

            board[i][j].x = x;
            board[i][j].y = y;

            board[i][j].player = EMPTY;

            x += 0.3f;
        }

        y += 0.3f;
    }
}

TurnResult* HumanTurn(Board board[N][M], int x, int y)
{
    static TurnResult result;
    int i, j;
    float ux, uy, x1, y1, x2, y2;

    ux = toUnitX(x, uppx, offx);
    uy = toUnitY(y, height, uppy, offy);

    result.found = false;

    for (i = 0; i < N; i++) {

        for (j = 0; j < M; j++) {

            x1 = board[i][j].x - 0.1f;
            x2 = board[i][j].x + 0.1f;

            y1 = board[i][j].y - 0.1f;
            y2 = board[i][j].y + 0.1f;

            if (x1 < ux && ux < x2 && y1 < uy && uy < y2) {

                if (board[i][j].player == EMPTY) {

                    result.i = i;
                    result.j = j;

                    result.found = true;

                    break;
                }
            }
        }
    }

    return &result;
}

// AI - maximizing player - pick positive score
void AITurn(Board board[N][M])
{
    int I, J, i, j, score, max_score;

    max_score = INT_MIN;

    for (i = 0; i < N; i++) {

        for (j = 0; j < M; j++) {

            if (board[i][j].player == EMPTY) {

                board[i][j].player = AI;

                score = Minimax(board, HUMAN);

                board[i][j].player = EMPTY;

                if (score > max_score) {

                    max_score = score;

                    I = i;
                    J = j;
                }
            }
        }
    }

    board[I][J].player = AI;
}

void OnLButtonDown(HWND hWnd, int x, int y)
{
    if (game_over) return;

    TurnResult* result;

    result = HumanTurn(board, x, y);

    if (CheckForWin(board, &line)) {
        game_over = true;
        return;
    }

    if (result->found) {

        board[result->i][result->j].player = HUMAN;

        AITurn(board);

        if (CheckForWin(board, &line)) {
            game_over = true;
            return;
        }
    }
}

void OnRButtonDown(HWND hWnd, int x, int y)
{
    HMENU hMenu;
    HMENU hSubMenu;
    POINT point;

    point.x = x;
    point.y = y;

    ClientToScreen(hWnd, &point);

    hMenu = LoadMenu(hInst, MAKEINTRESOURCEW(IDC_TICTACTOE));
    hSubMenu = GetSubMenu(hMenu, 0);
    TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, 0, hWnd, NULL);
}

void OnSize(HWND hWnd, int width, int height)
{
    if (width == 0) width = 1;
    if (height == 0) height = 1;

    float left, right, bottom, top, znear, zfar;
    float w, h;

    h = 1.0f;
    w = h * ((float)width / (float)height);

    left = -w / 2.0f;
    right = left + w;
    bottom = -h / 2.0f;
    top = bottom + h;
    znear = 0.0f;
    zfar = 1.0f;

    matrix1.Orthographic(left, right, bottom, top, znear, zfar);

    glViewport(0, 0, width, height);

    ::height = height;

    uppx = w / (float)width;
    uppy = h / (float)height;

    offx = left;
    offy = bottom;

}

void OnPaint(HDC hDC)
{
    CMatrix matrix, matrix2;
    int i, j;

    glClear(GL_COLOR_BUFFER_BIT);

    grid.Draw(matrix1);

    for (i = 0; i < N; i++) {

        for (j = 0; j < M; j++) {

            matrix2.Translate(board[i][j].x, board[i][j].y, 0.0f);
            matrix = matrix1 * matrix2;

            if (board[i][j].player == HUMAN)
                circle.Draw(matrix);
            else if (board[i][j].player == AI)
                cross.Draw(matrix);

        }
    }

    line.Draw(matrix1);

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
    //   |                  MODEL                  |
    //   +-----------------------------------------+

    char source1[] = "#version 330\n"
        "precision mediump float;\n"
        "in vec3 v_vertex;\n"
        "uniform mat4 m_matrix;\n"
        "void main()\n"
        "{\n"
        "gl_Position =  m_matrix * vec4(v_vertex, 1.0);\n"
        "}\n";

    char source2[] = "#version 330\n"
        "precision mediump float;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "FragColor = vec4(1.0, 1.0, 0.0 , 1.0);\n"
        "}\n";

    float color[] = { 0.0f, 0.0f, 1.0f, 1.0f };

    shader.Create(source1, source2);
    shader.Use();
    grid.Create(shader.Get());
    circle.Create(shader.Get());
    cross.Create(shader.Get());
    line.Create(shader.Get());

    //   +-----------------------------------------+
    //   |      PARAMETER INITIALIZATION           |
    //   +-----------------------------------------+

    game_over = false;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    PostMessage(hWnd, WM_AFTERWINDOWDISPLAYED, 0, (LPARAM)0);
}

void OnDestroy(HWND hWnd, HDC hDC)
{
    shader.Destroy();
    grid.Destroy();
    circle.Destroy();
    cross.Destroy();
    line.Destroy();

    HGLRC hglrc;

    wglMakeCurrent(hDC, NULL);      // get current OpenGL rendering context
    hglrc = wglGetCurrentContext(); // make the rendering context not current
    wglDeleteContext(hglrc);        // delete the rendering context
    ReleaseDC(hWnd, hDC);           // releases a device context

    // close the program.
    PostQuitMessage(0);
}

void OnToolsReset(HWND hWnd)
{
    game_over = false;
    line.Hide();
    EmptyBoard(board);
    AITurn(board);
}

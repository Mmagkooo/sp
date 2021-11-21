// check1.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "check1.h"
#include <fstream>
using namespace std;

#define ID_COMBOBOX_1 10001
#define ID_COMBOBOX_2 10002
#define ID_COMBOBOX_3 10003
#define ID_BUTTON_1 10004
#define ID_BUTTON_2 10005
#define MAX_LOADSTRING 100
const int x_size = 1000, y_size = 700;
// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна


HWND hwnd;
HDC hdc;


void OnLButtonDown(int x, int y);
void OnLButtonUp(int x, int y);
void OnMouseMove(int x, int y);

void Line(int x1, int y1, int x2, int y2, COLORREF color, int width, bool needMem);
void Ellipse(int x1, int y1, int x2, int y2, COLORREF color, int width, bool needMem);
void Rect(int x1, int y1, int x2, int y2, COLORREF color, int width, bool needMem);
void Fill(int x, int y, COLORREF color, bool needMem);
void GFill(int x, int y, bool needMem);
void check();
void SaveData();
void LoadData();
DWORD WINAPI fillGradient(LPVOID lpParam);

enum class ACTIONTYPE {
    LINE,
    ELLIPSE,
    FILL,
    GFILL,
    RECT
};

struct action {
    ACTIONTYPE type;
    INT x1, y1, x2, y2;
    COLORREF color;
    INT width;
};

struct THREAD_PARAMS {
    int x, y;
    bool memo;
};

vector<action> actions;
vector<action> check_actions;

enum class MODES {
    PEN,
    ERASER,
    FILL,
    GRADIENT,
    ELLIPSE,
    RECT
};
MODES mode = MODES::PEN;
COLORREF color = RGB(0, 0, 0);
int width = 3;
bool threadsStopped = false;

bool mouse_down = false;
bool started = true;
POINT ptPr;

HWND hWndComboBox1;
HWND hWndComboBox2;
HWND hWndComboBox3;




// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CHECK1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CHECK1));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHECK1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CHECK1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   hwnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hwnd)
   {
      return FALSE;
   }

   ShowWindow(hwnd, nCmdShow);
   UpdateWindow(hwnd);

   return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    switch (message)
    {
    case WM_CREATE:
        {
        //mode
        hWndComboBox1 = CreateWindow(L"COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWN,
            10, 10, 100, 800, hWnd, (HMENU)ID_COMBOBOX_1,
            (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);

        SendMessage(hWndComboBox1, CB_ADDSTRING, 0, (LPARAM)L"pen");
        SendMessage(hWndComboBox1, CB_ADDSTRING, 0, (LPARAM)L"eraser");
        SendMessage(hWndComboBox1, CB_ADDSTRING, 0, (LPARAM)L"fill");
        SendMessage(hWndComboBox1, CB_ADDSTRING, 0, (LPARAM)L"gradient fill");
        SendMessage(hWndComboBox1, CB_ADDSTRING, 0, (LPARAM)L"ellipse");
        SendMessage(hWndComboBox1, CB_ADDSTRING, 0, (LPARAM)L"rect");

        SendMessage(hWndComboBox1, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

        //color
        hWndComboBox2 = CreateWindow(L"COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWN,
            120, 10, 100, 800, hWnd, (HMENU)ID_COMBOBOX_2,
            (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);

        SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_2), CB_ADDSTRING, 0, (LPARAM)L"black");
        SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_2), CB_ADDSTRING, 0, (LPARAM)L"red");
        SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_2), CB_ADDSTRING, 0, (LPARAM)L"green");
        SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_2), CB_ADDSTRING, 0, (LPARAM)L"blue");
        SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_2), CB_ADDSTRING, 0, (LPARAM)L"yellow");
        SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_2), CB_ADDSTRING, 0, (LPARAM)L"purple");
        SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_2), CB_ADDSTRING, 0, (LPARAM)L"orange");
        SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_2), CB_ADDSTRING, 0, (LPARAM)L"pink");
        SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_2), CB_ADDSTRING, 0, (LPARAM)L"gray");
        SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_2), CB_ADDSTRING, 0, (LPARAM)L"brown");
        SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_2), CB_ADDSTRING, 0, (LPARAM)L"white");

        SendMessage(hWndComboBox2, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

        //width
        hWndComboBox3 = CreateWindow(L"COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWN,
            230, 10, 100, 800, hWnd, (HMENU)ID_COMBOBOX_3,
            (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);

        SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_3), CB_ADDSTRING, 0, (LPARAM)L"1");
        SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_3), CB_ADDSTRING, 0, (LPARAM)L"3");
        SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_3), CB_ADDSTRING, 0, (LPARAM)L"5");
        SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_3), CB_ADDSTRING, 0, (LPARAM)L"10");
        SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_3), CB_ADDSTRING, 0, (LPARAM)L"100");

        SendMessage(hWndComboBox3, CB_SETCURSEL, (WPARAM)1, (LPARAM)0);

        //button
        HWND hwndButton1 = CreateWindow(L"BUTTON", L"clear all", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            340, 10, 100, 24, hWnd, (HMENU)ID_BUTTON_1,
            (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);


        HWND hwndButton2 = CreateWindow(L"BUTTON", L"check", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            500, 10, 100, 24, hWnd, (HMENU)ID_BUTTON_2,
            (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
        }
        LoadData();
        break;
    case WM_LBUTTONDOWN:
        OnLButtonDown(LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_LBUTTONUP:
        OnLButtonUp(LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_MOUSEMOVE:
        OnMouseMove(LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_COMMAND:
        {
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                int ind;
                switch (LOWORD(wParam)) {
                case ID_COMBOBOX_1:
                    ind = SendMessage(hWndComboBox1, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                    switch (ind) {
                    case 0:
                        mode = MODES::PEN;
                        break;
                    case 1:
                        mode = MODES::ERASER;
                        break;
                    case 2:
                        mode = MODES::FILL;
                        break;
                    case 3:
                        mode = MODES::GRADIENT;
                        break;
                    case 4:
                        mode = MODES::ELLIPSE;
                        break;
                    case 5:
                        mode = MODES::RECT;
                        break;
                    default:
                        break;
                    }
                    break;
                case ID_COMBOBOX_2:
                    ind = SendMessage(hWndComboBox2, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                    switch (ind) {
                    case 0:
                        color = RGB(0, 0, 0);
                        break;
                    case 1:
                        color = RGB(255, 0, 0);
                        break;
                    case 2:
                        color = RGB(0, 255, 0);
                        break;
                    case 3:
                        color = RGB(0, 0, 255);
                        break;
                    case 4:
                        color = RGB(255, 255, 0);
                        break;
                    case 5:
                        color = RGB(128, 0, 128);
                        break;
                    case 6:
                        color = RGB(255, 127, 0);
                        break;
                    case 7:
                        color = RGB(255, 192, 203);
                        break;
                    case 8:
                        color = RGB(128, 128, 128);
                        break;
                    case 9:
                        color = RGB(150, 75, 0);
                        break;
                    case 10:
                        color = RGB(255, 255, 255);
                        break;
                    default:
                        break;
                    }
                    break;
                case ID_COMBOBOX_3:
                    ind = SendMessage(hWndComboBox3, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                    switch (ind) {
                    case 0:
                        width = 1;
                        break;
                    case 1:
                        width = 3;
                        break;
                    case 2:
                        width = 5;
                        break;
                    case 3:
                        width = 10;
                        break;
                    case 4:
                        width = 100;
                        break;
                    default:
                        break;
                    }
                    break;
                default:
                    break;
                }
            }
            if (LOWORD(wParam) == ID_BUTTON_1) {
                actions.clear();
                std::ofstream out;
                out.open("test.txt", std::ofstream::out | std::ofstream::trunc);
                out.close();
                threadsStopped = true;
                InvalidateRect(hWnd, NULL, TRUE);
            }

            if (LOWORD(wParam) == ID_BUTTON_2) {
                check(); 
                PostQuitMessage(0);
                DestroyWindow(hWnd);
            }
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            for (int i = 0; i < actions.size(); i++) {
                action act = actions[i];
                switch (act.type) {
                case ACTIONTYPE::LINE:
                    Line(act.x1, act.y1, act.x2, act.y2, act.color, act.width, false);
                    break;
                case ACTIONTYPE::ELLIPSE:
                    Ellipse(act.x1, act.y1, act.x2, act.y2, act.color, act.width, false);
                    break;
                case ACTIONTYPE::RECT:
                    Rect(act.x1, act.y1, act.x2, act.y2, act.color, act.width, false);
                case ACTIONTYPE::FILL:
                    Fill(act.x1, act.y1, act.color, false);
                    break;
                case ACTIONTYPE::GFILL:
                    GFill(act.x1, act.y1, false);
                    break;
                default:
                    break;
                }
            }
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}




void check() {
    SaveData();
    char szFileName[MAX_PATH];
    GetModuleFileNameA(NULL, szFileName, MAX_PATH);

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    CreateProcessA(szFileName, NULL, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
}

void SaveData() {
    std::ofstream out;
    out.open("test.txt");
    if (out.is_open())
    {
        for (int i = 0; i < actions.size(); i++) {
            out << actions[i].x1 << " " << actions[i].x2 << " " << actions[i].y1 << " " << actions[i].y2 << " " << actions[i].color << " " << actions[i].width << " " << (int)actions[i].type << endl;
        }
    }
    out.close();
}

void LoadData() {
    actions.clear();
    std::ifstream in("test.txt");
    INT check_type;
    INT check_x1, check_y1, check_x2, check_y2;
    COLORREF check_color;
    INT check_width;
    if (in.is_open())
    {
        while (in >> check_x1 >> check_x2 >> check_y1 >> check_y2 >> check_color >> check_width >> check_type)
        {
            actions.push_back({ ACTIONTYPE(check_type), check_x1, check_y1, check_x2, check_y2, check_color, check_width });
        }
    }
    in.close();
}

void OnLButtonDown(int x, int y)
{
    switch (mode) {
    case MODES::PEN:
    {
        mouse_down = true;
        ptPr.x = x;
        ptPr.y = y;
        return;
    }
    case MODES::ERASER:
    {
        mouse_down = TRUE;
        ptPr.x = x;
        ptPr.y = y;
        return;
    }
    case MODES::FILL:
    {
        Fill(x, y, color, true);
        return;
    }
    case MODES::GRADIENT:
    {
        ptPr.x = x;
        ptPr.y = y;
        return;
    }
    case MODES::ELLIPSE:
    {
        ptPr.x = x;
        ptPr.y = y;
        return;
    }
    case MODES::RECT:
    {
        ptPr.x = x;
        ptPr.y = y;
        return;
    }
    default:
        return;
    }
}

void OnLButtonUp(int x, int y)
{
    switch (mode) {
    case MODES::PEN:
    {
        if (mouse_down) {
            int x1 = ptPr.x, y1 = ptPr.y;
            ptPr.x = x; ptPr.y = y;
            Line(x1, y1, x, y, color, width, true);
        }
        mouse_down = false;
        return;
    }
    case MODES::ERASER:
    {
        if (mouse_down) {
            int x1 = ptPr.x, y1 = ptPr.y;
            ptPr.x = x; ptPr.y = y;
            Line(x1, y1, x, y, RGB(255, 255, 255), width, true);
        }
        mouse_down = false;
        return;
    }
    case MODES::FILL:
    {
        return;
    }
    case MODES::GRADIENT:
    {
        THREAD_PARAMS params;
        params.x = x;
        params.y = y;
        params.memo = true;
        DWORD dwThreadId;
        threadsStopped = false;

        HANDLE hThread = CreateThread(NULL, 0, fillGradient, &params, 0, &dwThreadId);
        return;
    }
    case MODES::ELLIPSE:
    {
        int dx = abs(x - ptPr.x);
        int dy = abs(y - ptPr.y);
        int x1 = ptPr.x - dx;
        int y1 = ptPr.y - dy;
        int x2 = ptPr.x + dx;
        int y2 = ptPr.y + dx;
        Ellipse(x1, y1, x2, y2, color, width, true);
        return;
    }
    case MODES::RECT:
    {
        int dx = abs(x - ptPr.x);
        int dy = abs(y - ptPr.y);
        int x1 = ptPr.x - dx;
        int y1 = ptPr.y - dy;
        int x2 = ptPr.x + dx;
        int y2 = ptPr.y + dx;
        Rect(x1, y1, x2, y2, color, width, true);
        return;
    }
    default:
        return;
    }
}

void OnMouseMove(int x, int y)
{
    switch (mode) {
    case MODES::PEN:
    {
        if (mouse_down)
        {
            int x1 = ptPr.x, y1 = ptPr.y;
            ptPr.x = x; ptPr.y = y;
            Line(x1, y1, x, y, color, width, true);
        }
        return;
    }
    case MODES::ERASER:
    {
        if (mouse_down)
        {
            int x1 = ptPr.x, y1 = ptPr.y;
            ptPr.x = x; ptPr.y = y;
            Line(x1, y1, x, y, RGB(255, 255, 255), width, true);
        }
        return;
    }
    case MODES::FILL:
    {
        return;
    }
    case MODES::GRADIENT:
    {
        return;
    }
    case MODES::ELLIPSE:
    {
        return;
    }
    case MODES::RECT:
    {
        return;
    }
    default:
        return;
    }
}

void Line(int x1, int y1, int x2, int y2, COLORREF color, int width, bool needMem) {
    if (needMem) actions.push_back({ ACTIONTYPE::LINE, x1, y1, x2, y2, color, width });
    HDC hdc = GetDC(hwnd);
    HPEN pen = CreatePen(PS_SOLID, width, color);
    SelectObject(hdc, pen);
    MoveToEx(hdc, x1, y1, NULL);
    LineTo(hdc, x2, y2);
    DeleteObject(pen);
    ReleaseDC(hwnd, hdc);
}

void Ellipse(int x1, int y1, int x2, int y2, COLORREF color, int width, bool needMem) {
    if (needMem) actions.push_back({ ACTIONTYPE::ELLIPSE, x1, y1, x2, y2, color, width });
    HDC hdc = GetDC(hwnd);
    HPEN pen = CreatePen(PS_SOLID, width, color);
    SelectObject(hdc, pen);
    Ellipse(hdc, x1, y1, x2, y2);
    DeleteObject(pen);
    ReleaseDC(hwnd, hdc);
}

void Rect(int x1, int y1, int x2, int y2, COLORREF color, int width, bool needMem) {
    if (needMem) actions.push_back({ ACTIONTYPE::RECT, x1, y1, x2, y2, color, width });
    HDC hdc = GetDC(hwnd);
    HPEN pen = CreatePen(PS_SOLID, width, color);
    SelectObject(hdc, pen);
    Rectangle(hdc, x1, y1, x2, y2);
    DeleteObject(pen);
    ReleaseDC(hwnd, hdc);
}

void Fill(int x, int y, COLORREF color, bool needMem) {
    if (needMem) actions.push_back({ ACTIONTYPE::FILL, x, y, -1, -1, color, -1 });
    HDC hdc = GetDC(hwnd);
    HBRUSH br = CreateSolidBrush(color);
    SelectObject(hdc, br);
    ExtFloodFill(hdc, x, y, GetPixel(hdc, x, y), FLOODFILLSURFACE);
    DeleteObject(br);
    ReleaseDC(hwnd, hdc);
}

void GFill(int x, int y, bool needMem)
{
    if (needMem) actions.push_back({ ACTIONTYPE::GFILL, x, y, -1, -1, RGB(0, 0, 0), -1 });
    HDC hdc = GetDC(hwnd);
    int sx = x, sy = y;
    COLORREF old_color = GetPixel(hdc, x, y);
    queue< pair<int, int> > q;
    q.push({ x,y });
    while (!q.empty())
    {
        //
        pair<int, int> p = q.front(); q.pop();
        x = p.first; y = p.second;
        if (x < 0 || x >= x_size) continue;
        if (y < 0 || y >= y_size) continue;
        COLORREF tmp_color = GetPixel(hdc, x, y);
        if (tmp_color == old_color) {
            int t = abs((x - y) - (sx - sy)) % (52 + 51);
            if (t > 51) t = 51 * 2 - t;
            INT r = 0;
            INT g = t * 5;
            INT b = 255 - t * 5;
            //
            SetPixel(hdc, x, y, RGB(r, g, b));
            //
            q.push({ x - 1,y });
            q.push({ x,y - 1 });
            q.push({ x + 1,y });
            q.push({ x,y + 1 });
        }
        //
    }
    ReleaseDC(hwnd, hdc);
}

DWORD WINAPI fillGradient(LPVOID lpParam) {
    THREAD_PARAMS* params = (THREAD_PARAMS*)lpParam;

    int x = params->x;
    int y = params->y;
    bool needMem = params->memo;

    if (needMem) actions.push_back({ ACTIONTYPE::GFILL, x, y, -1, -1, RGB(0, 0, 0), -1 });
    HDC hdc = GetDC(hwnd);
    int sx = x, sy = y;
    COLORREF old_color = GetPixel(hdc, x, y);
    queue< pair<int, int> > q;
    q.push({ x,y });
    while (!q.empty())
    {
        //
        pair<int, int> p = q.front(); q.pop();
        x = p.first; y = p.second;
        if (x < 0 || x >= x_size) continue;
        if (y < 0 || y >= y_size) continue;
        COLORREF tmp_color = GetPixel(hdc, x, y);
        if (tmp_color == old_color) {
            int t = abs((x - y) - (sx - sy)) % (52 + 51);
            if (t > 51) t = 51 * 2 - t;
            INT r = 0;
            INT g = t * 5;
            INT b = 255 - t * 5;
            //
            SetPixel(hdc, x, y, RGB(g, r, b));
            //
            q.push({ x - 1,y });
            q.push({ x,y - 1 });
            q.push({ x + 1,y });
            q.push({ x,y + 1 });
        }
        //
    }
    ReleaseDC(hwnd, hdc);
    return 0;
}
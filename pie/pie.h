#define UNICODE
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>
#include <windows.h>
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#define CLASSNAME L"main"
#define WINDOWNAME L"gdi32 Pie Demo"
#define REDRAW_TIMER_ID 1
#define REDRAW_TIMER_INTERVAL 50
#define STAGECOUNT 100
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

int count = 0;
double rad = 0;
int sin_multiplier;
int direction;
const WCHAR* note;

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT WndProc(HWND, UINT, WPARAM, LPARAM);
void DrawFrame(HWND, HDC);

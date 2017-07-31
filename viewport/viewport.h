#define UNICODE
#include <io.h>
#include <atomic>
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>
#include <windows.h>
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#include "../shared/gc.h"
#include "../shared/util.h"

#define CLASSNAME L"main"
#define WINDOWNAME L"gdi32 Viewport Demo"
#define REDRAW_TIMER_ID 1
#define REDRAW_TIMER_INTERVAL 50
#define FRAMES_PER_STAGE 100
#define MINIMUM_PENDING_DURATION 3000
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

GarbageCollector gc;

int count = 0;
int stage = 0;
double rad = 0;

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT WndProc(HWND, UINT, WPARAM, LPARAM);
void DrawFrame(HWND, HDC);

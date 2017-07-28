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
#include "../shared/dib.h"
#include "../shared/util.h"
#include "../shared/net.h"

#define CLASSNAME L"main"
#define WINDOWNAME L"gdi32 Transform Demo"
#define REDRAW_TIMER_ID 1
#define REDRAW_TIMER_INTERVAL 50
#define FRAMES_PER_STAGE 100
#define MINIMUM_PENDING_DURATION 3000
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define PENDING_CIRCUIT_SPEED_MULTIPLIER 4

#define IMAGE_URL L"https://pbs.twimg.com/media/DChqXOKUwAETQiU.jpg"
#define IMAGE_CACHE_NAME L"cache.jpg"

GarbageCollector gc;

std::shared_ptr<DIB> dib;
enum DIBLOADSTATUS { PENDING, SUCCEEDED, FAILED };
HANDLE dibLoadThreadHandle;
DWORD dibLoadThreadId;
std::atomic<DIBLOADSTATUS> dibLoadStatus = DIBLOADSTATUS::PENDING;
std::atomic<const WCHAR*> dibLoadFailedReason = NULL;

int count = 0;
int stage = 0;
double rad = 0;

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT WndProc(HWND, UINT, WPARAM, LPARAM);
void DrawFrame(HWND, HDC);

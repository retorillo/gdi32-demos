#include "viewport.h"

#define HUD_TEXT L"SetViewPortOrgEx can simplify coordinates as follows:\
\nRECT = { -S, -S, +S, +S }\
\nORG  = { W + cos(%ddeg), H + sin(%ddeg) }\
\n * S = (square width / 2)\
\n * W = (client width / 2)\
\n * H = (client height / 2)"

int WinMain(HINSTANCE i, HINSTANCE p, LPSTR c, int n) {
  CoInitializeEx(NULL, COINIT_MULTITHREADED);
  
  WNDCLASSEX cls;
  memset(&cls, 0, sizeof(WNDCLASSEX));
  cls.cbSize = sizeof(WNDCLASSEX);
  cls.lpfnWndProc = reinterpret_cast<WNDPROC>(WndProc);
  cls.hInstance = i;
  cls.lpszClassName = CLASSNAME;
  ATOM atom = RegisterClassEx(&cls);
  if (!atom)
    return 1;
  
  POINT cursor;
  if (!GetCursorPos(&cursor))
    return 2;
  HWND hwnd = CreateWindowEx(NULL, reinterpret_cast<LPCWSTR>(atom), WINDOWNAME, 
    WS_OVERLAPPEDWINDOW, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, i, NULL);
  if (!hwnd)
    return 3;
  
  ShowWindow(hwnd, n);
  SetTimer(hwnd, REDRAW_TIMER_ID, REDRAW_TIMER_INTERVAL, NULL);

  MSG m;
  BOOL r;
  while (0 != (r = GetMessage(&m, NULL, 0, 0))) {
    if (r == -1)
        break;
    TranslateMessage(&m);
    DispatchMessage(&m);
  }
  return 0;
}
LRESULT WndProc(HWND hwnd, UINT m, WPARAM w, LPARAM l) {
  switch(m) {
    case WM_CREATE:
      return 0;
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);
      DrawFrame(hwnd, hdc);
      EndPaint(hwnd, &ps);
      } return 0;
    case WM_TIMER: {
      ::count++;
      double c = ::count % FRAMES_PER_STAGE;
      ::rad = (M_PI * 2) * (c / FRAMES_PER_STAGE);
      ::stage = static_cast<int>(floor((double)count / FRAMES_PER_STAGE));
      InvalidateRect(hwnd, NULL, false);
      UpdateWindow(hwnd);
      } return 0;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
  }
  return DefWindowProc(hwnd, m, w, l);
}
void DrawFrame(HWND hwnd, HDC hdc) {
  if (count == 0) return;
  RECT clientRect;
  GetClientRect(hwnd, &clientRect);
  HDC backdc = CreateCompatibleDC(hdc);
  DPtoLP(hdc, reinterpret_cast<POINT*>(&clientRect), 2);

  int cw = clientRect.right - clientRect.left;
  int ch = clientRect.bottom - clientRect.top;
  RECT backrt = { 0, 0, cw, ch };

  HBITMAP backbmp = CreateCompatibleBitmap(hdc, cw, ch);
  HGDIOBJ backoldbmp = SelectObject(backdc, reinterpret_cast<HGDIOBJ>(backbmp));
  FillRect(backdc, &backrt, reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));

  double squareSize = static_cast<double>(min(cw, ch)) * 0.5;
  RECT squareRect;
  squareRect.left = static_cast<int>(round(-squareSize / 2));
  squareRect.top = static_cast<int>(round(-squareSize / 2));
  squareRect.right = static_cast<int>(round(squareSize / 2));
  squareRect.bottom = static_cast<int>(round(squareSize / 2));
  HBRUSH squareBrush = CreateSolidBrush(RGB(240, 80, 250));

  SetGraphicsMode(backdc, GM_ADVANCED);
  SetStretchBltMode(backdc, HALFTONE);
  SetBrushOrgEx(backdc, 0, 0, NULL);

  SetViewportOrgEx(backdc,
    static_cast<int>(round(cw / 2 + cos(rad) * cw / 4)),
    static_cast<int>(round(ch / 2 + sin(rad) * ch / 4)), NULL);

  FillRect(backdc, &squareRect, squareBrush);
  DeleteObject(squareBrush);

  const WCHAR* textout = L"TextOut at (0, 0)";
  TextOut(backdc, 0, 0, textout, wcslen(textout));

  SetViewportOrgEx(backdc, 0, 0, NULL);

  SIZE viewportExt { 0, 0 };
  GetViewportExtEx(backdc, &viewportExt);
  WCHAR hud[256];
  int deg = static_cast<int>(round((rad / M_PI) * 180));
  wsprintf(hud, HUD_TEXT, deg, deg);
  DrawText(backdc, hud, -1, &clientRect, NULL);

  BitBlt(hdc, 0, 0, cw, ch, backdc, 0, 0, SRCCOPY);

  SelectObject(backdc, backoldbmp);
  DeleteObject(backbmp);
  DeleteDC(backdc);
}

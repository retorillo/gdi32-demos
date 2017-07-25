#include "pie.h"

int WinMain(HINSTANCE i, HINSTANCE p, LPSTR c, int n) {
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
  if (!hwnd) {
    return 3;
  }
  ShowWindow(hwnd, n);
  SetLayeredWindowAttributes(hwnd, NULL, NULL, NULL);
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
      count++;
      double c = count % STAGECOUNT;
      if (c == 0) { count++; c++; } // Skip when c = 0, because it is not pie but circle
      rad = (M_PI * 2) * (c / STAGECOUNT);
      switch (static_cast<int>(floor((double)count / STAGECOUNT)) % 3) {
        case 0: 
          direction = AD_CLOCKWISE;
          sin_multiplier = 1;
          note = L"(1/3) On clockwise, can use simply sin(rad) and cos(rad)";
          break;
        case 1: 
          direction = AD_COUNTERCLOCKWISE;
          sin_multiplier = 1;
          note = L"(2/3) On counterclockwise, note that fill region is inverted unexpectedly";
          break;
        case 2: 
          direction = AD_COUNTERCLOCKWISE;
          sin_multiplier = -1;
          note = L"(3/3) So, should be invert sin(rad) value on counterclockwise";
          break;
      }
      InvalidateRect(hwnd, NULL, true);
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

  HBRUSH brush = CreateSolidBrush(RGB(180, 0, 100));
  HBRUSH oldbrush = reinterpret_cast<HBRUSH>(SelectObject(hdc, brush));
  
  RECT clientRect;
  GetClientRect(hwnd, &clientRect);

  int cw = clientRect.right - clientRect.left;
  int ch = clientRect.bottom - clientRect.top;
  int r2 = min(cw, ch);
  int r = static_cast<int>(round(static_cast<double>(r2) / 2));

  RECT piRect;
  piRect.left = static_cast<int>(round(static_cast<double>(cw - r2) / 2));
  piRect.top = static_cast<int>(round(static_cast<double>(ch - r2) / 2));
  piRect.right = piRect.left + r2;
  piRect.bottom = piRect.top + r2;
  
  SetArcDirection(hdc, direction);
  FillRect(hdc, &clientRect, reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));

  Pie(hdc, piRect.left, piRect.top, piRect.right, piRect.bottom,
    round(cos(0) * r + r + piRect.left), 
    round(sin(0) * sin_multiplier * r + r + piRect.top), 
    round(cos(rad) * r + r + piRect.left), 
    round(sin(rad) * sin_multiplier * r + r + piRect.top));

  WCHAR hudtext[256];
  swprintf(hudtext, 256, L"%s\r\nDIR: %s\r\nCNT: %d\r\nRAD: %g\r\nCOS: %g\r\nSIN %g%s", 
    note, direction == AD_COUNTERCLOCKWISE ? L"COUNTERCLOCKWISE" : L"CLOCKWISE",
    count, rad, cos(rad), sin(rad), sin_multiplier == -1 ? L" * -1" : L"" );
  DrawText(hdc, hudtext, wcslen(hudtext), &clientRect, DT_WORDBREAK);

  SelectObject(hdc, reinterpret_cast<HGDIOBJ>(oldbrush));
  DeleteObject(reinterpret_cast<HGDIOBJ>(brush));
}

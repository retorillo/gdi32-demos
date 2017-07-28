#include "transform.h"

DWORD WINAPI ImageLoadThreadProc(LPVOID param) {
  try {
    auto imagepath = GetExecutableSiblingsPath(IMAGE_CACHE_NAME);
    if (_waccess(imagepath.get(), 0) == 0) {
      ::dibLoadFailedReason = L"Loading image from cache...";
      ::dib = DIB::fromFile(imagepath.get());
    }
    else {
      ::dibLoadFailedReason = L"Downloading image...";
      GarbageCollector gc;
      IStream* stream = download(IMAGE_URL, NULL);
      gc.push([&stream](){ stream->Release(); });  
      FILE* f = _wfopen(imagepath.get(), L"wb");
      const int buffersize = 1024;
      BYTE buffer[buffersize];
      ULONG readsize;
      do {
        HRESULT hr = stream->Read(reinterpret_cast<void*>(buffer), buffersize, &readsize);
        if (hr != S_OK && hr != S_FALSE)
          throw L"Faild to read IStream";
        size_t writesize = fwrite(reinterpret_cast<void*>(buffer), sizeof(BYTE), readsize, f);
        if (writesize != readsize)
          throw L"Failed to write";
      } while(readsize > 0);
      fclose(f);
      LARGE_INTEGER zero;
      memset(&zero, 0, sizeof(LARGE_INTEGER));
      stream->Seek(zero, STREAM_SEEK_SET, reinterpret_cast<ULARGE_INTEGER*>(NULL));
      ::dib = DIB::fromStream(stream);
    }
    ::dibLoadStatus = DIBLOADSTATUS::SUCCEEDED;
    return true;
  }
  catch (const WCHAR* reason) {
    ::dibLoadStatus = DIBLOADSTATUS::FAILED;
    ::dibLoadFailedReason = reason;
    return false;
  }
}

int WinMain(HINSTANCE i, HINSTANCE p, LPSTR c, int n) {
  CoInitializeEx(NULL, COINIT_MULTITHREADED);
  
  dibLoadThreadHandle = CreateThread(NULL, NULL, ImageLoadThreadProc,
    NULL , NULL, &dibLoadThreadId);
  
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
  if (!dibLoadThreadHandle)
    throw L"Failed to create thread.";
  
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

  double cosval = cos(rad);
  double sinval = sin(rad);
  XFORM translate { 1, 0, 0, 1, 
    - static_cast<double>(cw) / 2.0, 
    - static_cast<double>(ch) / 2.0};
  XFORM rotate { cosval, sinval, -sinval, cosval, 
    static_cast<double>(cw) / 2.0, 
    static_cast<double>(ch) / 2.0};
  XFORM combined;
  
  SetGraphicsMode(backdc, GM_ADVANCED);
  CombineTransform(&combined, &translate, &rotate);

  SetStretchBltMode(backdc, HALFTONE);
  SetBrushOrgEx(backdc, 0, 0, NULL);

  bool forcePending = count < MINIMUM_PENDING_DURATION / REDRAW_TIMER_INTERVAL;
  switch (forcePending ? DIBLOADSTATUS::PENDING : ::dibLoadStatus) {
  case DIBLOADSTATUS::PENDING: {
    auto diameter = static_cast<double>(min(cw, ch)) * 0.2;
    auto penwidth = static_cast<int>(max(1, round(diameter * 0.1)));
    auto radius = diameter / 2;
    auto arcleft = (cw - diameter) / 2;
    auto arctop = (ch - diameter) / 2;
    HPEN pen = CreatePen(PS_SOLID, penwidth, RGB(255, 255, 255));
    HGDIOBJ initialpen = SelectObject(backdc, pen);

    int d = static_cast<int>(round(rad / (M_PI * 2) * 360));
    auto md = d * PENDING_CIRCUIT_SPEED_MULTIPLIER;
    double R = static_cast<double>(md % 360) / 360 * (M_PI * 2);

    double r1, r2;
    if (R > M_PI) { r1 = 0; r2 = R * 2; }
    else { r1 = R * 2; r2 = 0; }
    r1 += R; r2 += R;
    if (0 < R && R < M_PI * 2)
      Arc(backdc,
        static_cast<int>(round(arcleft)),
        static_cast<int>(round(arctop)),
        static_cast<int>(round(arcleft + diameter)),
        static_cast<int>(round(arctop + diameter)),
        static_cast<int>(round(cos(r1) * radius + radius + arcleft)),
        static_cast<int>(round(sin(r1) * radius + radius + arctop)),
        static_cast<int>(round(cos(r2) * radius + radius + arcleft)),
        static_cast<int>(round(sin(r2) * radius + radius + arctop))
      );
    SelectObject(backdc, initialpen);
    DeleteObject(pen);

    RECT textRect { 0, arctop + diameter, cw, ch };
    SetTextColor(backdc, RGB(255, 255, 255));
    SetBkColor(backdc, RGB(0, 0, 0));
    DrawText(backdc, dibLoadFailedReason, -1, &textRect,
      DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    } break;
  case DIBLOADSTATUS::FAILED: {
    RECT textRect { 0, 0, cw, ch };
    DrawText(backdc, dibLoadFailedReason, -1, &textRect,
      DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    } break;
    break;
  case DIBLOADSTATUS::SUCCEEDED: {
    SetWorldTransform(backdc, &combined);
    auto imageWidth = abs(::dib.get()->info.bmiHeader.biWidth);
    auto imageHeight = abs(::dib.get()->info.bmiHeader.biHeight);
    double imageAspect = static_cast<double>(imageHeight) / imageWidth;
    auto drawWidth = static_cast<double>(min(cw, ch));
    auto drawHeight = drawWidth * imageAspect;
    if (drawHeight > ch) {
      drawHeight = ch;
      drawWidth = drawHeight / imageAspect;
    }
    StretchDIBits(backdc, 
      static_cast<int>(round((cw - drawWidth) / 2)),
      static_cast<int>(round((ch - drawHeight) / 2)),
      static_cast<int>(round(drawWidth)),
      static_cast<int>(round(drawHeight)),
      0, 0, imageWidth, imageHeight,
      ::dib.get()->bits.get(), 
      &::dib.get()->info, 
      DIB_RGB_COLORS, 
      SRCCOPY);
    } break;
  }

  // NOTE: Reset source's world transform, otherwise BitBlt will fail
  ModifyWorldTransform(backdc, NULL, MWT_IDENTITY);

  BitBlt(hdc, 0, 0, cw, ch, backdc, 0, 0, SRCCOPY);

  SelectObject(backdc, backoldbmp);
  DeleteObject(backbmp);
  DeleteDC(backdc);
}

#ifndef _INCLUDE_DIB
#define _INCLUDE_DIB

#define UNICODE
#include <memory>
#include <utility>
#include <functional>
#include <windows.h>
#pragma comment(lib, "gdi32.lib")
#include <wincodec.h>
#pragma comment(lib, "windowscodecs.lib")
#define CREATEWICDECODER std::function<HRESULT(IWICImagingFactory*, IWICBitmapDecoder**)>

#include "net.h"

struct DIB {
public:
  BITMAPINFO info;
  std::shared_ptr<BYTE> bits;
  DIB();
  ~DIB();
  static std::shared_ptr<DIB> from(CREATEWICDECODER);
  static std::shared_ptr<DIB> fromFile(const WCHAR*);
  static std::shared_ptr<DIB> fromUrl(const WCHAR*, const WCHAR*);
  static std::shared_ptr<DIB> fromStream(IStream*);
};

#endif

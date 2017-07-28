#include "dib.h"
DIB::DIB() {
  memset(reinterpret_cast<BYTE*>(&info), 0, sizeof(info));
  info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
}
DIB::~DIB() {
}
std::shared_ptr<DIB> DIB::from(CREATEWICDECODER createDecoder) {
  std::shared_ptr<DIB> dib(new DIB());
  GarbageCollector gc;
  IWICImagingFactory* factory;
  HRESULT hr;
  hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, 
    IID_PPV_ARGS(&factory));
  if (hr != S_OK)
    throw L"Failed to initialize Windows Imaging Component";
  gc.push([factory](){ factory->Release(); });
  IWICBitmapDecoder* decoder;
  hr = createDecoder(factory, &decoder);
  if (hr != S_OK)
    throw L"Failed to decode image file";
  gc.push([decoder](){ decoder->Release(); });
  IWICBitmapFrameDecode* frame;
  hr = decoder->GetFrame(0, &frame);
  if (hr != S_OK)
    throw L"Failed to load frame";
  gc.push([frame]() { frame->Release(); });
  IWICFormatConverter* converter;
  hr = factory->CreateFormatConverter(&converter);
  if (hr != S_OK)
    throw L"Failed to create converter";
  gc.push([converter]() { converter->Release(); });
  converter->Initialize(frame, 
    GUID_WICPixelFormat32bppBGRA,
    WICBitmapDitherTypeNone,
    NULL, 0.f,
    WICBitmapPaletteTypeMedianCut);
  UINT w, h, size;
  frame->GetSize(&w, &h);
  size = w * h * 4;
  dib->bits.reset(new BYTE[w * h * 4]);
  dib->info.bmiHeader.biWidth = w;
  dib->info.bmiHeader.biHeight = -h;
  dib->info.bmiHeader.biPlanes = 1;
  dib->info.bmiHeader.biBitCount = 32;
  dib->info.bmiHeader.biCompression = BI_RGB;
  hr = converter->CopyPixels(0, w * 4, size, dib->bits.get());
  if (hr != S_OK)
    throw L"Failed to copy pixels";
  GdiFlush();
  return dib;

}
std::shared_ptr<DIB> DIB::fromFile(const WCHAR* filename) {
  return DIB::from([filename](IWICImagingFactory* factory, IWICBitmapDecoder** decoder) {
    return factory->CreateDecoderFromFilename(filename, 
      NULL, GENERIC_READ, static_cast<WICDecodeOptions>(NULL),
      decoder);
  });
}
std::shared_ptr<DIB> DIB::fromStream(IStream* stream) {
  return DIB::from([stream](IWICImagingFactory* factory, IWICBitmapDecoder** decoder) {
    return factory->CreateDecoderFromStream(stream, 
      NULL, static_cast<WICDecodeOptions>(NULL), decoder);
  });
}
std::shared_ptr<DIB> DIB::fromUrl(const WCHAR* url, const WCHAR* referer) {
  IStream* stream = download(url, referer);
  return DIB::from([&stream](IWICImagingFactory* factory, IWICBitmapDecoder** decoder) {
    HRESULT hr = factory->CreateDecoderFromStream(stream, 
      NULL, static_cast<WICDecodeOptions>(NULL), decoder);
    stream->Release();
    return hr;
  });
}

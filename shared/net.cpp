#include "net.h"

IStream* download(const WCHAR* url, const WCHAR* referer) {
  GarbageCollector gc;
  HINTERNET session = WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
    WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, NULL);
  if (!session)
    throw L"Failed to open session";
  gc.push([session](){ WinHttpCloseHandle(session); });
  WinHttpSetOption(session, WINHTTP_OPTION_REDIRECT_POLICY,
    reinterpret_cast<void*>(WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS), NULL);
  URL_COMPONENTS urlc;
  memset(reinterpret_cast<BYTE*>(&urlc), 0, sizeof(URL_COMPONENTS));
  urlc.dwStructSize = sizeof(URL_COMPONENTS);
  urlc.dwHostNameLength = -1;
  urlc.dwUrlPathLength = -1;
  WinHttpCrackUrl(url, NULL, NULL, &urlc);
  if (urlc.nScheme != INTERNET_SCHEME_HTTP && 
      urlc.nScheme != INTERNET_SCHEME_HTTPS)
    throw L"Supports only HTTP";
  std::unique_ptr<WCHAR> hostname(new WCHAR[urlc.dwHostNameLength + 1]);
  hostname.get()[urlc.dwHostNameLength] = L'\0';
  memcpy(hostname.get(), urlc.lpszHostName, sizeof(WCHAR) * urlc.dwHostNameLength);
  std::unique_ptr<WCHAR> urlpath(new WCHAR[urlc.dwUrlPathLength + 1]);
  urlpath.get()[urlc.dwUrlPathLength] = L'\0';
  memcpy(urlpath.get(), urlc.lpszUrlPath, sizeof(WCHAR) * urlc.dwUrlPathLength);
  HINTERNET connect = WinHttpConnect(session, hostname.get(), urlc.nPort, NULL);
  if (!connect)
    throw L"Failed to connect host";
  gc.push([connect]() { WinHttpCloseHandle(connect); });
  const WCHAR* acceptTypes [] = { L"*", NULL };
  DWORD requestFlags = NULL;
  if (urlc.nScheme == INTERNET_SCHEME_HTTPS)
    requestFlags |= WINHTTP_FLAG_SECURE;
  HINTERNET request = WinHttpOpenRequest(connect, NULL, urlpath.get(),
    NULL, referer ? referer : WINHTTP_NO_REFERER, 
    acceptTypes, requestFlags);
  if (!request)
    throw L"Failed to create request";
  gc.push([request]() { WinHttpCloseHandle(request); });
  BOOL successRequest = WinHttpSendRequest(request, 
    WINHTTP_NO_ADDITIONAL_HEADERS,
    NULL, WINHTTP_NO_REQUEST_DATA, NULL, NULL, NULL);
  if (!successRequest)
    throw L"Failed to send request";
  BOOL successResponse = WinHttpReceiveResponse(request, NULL);
  if (!successResponse)
    throw L"Failed to receive response";
  IStream* stream = SHCreateMemStream(NULL, NULL);
  DWORD size, read;
  do {
    BOOL successQuery = WinHttpQueryDataAvailable(request, &size);
    if (!successQuery)
      throw L"Failed to query data avaiable";
    std::unique_ptr<BYTE> buffer(new BYTE[size]);
    WinHttpReadData(request, reinterpret_cast<void*>(buffer.get()), size, &read);
    stream->Write(buffer.get(), read, NULL);
  } while (size > 0);
  LARGE_INTEGER zero;
  memset(reinterpret_cast<BYTE*>(&zero), 0, sizeof(LARGE_INTEGER));
  stream->Seek(zero, STREAM_SEEK_SET, reinterpret_cast<ULARGE_INTEGER*>(NULL));
  return stream;
}

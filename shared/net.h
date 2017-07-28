#define UNICODE
#include <memory>
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "gc.h"

IStream* download(const WCHAR*, const WCHAR*);

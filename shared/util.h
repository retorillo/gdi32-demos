#define UNICODE
#include <windows.h>
#include <memory>
#include <pathcch.h>
#pragma comment(lib, "pathcch.lib")

std::shared_ptr<WCHAR> GetExecutableSiblingsPath(const WCHAR* rel);
std::shared_ptr<WCHAR> GetExecutablePath();

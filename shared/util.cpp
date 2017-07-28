#include "util.h"

std::shared_ptr<WCHAR> GetExecutableSiblingsPath(const WCHAR* rel) {
  std::shared_ptr<WCHAR> path = GetExecutablePath();
  PathCchRemoveFileSpec(path.get(), wcslen(path.get()));
  WCHAR* sibling;
  PathAllocCombine(path.get(), rel, PATHCCH_ALLOW_LONG_PATHS, &sibling);
  int len = wcslen(sibling) + 1;
  std::shared_ptr<WCHAR> result(new WCHAR[len]);
  memcpy(result.get(), sibling, sizeof(WCHAR) * len);
  LocalFree(sibling);
  return result;
}
std::shared_ptr<WCHAR> GetExecutablePath() {
  WCHAR* filename = NULL;
  for (unsigned int c = 256, n = c; c < INT_MAX; c <<= 1) {
    if (filename) delete[] filename;
    filename = new TCHAR[c];
    n = GetModuleFileName(NULL, filename, c);
    if (c != n)
      break;
  }
  return std::shared_ptr<WCHAR>(filename);
}

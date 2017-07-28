#ifndef __INCLUDE_GC
#define __INCLUDE_GC

#include <vector>
#include <functional>

#define DISPOSEACTION std::function<void(void)>

class GarbageCollector {
private:
  std::vector<DISPOSEACTION> actions;
public:
  ~GarbageCollector();
  void push(DISPOSEACTION action);
};

#endif

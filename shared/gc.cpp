#include "gc.h"

GarbageCollector::~GarbageCollector() {
  for (auto action : this->actions)
    action();
}
void GarbageCollector::push(DISPOSEACTION action) {
  this->actions.push_back(action);
}

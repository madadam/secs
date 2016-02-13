#include "component_store.h"

using namespace secs;

void ComponentStore::erase(size_t index) {
  if (!contains(index)) return;

  _deleter(_data, index);
  _flags[index] = false;
}

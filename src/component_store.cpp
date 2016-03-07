#include "secs/component_store.h"
#include "secs/entity.h"

using namespace secs;

void ComponentStore::erase(size_t index) {
  if (!contains(index)) return;
  _destroy(_data.get(), index);
  _flags[index] = false;
}

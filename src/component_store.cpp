#include "secs/component_store.h"

using namespace secs;

void ComponentStore::erase(size_t index) {
  if (!contains(index)) return;

  _ops->erase(_data.get(), index);
  _flags[index] = false;
}

void ComponentStore::copy( size_t     source_index
                         , Container& target_container
                         , size_t     target_index)
{
  if (!contains(source_index)) return;
  _ops->copy(_data.get(), source_index, target_container, target_index);
}

void ComponentStore::move( size_t     source_index
                         , Container& target_container
                         , size_t     target_index)
{
  if (!contains(source_index)) return;
  _ops->move(_data.get(), source_index, target_container, target_index);
  _flags[source_index] = false;
}

#include "secs/container.h"

using namespace secs;

ComponentStore Container::_empty_store;

Entity Container::create() {
  size_t index;

  if (_holes.empty()) {
    index = _capacity++;
  } else {
    index = _holes.back();
    _holes.pop_back();
  }

  if (index >= _versions.size()) {
    _versions.resize(index + 1);
  }

  return Entity(*this, index, ++_versions[index]);
}

void Container::destroy(const Entity& entity) {
  for (auto& store : _stores) {
    store.erase(entity._index);
  }

  _holes.push_back(entity._index);
  ++_versions[entity._index];
}

void Container::copy( size_t     source_index
                    , Container& target_container
                    , size_t     target_index)
{
  assert(get(source_index));
  assert(target_container.get(target_index));

  for (auto& store : _stores) {
    store.copy(source_index, target_container, target_index);
  }
}

void Container::move( size_t     source_index
                    , Container& target_container
                    , size_t     target_index)
{
  assert(get(source_index));
  assert(target_container.get(target_index));

  for (auto& store : _stores) {
    store.move(source_index, target_container, target_index);
  }
}
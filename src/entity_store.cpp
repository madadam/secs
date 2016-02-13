#include "entity_store.h"

using namespace secs;

ComponentStore EntityStore::_empty_store;

Entity EntityStore::create() {
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

void EntityStore::destroy(const Entity& entity) {
  for (auto& store : _stores) {
    store.erase(entity._index);
  }

  _holes.push_back(entity._index);
  ++_versions[entity._index];
}

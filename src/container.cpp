#include "secs/container.h"

using namespace secs;

Container::~Container() {
  for (auto e : entities()) {
    e.destroy();
  }
}

Entity Container::create() {
  size_t index;

  if (_holes.empty()) {
    index = _capacity++;
  } else {
    index = _holes.back();
    _holes.pop_back();
  }

  if (index >= _meta.size()) {
    _meta.resize(index + 1);
  }

  _meta[index].version.create();

  return Entity(*this, index, _meta[index].version);
}

void Container::destroy(const Entity& entity) {
  for (auto& ops : _ops) {
    ops.destroy(entity);
  }

  _holes.push_back(entity._index);
  _meta[entity._index].version.destroy();
  _meta[entity._index].signals = {};
}

void Container::copy(const Entity& source, const Entity& target) {
  assert(source._container == this);

  for (auto& ops : _ops) {
    ops.copy(source, target);
  }
}

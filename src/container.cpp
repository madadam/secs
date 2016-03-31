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

  if (index >= _versions.size()) {
    _versions.resize(index + 1);
  }

  _versions[index].create();

  return Entity(*this, index, _versions[index]);
}

void Container::destroy(const Entity& entity) {
  for (auto& ops : _ops) {
    ops.destroy(entity);
  }

  _holes.push_back(entity._index);
  _versions[entity._index].destroy();
}

void Container::copy(const Entity& source, const Entity& target) {
  assert(source._container == this);

  for (auto& ops : _ops) {
    ops.copy(source, target);
  }
}

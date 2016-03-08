#include "secs/container.h"

using namespace secs;

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
  for (auto& ops : _ops) {
    ops.destroy(entity);
  }

  after_destroy(entity._index);
}

void Container::copy(const Entity& source, const Entity& target) {
  assert(source._container == this);

  for (auto& ops : _ops) {
    ops.copy(source, target);
  }
}

void Container::move(const Entity& source, const Entity& target) {
  assert(source._container == this);

  for (auto& ops : _ops) {
    ops.move(source, target);
  }

  after_destroy(source._index);
}

void Container::after_destroy(size_t index) {
  _holes.push_back(index);
  ++_versions[index];
}
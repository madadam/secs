#include "environment.h"

ComponentStore Environment::_empty_store;

Entity Environment::create_entity() {
  if (_holes.empty()) {
    ++_entity_capacity;
    return Entity(*this, _entity_capacity - 1);
  } else {
    auto index = _holes.back();
    _holes.pop_back();
    return Entity(*this, index);
  }
}

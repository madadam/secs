#pragma once

#include "secs/component_ptr.h"
#include "secs/entity.h"

namespace secs {

// Event emitted after a Component of type T is created.
template<typename T> struct OnCreate {
  const Entity          entity;
  const ComponentPtr<T> component;
};

// Event emitted before a Component of type T is destroyed.
template<typename T> struct OnDestroy {
  const Entity          entity;
  const ComponentPtr<T> component;
};

} // namespace secs
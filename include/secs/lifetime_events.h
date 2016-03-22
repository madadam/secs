#pragma once

#include "secs/component_ptr.h"
#include "secs/entity.h"

namespace secs {

template<typename T>
struct CreateEvent {
  const Entity          entity;
  const ComponentPtr<T> component;
};

template<typename T>
struct DestroyEvent {
  const Entity          entity;
  const ComponentPtr<T> component;
};

}
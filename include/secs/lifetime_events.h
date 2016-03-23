#pragma once

#include "secs/component_ptr.h"
#include "secs/entity.h"

namespace secs {

template<typename T>
struct BeforeCreate {
  const Entity entity;
};

template<typename T>
struct AfterCreate {
  const Entity          entity;
  const ComponentPtr<T> component;
};

template<typename T>
struct BeforeDestroy {
  const Entity          entity;
  const ComponentPtr<T> component;
};

template<typename T>
struct AfterDestroy {
  const Entity entity;
};

}
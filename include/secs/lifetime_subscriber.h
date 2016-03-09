#pragma once

#include "secs/component_ptr.h"
#include "secs/event.h"

namespace secs {

class Entity;

template<typename T>
struct OnCreate {
  const Entity          entity    = nullptr;
  const ComponentPtr<T> component = nullptr;
};

template<typename T>
struct OnDestroy {
  const Entity          entity    = nullptr;
  const ComponentPtr<T> component = nullptr;
};

template<typename T>
class LifetimeSubscriber : public Subscriber<OnCreate<T>>
                         , public Subscriber<OnDestroy<T>>
{
public:

  virtual void on_create (Entity, ComponentPtr<T>) {};
  virtual void on_destroy(Entity, ComponentPtr<T>) {};

  void receive(OnCreate<T>  event) override {
    on_create (event.entity, event.component);
  }

  void receive(OnDestroy<T> event) override {
    on_destroy(event.entity, event.component);
  }
};

} // namespace secs
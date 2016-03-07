#pragma once

#include "secs/component_ptr.h"
#include "secs/event.h"

namespace secs {

template<typename T>
struct OnCreate {
  ComponentPtr<T> component = nullptr;
};

template<typename T>
struct OnDestroy {
  ComponentPtr<T> component = nullptr;
};

template<typename T>
class LifetimeSubscriber : public Subscriber<OnCreate<T>>
                         , public Subscriber<OnDestroy<T>>
{
public:

  virtual void on_create (ComponentPtr<T>) {};
  virtual void on_destroy(ComponentPtr<T>) {};

  void receive(OnCreate<T>  event) override { on_create (event.component); }
  void receive(OnDestroy<T> event) override { on_destroy(event.component); }
};

} // namespace secs
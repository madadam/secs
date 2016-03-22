#pragma once

#include "secs/component_ptr.h"
#include "secs/entity.h"
#include "secs/event_manager.h"

namespace secs {

template<typename T>
struct CreateEvent {
  Entity          entity;
  ComponentPtr<T> component;
};

template<typename T>
struct DestroyEvent {
  Entity          entity;
  ComponentPtr<T> component;
};

template<typename T>
class LifetimeSubscriber : public Subscriber<CreateEvent<T>>
                         , public Subscriber<DestroyEvent<T>>
{
public:

  virtual void on_create (const Entity&, const ComponentPtr<T>&) {}
  virtual void on_destroy(const Entity&, const ComponentPtr<T>&) {}

  void receive(const CreateEvent<T>& event) {
    on_create(event.entity, event.component);
  }

  void receive(const DestroyEvent<T>& event) {
    on_destroy(event.entity, event.component);
  }
};

}
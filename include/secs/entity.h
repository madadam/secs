#pragma once

#include "secs/handle.h"

namespace secs {

template<typename> class ComponentPtr;
class Container;

class Entity : public Handle<Entity> {
  using Handle::Handle;

public:
  explicit operator bool () const {
    return valid();
  }

  template<typename T> ComponentPtr<T> component() const;

  Entity copy();
  Entity copy_to(Container&);
  Entity move_to(Container&);
  void destroy() const;

private:
  friend class Container;
};

} // namespace secs

#include "container.inline.h"
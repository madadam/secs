#pragma once

#include <vector>
#include "secs/omniset.h"

namespace secs {

template<typename> class Publisher;
template<typename> class Subscriber;

class EventManager {
public:

  template<typename E>
  void subscribe(Subscriber<E>& subscriber) {
    _publishers.get<Publisher<E>>().subscribe(subscriber);
  }

  template<typename E>
  void unsubscribe(Subscriber<E>& subscriber) {
    _publishers.get<Publisher<E>>().unsubscribe(subscriber);
  }

  template<typename E>
  void emit(const E& event) const {
    _publishers.get<Publisher<E>>().emit(event);
  }

private:

  Omniset _publishers;
};

template<typename E>
class Subscriber {
public:
  Subscriber() = default;

  Subscriber(const Subscriber<E>& other)
    : _publisher(nullptr)
    , _index(0)
  {
    if (other._publisher) {
      other._publisher->subscribe(*this);
    }
  }

  Subscriber(Subscriber<E>&& other)
    : _publisher(other._publisher)
    , _index(other._index)
  {
    if (_publisher) {
      _publisher->_subscribers[_index] = this;
    }
  }

  virtual ~Subscriber() {
    if (_publisher) {
      _publisher->unsubscribe(*this);
    }
  }

  Subscriber<E>& operator = (const Subscriber<E>& other) {
    if (&other == this) {
      return *this;
    }

    if (_publisher) {
      _publisher->unsubscribe(*this);
      _publisher = nullptr;
    }

    if (other._publisher) {
      other._publisher->subscribe(*this);
    }

    return *this;
  }

  Subscriber<E>& operator = (Subscriber<E>&& other) {
    if (_publisher) {
      _publisher->unsubscribe(*this);
    }

    if (other._publisher) {
      other._publisher->subscribe(*this);
      other._publisher->unsubscribe(other);
    }

    return *this;
  }

  virtual void receive(const E&) {}

private:

  Publisher<E>* _publisher = nullptr;
  size_t        _index = 0;

  friend class Publisher<E>;
};

template<typename E>
class Publisher {
public:
  Publisher() = default;
  Publisher(const Publisher<E>&) = delete;

  Publisher(Publisher<E>&& other)
    : _subscribers(std::move(other._subscribers))
    , _holes(std::move(other._holes))
  {
    for (auto subscriber : _subscribers) {
      if (subscriber) subscriber->_publisher = this;
    }
  }

  ~Publisher() {
    for (auto subscriber : _subscribers) {
      if (subscriber) subscriber->_publisher = nullptr;
    }
  }

  Publisher<E>& operator = (const Publisher<E>&) = delete;

  Publisher<E>& operator = (Publisher<E>&& other) {
    _subscribers = std::move(other._subscribers);
    _holes       = std::move(other._holes);

    for (auto subscriber : _subscribers) {
      if (subscriber) subscriber->_publisher = this;
    }

    return *this;
  }

  void subscribe(Subscriber<E>& subscriber) {
    if (subscriber._publisher == this) return;

    if (subscriber._publisher) {
      subscriber._publisher->unsubscribe(subscriber);
    }

    subscriber._publisher = this;

    if (!_holes.empty()) {
      subscriber._index = _holes.back();
      _subscribers[_holes.back()] = &subscriber;
      _holes.pop_back();
    } else {
      subscriber._index = _subscribers.size();
      _subscribers.push_back(&subscriber);
    }
  }

  void unsubscribe(Subscriber<E>& subscriber) {
    if (subscriber._publisher != this) return;

    _subscribers[subscriber._index] = nullptr;
    _holes.push_back(subscriber._index);
    subscriber._publisher = nullptr;
  }

  void emit(const E& event) const {
    for (auto subscriber : _subscribers) {
      subscriber->receive(event);
    }
  }

private:

  std::vector<Subscriber<E>*> _subscribers;
  std::vector<size_t>         _holes;

  friend class Subscriber<E>;
};

} // namespace secs
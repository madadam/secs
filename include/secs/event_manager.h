#pragma once

#include <algorithm>
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
  Subscriber(const Subscriber<E>&) = delete;

  Subscriber(Subscriber<E>&& other)
    : _publisher(other._publisher)
  {
    if (_publisher) {
      auto it = std::find( _publisher->_subscribers.begin()
                         , _publisher->_subscribers.end()
                         , &other);

      if (it != _publisher->_subscribers.end()) {
        *it = this;
      }
    }
  }

  virtual ~Subscriber() {
    if (_publisher) {
      _publisher->unsubscribe(*this);
    }
  }

  Subscriber<E>& operator = (const Subscriber<E>&) = delete;

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
  friend class Publisher<E>;
};

template<typename E>
class Publisher {
public:
  Publisher() = default;
  Publisher(const Publisher<E>&) = delete;

  Publisher(Publisher<E>&& other)
    : _subscribers(std::move(other._subscribers))
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
    _subscribers.push_back(&subscriber);
  }

  void unsubscribe(Subscriber<E>& subscriber) {
    if (subscriber._publisher != this) return;

    auto it = std::find(_subscribers.begin(), _subscribers.end(), &subscriber);

    if (it != _subscribers.end()) {
      _subscribers.erase(it);
      subscriber._publisher = nullptr;
    }
  }

  void emit(const E& event) const {
    for (auto subscriber : _subscribers) {
      subscriber->receive(event);
    }
  }

private:
  std::vector<Subscriber<E>*> _subscribers;
  friend class Subscriber<E>;
};

} // namespace secs
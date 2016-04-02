#pragma once

#include <algorithm>
#include <vector>
#include "secs/omniset.h"

namespace secs {

template<typename> class Sender;
template<typename> class Receiver;

class EventManager {
public:

  template<typename E>
  void subscribe(Receiver<E>& receiver) {
    _senders.get<Sender<E>>().subscribe(receiver);
  }

  template<typename E>
  void unsubscribe(Receiver<E>& receiver) {
    _senders.get<Sender<E>>().unsubscribe(receiver);
  }

  template<typename E>
  void send(const E& event) const {
    _senders.get<Sender<E>>().send(event);
  }

private:

  Omniset _senders;
};

template<typename E>
class Receiver {
public:
  Receiver() = default;
  Receiver(const Receiver<E>&) = delete;

  Receiver(Receiver<E>&& other)
    : _sender(other._sender)
  {
    if (_sender) {
      auto it = std::find( _sender->_receivers.begin()
                         , _sender->_receivers.end()
                         , &other);

      if (it != _sender->_receivers.end()) {
        *it = this;
      }
    }
  }

  virtual ~Receiver() {
    if (_sender) {
      _sender->unsubscribe(*this);
    }
  }

  Receiver<E>& operator = (const Receiver<E>&) = delete;

  Receiver<E>& operator = (Receiver<E>&& other) {
    if (_sender) {
      _sender->unsubscribe(*this);
    }

    if (other._sender) {
      other._sender->subscribe(*this);
      other._sender->unsubscribe(other);
    }

    return *this;
  }

  virtual void receive(const E&) {}

private:
  Sender<E>* _sender = nullptr;
  friend class Sender<E>;
};

template<typename E>
class Sender {
public:
  Sender() = default;
  Sender(const Sender<E>&) = delete;

  Sender(Sender<E>&& other)
    : _receivers(std::move(other._receivers))
  {
    for (auto receiver : _receivers) {
      if (receiver) receiver->_sender = this;
    }
  }

  ~Sender() {
    for (auto receiver : _receivers) {
      if (receiver) receiver->_sender = nullptr;
    }
  }

  Sender<E>& operator = (const Sender<E>&) = delete;

  Sender<E>& operator = (Sender<E>&& other) {
    _receivers = std::move(other._receivers);

    for (auto receiver : _receivers) {
      if (receiver) receiver->_sender = this;
    }

    return *this;
  }

  void subscribe(Receiver<E>& receiver) {
    if (receiver._sender == this) return;

    if (receiver._sender) {
      receiver._sender->unsubscribe(receiver);
    }

    receiver._sender = this;
    _receivers.push_back(&receiver);
  }

  void unsubscribe(Receiver<E>& receiver) {
    if (receiver._sender != this) return;

    auto it = std::find(_receivers.begin(), _receivers.end(), &receiver);

    if (it != _receivers.end()) {
      _receivers.erase(it);
      receiver._sender = nullptr;
    }
  }

  void send(const E& event) const {
    for (auto receiver : _receivers) {
      receiver->receive(event);
    }
  }

private:
  std::vector<Receiver<E>*> _receivers;
  friend class Receiver<E>;
};

} // namespace secs
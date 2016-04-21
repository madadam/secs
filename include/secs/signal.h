#pragma once

// Minimalistic implementation of the Signal-Slot mechanism.

#include <cassert>
#include <functional>
#include <vector>

#include "secs/functional.h"

namespace secs {

class ScopedConnection;
template<typename> class Signal;

class Connection {
public:
  Connection() = default;

  Connection(Connection&&) noexcept;
  Connection& operator = (Connection&& other) noexcept;

  Connection(const Connection&) = delete;
  Connection& operator = (const Connection&) = delete;

  bool connected() const {
    return _signal != nullptr;
  }

  void disconnect();

  // Convert this connecion to ScopedConnection,
  ScopedConnection scoped() &&;

private:
  using DisconnectFun = void (*)(void*, size_t);

  template<typename T>
  static void disconnect(void* signal, size_t index) {
    reinterpret_cast<Signal<T>*>(signal)->disconnect(index);
  }

private:
  template<typename T>
  Connection(Signal<T>* signal, size_t index)
    : _signal(signal)
    , _index(index)
    , _disconnect(&disconnect<T>)
  {}

  void*         _signal = nullptr;
  size_t        _index  = 0;
  DisconnectFun _disconnect = nullptr;

  template<typename> friend class Signal;
};

template<typename T>
class Signal {
  static_assert(!std::is_reference<T>::value, "T cannot be reference");

public:
  Signal() = default;
  Signal(const Signal&) = delete;
  Signal(Signal&&) = delete;

  ~Signal() {
    disconnect_all();
  }

  Signal& operator = (const Signal&) = delete;
  Signal& operator = (Signal&&) = delete;

  template<typename F>
  Connection connect(F&& fun);

  void disconnect_all() {
    _slots.clear();
    _holes.clear();
  }

  void operator () (const T& event) const {
    for (auto& slot : _slots) {
      if (slot) slot(event);
    }
  }

private:
  size_t reserve() {
    if (_holes.empty()) {
      _slots.resize(_slots.size() + 1);
      return _slots.size() - 1;
    } else {
      auto index = _holes.back();
      _holes.pop_back();
      return index;
    }
  }

  void disconnect(size_t index) {
    assert(index < _slots.size());
    assert(_slots[index]);

    _slots[index] = nullptr;
    _holes.push_back(index);
  }

private:
  std::vector<std::function<void(const T&)>> _slots;
  std::vector<size_t>                        _holes;

  friend class Connection;
};

template<typename F, typename T>
std::result_of_t<F(const T&)> invoke(F fun, const T& event) {
  return fun(event);
}

template<typename F, typename T>
std::result_of_t<F()> invoke(F fun, const T&) {
  return fun();
}

// Guard class that automatically disconnects the associated connection when it
// goes out of scope.
//
// Warning: make sure this class never outlives the Signal it's connected to.
class ScopedConnection {
public:
  ScopedConnection() = default;
  ScopedConnection(Connection&& connection)
    : _connection(std::move(connection))
  {}

  ~ScopedConnection() { disconnect(); }

  ScopedConnection(ScopedConnection&&) = default;
  ScopedConnection& operator = (ScopedConnection&&) = default;

  ScopedConnection& operator = (Connection&& connection) {
    _connection = std::move(connection);
    return *this;
  }

  bool connected() const { return _connection.connected(); }
  void disconnect() { _connection.disconnect(); }

private:
  Connection _connection;
};

inline ScopedConnection Connection::scoped() && {
  return { std::move(*this) };
}

} // namespace secs

template<typename T> template<typename F>
secs::Connection secs::Signal<T>::connect(F&& fun) {
  using secs::invoke;

  auto index = reserve();
  _slots[index] = [fun = std::forward<F>(fun)](const T& event) {
    invoke(fun, event);
  };
  return Connection(this, index);
}

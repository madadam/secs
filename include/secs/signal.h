#pragma once

// Minimalistic implementation of the Signal-Slot mechanism.

#include <cassert>
#include <functional>
#include <vector>

namespace secs {
template<typename...> class ScopedConnection;
template<typename...> class Signal;

template<typename... Args>
class Connection {
public:
  Connection() = default;

  Connection(Connection&& other) noexcept
    : _signal(other._signal)
    , _index(other._index)
  {
    if (_signal) {
      _signal->_slots[_index].connection = this;
      other._signal = nullptr;
    }
  }

  Connection& operator = (Connection&& other) noexcept {
    _signal = other._signal;
    _index  = other._index;

    if (_signal) {
      _signal->_slots[_index].connection = this;
      other._signal = nullptr;
    }

    return *this;
  }

  Connection(const Connection&) = delete;
  Connection& operator = (const Connection&) = delete;

  bool connected() const {
    return _signal != nullptr;
  }

  void disconnect() {
    if (!_signal) return;

    _signal->disconnect(_index);
    _signal = nullptr;
  }

  // Convert this connecion to ScopedConnection,
  ScopedConnection<Args...> scoped() && {
    return { std::move(*this) };
  }

private:
  Connection(Signal<Args...>* signal, size_t index)
    : _signal(signal)
    , _index(index)
  {}

  Signal<Args...>* _signal = nullptr;
  size_t           _index  = 0;

  template<typename...> friend class Signal;
};

template<typename... Args>
class ScopedConnection {
public:
  ScopedConnection() = default;
  ScopedConnection(Connection<Args...>&& connection)
    : _connection(std::move(connection))
  {}

  ~ScopedConnection() { disconnect(); }

  ScopedConnection(ScopedConnection&&) = default;
  ScopedConnection& operator = (ScopedConnection&&) = default;

  bool connected() const { return _connection.connection(); }
  void disconnect() {  _connection.disconnect(); }

private:
  Connection<Args...> _connection;
};

template<typename... Args>
class Signal {
private:
  struct Slot {
    std::function<void(Args...)> function;
    Connection<Args...>*         connection = nullptr;
  };

public:
  Signal() = default;

  Signal(Signal&& other)
    : _slots(std::move(other._slots))
    , _holes(std::move(other._holes))
  {
    for (auto& slot : _slots) {
      if (slot.connection) {
        slot.connection->_signal = this;
      }
    }
  }

  ~Signal() {
    disconnect_all();
  }

  Signal(const Signal&) = delete;
  Signal& operator = (const Signal&) = delete;

  Connection<Args...> connect(std::function<void(Args...)> fun) {
    if (_holes.empty()) {
      auto index = _slots.size();
      Connection<Args...> connection(this, index);
      _slots.push_back({ std::move(fun), &connection });

      return connection;
    } else {
      auto index = _holes.back();
      _holes.pop_back();
      Connection<Args...> connection(this, index);
      _slots[index].function   = std::move(fun);
      _slots[index].connection = &connection;

      return connection;
    }
  }

  void disconnect_all() {
    for (auto& slot : _slots) {
      if (slot.connection) {
        slot.connection->_signal = nullptr;
        slot.connection = nullptr;
      }

      slot.function = nullptr;
    }

    _holes.clear();
  }

  void operator () (Args&&... args) const {
    for (auto& slot : _slots) {
      if (slot.function) {
        slot.function(std::forward<Args>(args)...);
      }
    }
  }

private:
  void disconnect(size_t index) {
    assert(index >= 0 && index < _slots.size());
    assert(_slots[index].function);

    _slots[index].function   = nullptr;
    _slots[index].connection = nullptr;

    _holes.push_back(index);
  }

private:
  std::vector<Slot>   _slots;
  std::vector<size_t> _holes;

  template<typename...> friend class Connection;
};

} // namespace secs

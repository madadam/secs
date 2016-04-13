#include "secs/signal.h"

using namespace secs;

Connection::Connection(Connection&& other) noexcept
  : _signal(other._signal)
  , _index(other._index)
  , _disconnect(other._disconnect)
{
  other._signal = nullptr;
}

Connection& Connection::operator = (Connection&& other) noexcept {
  _signal     = other._signal;
  _index      = other._index;
  _disconnect = other._disconnect;

  other._signal = nullptr;

  return *this;
}

void Connection::disconnect() {
  if (!_signal) return;
  _disconnect(_signal, _index);
  _signal = nullptr;
}

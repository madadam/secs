#include "secs/container.h"
#include "secs/handle.h"

using namespace secs;

bool detail::Handle::valid() const {
  return _container && _version > 0 && _container->get_version(_index) == _version;
}

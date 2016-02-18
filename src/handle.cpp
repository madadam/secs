#include "secs/handle.h"
#include "secs/entity_store.h"

using namespace secs;

bool detail::Handle::valid() const {
  return _store && _version > 0 && _store->get_version(_index) == _version;
}

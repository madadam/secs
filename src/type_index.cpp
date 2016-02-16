#include "type_index.h"

using namespace secs;

const size_t TypeIndex::INVALID = std::numeric_limits<size_t>::max();
size_t       TypeIndex::_next = 0;
std::mutex   TypeIndex::_mutex;

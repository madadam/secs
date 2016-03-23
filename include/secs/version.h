#pragma once

namespace secs {

struct Version {
public:
  bool exists() const {
    return _value & 1;
  }

  uint32_t serial() const {
    return _value >> 1;
  }

  void create() {
    assert(!exists());
    _value = (((_value >> 1) + 1) << 1) | 1;
  }

  void destroy() {
    assert(exists());
    _value = ((_value >> 1) + 1) << 1;
  }

private:
  uint32_t _value = 0;

  friend bool operator == (Version, Version);
  friend bool operator != (Version, Version);
  friend bool operator <  (Version, Version);
  friend bool operator >  (Version, Version);
};

inline bool operator == (Version a, Version b) { return a._value == b._value; }
inline bool operator != (Version a, Version b) { return a._value != b._value; }
inline bool operator <  (Version a, Version b) { return a._value <  b._value; }
inline bool operator >  (Version a, Version b) { return a._value >  b._value; }

} // namespace secs
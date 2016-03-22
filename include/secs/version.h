#pragma once

namespace secs {

struct Version {
public:
  Version()
    : whole(0)
  {}

  bool exists() const {
    return parts.exists;
  }

  uint32_t serial() const {
    return parts.serial;
  }

  Version create() {
    parts.exists = true;
    ++parts.serial;
    return *this;
  }

  Version destroy() {
    parts.exists = false;
    ++parts.serial;
    return *this;
  }

private:
  union {
    struct {
      uint32_t serial : 31;
      bool     exists : 1;
    } parts;

    uint32_t whole;
  };

  friend bool operator == (Version, Version);
  friend bool operator <  (Version, Version);
};

inline bool operator == (Version a, Version b) { return a.whole == b.whole; }
inline bool operator != (Version a, Version b) { return !(a == b); }
inline bool operator <  (Version a, Version b) { return a.whole < b.whole; }

} // namespace secs
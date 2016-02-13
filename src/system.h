#pragma once

namespace secs {

template<typename...> class Environment;

template<typename... Context>
class System {
public:

  virtual ~System() {}
  virtual void update(Environment<Context...>& env, Context... context) = 0;
};

} // namespace secs

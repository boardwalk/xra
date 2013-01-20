#ifndef XRA_ENV_HPP
#define XRA_ENV_HPP

#include "scoped-map.hpp"
#include "value.hpp"

namespace xra {

class Env : public ScopedMap<string, ValuePtr>
{
public:
  void Apply(const TypeSubst& subst)
  {
    for(auto& pair : Values())
      pair.second->Apply(subst);
  }
};

inline ostream& operator<<(ostream& os, const Env& env)
{
  os << "{env";
  for(auto& pair : env.Values()) {
    os << " " << pair.first << "=";
    if(pair.second)
      os << *pair.second;
    else
      os << "(null)";
  }
  os << "}";
  return os;
}

} // namespace xra

#endif // XRA_ENV_HPP

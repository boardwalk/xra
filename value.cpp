#include "common.hpp"
#include "value.hpp"
#include "type.hpp"

namespace xra {

void Value::Apply(const TypeSubst& subst)
{
  if(type) {
    auto localSubst = subst;
    for(auto& var : freeVars)
      localSubst.erase(var);
    type = xra::Apply(localSubst, *type);
  }
}

} // namespace xra

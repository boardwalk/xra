#include "common.hpp"
#include "value.hpp"
#include "type.hpp"

namespace xra {

static ValuePtr MakeVoid()
{
  ValuePtr value = new VConstant;
  value->type = new TList;
  return value;
}
const ValuePtr VoidValue = MakeVoid();

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

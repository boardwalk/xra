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

VConstant::VConstant() :
  Value(Kind_VConstant)
{
  type = VoidType;
}

VConstant::VConstant(bool value) :
  Value(Kind_VConstant),
  boolValue(value)
{
  type = BooleanType;
}

VConstant::VConstant(long value) :
  Value(Kind_VConstant),
  intValue(value)
{
  type = IntegerType;
}

VConstant::VConstant(double value) :
  Value(Kind_VConstant),
  floatValue(value)
{
  type = FloatType;
}

VConstant::VConstant(const string& value) :
  Value(Kind_VConstant),
  strValue(value)
{
  type = StringType;
}

} // namespace xra

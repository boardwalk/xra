#include "common.hpp"
#include "visitor.hpp"

namespace xra {

struct ValueToStringVisitor : Visitor<ValueToStringVisitor, const Value>
{
  ostream& os;

  ValueToStringVisitor(ostream& os_) :
    os(os_)
  {}

#define VISIT(c) \
  void Visit##c(const c& value) { \
    os << #c;                     \
    if(value.type)                \
      os << ":" << *value.type;   \
  }

  VISIT(VBuiltin)
  VISIT(VTemporary)
  VISIT(VConstant)
  VISIT(VLocal)
  VISIT(VExtern)

#undef VISIT
};

ostream& operator<<(ostream& os, const Value& value)
{
  ValueToStringVisitor visitor(os);
  visitor.Visit(&value);
  return os;
}

} // namespace xra

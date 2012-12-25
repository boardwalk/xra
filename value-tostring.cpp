#include "common.hpp"
#include "value.hpp"
#include "value-visitor.hpp"

namespace xra {

struct ValueToStringVisitor : ValueVisitor<ValueToStringVisitor, const Value>
{
  ostream& os;

  ValueToStringVisitor(ostream& os_) :
    os(os_)
  {}

#define VISIT(c) \
  void Visit##c(const V##c& value) { \
    os << #c;                        \
    if(value.type)                   \
      os << ":" << *value.type;      \
  }

  VISIT(Builtin)
  VISIT(Temporary)
  VISIT(Constant)
  VISIT(Local)
  VISIT(Extern)

#undef VISIT
};

ostream& operator<<(ostream& os, const Value& value)
{
  ValueToStringVisitor visitor(os);
  visitor.Visit(&value);
  return os;
}

} // namespace xra

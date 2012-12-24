#include "common.hpp"
#include "value.hpp"
#include "value-visitor.hpp"

namespace xra {

struct ValueToStringVisitor : ValueVisitor<ValueToStringVisitor, const Value>
{
  stringstream& ss;

  ValueToStringVisitor(stringstream& ss_) :
    ss(ss_)
  {}

#define VISIT(c) \
  void Visit##c(const V##c& value) { \
    ss << #c;                        \
    if(value.type) {                 \
      ss << ":";                     \
      ToString(*value.type, ss);     \
    }                                \
  }

  VISIT(Error)
  VISIT(Builtin)
  VISIT(Temporary)
  VISIT(Constant)
  VISIT(Local)
  VISIT(Extern)

#undef VISIT
};

void ToString(const Value& value, stringstream& ss)
{
  ValueToStringVisitor visitor(ss);
  visitor.Visit(value);
}

} // namespace xra

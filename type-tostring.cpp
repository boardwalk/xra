#include "common.hpp"
#include "type.hpp"
#include "type-visitor.hpp"

namespace xra {

struct TypeToStringVisitor
{
  string str;

  void Visit(const TVariable& type)
  {
    str.append(type.name);
  }
};

string Type::ToString() const
{
  TypeToStringVisitor visitor;
  VisitType(*this, visitor);
  return visitor.str;
}

} // namespace xra

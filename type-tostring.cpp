#include "common.hpp"
#include "type.hpp"
#include "type-visitor.hpp"

namespace xra {

struct TypeToStringVisitor : TypeVisitor<TypeToStringVisitor>
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
  visitor.DoVisit(*this);
  return visitor.str;
}

} // namespace xra

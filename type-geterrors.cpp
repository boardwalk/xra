#include "common.hpp"
#include "type.hpp"
#include "type-visitor.hpp"

namespace xra {

struct TypeGetErrorsVisitor : TypeVisitor<TypeGetErrorsVisitor, const Type>
{
  string str;

  void Visit(const TError& type)
  {
    if(!str.empty())
      str += '\n';
    str += type.what;
  }
};

string Type::GetErrors() const
{
  TypeGetErrorsVisitor visitor;
  visitor.VisitAny(*this);
  return move(visitor.str);
}

} // namespace xra

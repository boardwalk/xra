#include "common.hpp"
#include "type.hpp"
#include "type-visitor.hpp"

namespace xra {

struct TypeGetErrorsVisitor : TypeVisitor<TypeGetErrorsVisitor, const Type>
{
  string str;

  void VisitError(const TError& type)
  {
    if(!str.empty())
      str += '\n';
    str += type.what;
    base::VisitError(type);
  }
};

string Type::GetErrors() const
{
  TypeGetErrorsVisitor visitor;
  visitor.Visit(*this);
  return move(visitor.str);
}

} // namespace xra

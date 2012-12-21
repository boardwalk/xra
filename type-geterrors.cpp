#include "common.hpp"
#include "type.hpp"
#include "type-visitor.hpp"

namespace xra {

struct TypeGetErrorsVisitor
{
  string str;

  void Visit(const Type& type)
  {
    // nothing
  }

  void Visit(const TError& type)
  {
    if(!str.empty())
      str += '\n';
    str += type.what;
  }

  void Visit(const TList& type)
  {
    for(auto& t : type.types)
      VisitType(*t, *this);
  }
  
  void Visit(const TFunction& type)
  {
    VisitType(*type.argument, *this);
    VisitType(*type.result, *this);
  }
};

string Type::GetErrors() const
{
  TypeGetErrorsVisitor visitor;
  VisitType(*this, visitor);
  return move(visitor.str);
}

} // namespace xra

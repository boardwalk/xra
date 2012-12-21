#include "common.hpp"
#include "type.hpp"
#include "type-visitor.hpp"
#include <sstream>

namespace xra {

struct TypeToStringVisitor
{
  stringstream ss;

  void Visit(const TError& type)
  {
    ss << "<" << type.what << ">";
  }

  void Visit(const TVoid& type)
  {
    ss << "()";
  }

  void Visit(const TVariable& type)
  {
    ss << type.name;
  }

  void Visit(const TList& type)
  {
    int i = 0;
    for(auto& t : type.types) {
      if(i++ != 0) ss << ", ";
      VisitType(*t, *this);
    }
  }

  void Visit(const TFunction& type)
  {
    VisitType(*type.argument, *this);
    ss << " -> ";
    VisitType(*type.result, *this);
  }
};

string Type::ToString() const
{
  TypeToStringVisitor visitor;
  VisitType(*this, visitor);
  return visitor.ss.str();
}

} // namespace xra

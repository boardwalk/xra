#include "common.hpp"
#include "type.hpp"
#include "type-visitor.hpp"
#include <sstream>

namespace xra {

struct TypeToStringVisitor : TypeVisitor<TypeToStringVisitor, const Type>
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
    ss << "(list";
    base::Visit(type);
    ss << ")";
  }

  void Visit(const TFunction& type)
  {
    ss << "(fn";
    base::Visit(type);
    ss << ")";
  }
};

string Type::ToString() const
{
  TypeToStringVisitor visitor;
  visitor.VisitAny(*this);
  return visitor.ss.str();
}

} // namespace xra

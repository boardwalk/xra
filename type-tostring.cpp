#include "common.hpp"
#include "type.hpp"
#include "type-visitor.hpp"

namespace xra {

struct TypeToStringVisitor : TypeVisitor<TypeToStringVisitor, const Type>
{
  ostream& os;
  bool firstVisit;

  TypeToStringVisitor(ostream& os_) :
    os(os_), firstVisit(true)
  {}

  void VisitVoid(const TVoid& type)
  {
    os << "()";
  }

  void VisitBoolean(const TBoolean& type)
  {
    os << "bool";
  }

  void VisitInteger(const TInteger& type)
  {
    os << "int";
  }

  void VisitFloat(const TFloat& type)
  {
    os << "float";
  }

  void VisitString(const TString& type)
  {
    os << "str";
  }

  void VisitVariable(const TVariable& type)
  {
    os << "`" << type.name << "`";
  }

  void VisitList(const TList& type)
  {
    os << "(list";
    base::VisitList(type);
    os << ")";
  }

  void VisitFunction(const TFunction& type)
  {
    os << "(fn";
    base::VisitFunction(type);
    os << ")";
  }

  void Visit(const Type* type)
  {
    if(firstVisit)
      firstVisit = false;
    else
      os << " ";
    base::Visit(type);
  }
};

ostream& operator<<(ostream& os, const Type& type)
{
  TypeToStringVisitor visitor(os);
  visitor.Visit(&type);
  return os;
}

} // namespace xra

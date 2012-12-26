#include "common.hpp"
#include "visitor.hpp"

namespace xra {

struct TypeToStringVisitor : Visitor<TypeToStringVisitor, const Type>
{
  ostream& os;
  bool firstVisit;

  TypeToStringVisitor(ostream& os_) :
    os(os_), firstVisit(true)
  {}

  void VisitTVoid(const TVoid& type)
  {
    os << "()";
  }

  void VisitTBoolean(const TBoolean& type)
  {
    os << "bool";
  }

  void VisitTInteger(const TInteger& type)
  {
    os << "int";
  }

  void VisitTFloat(const TFloat& type)
  {
    os << "float";
  }

  void VisitTString(const TString& type)
  {
    os << "str";
  }

  void VisitTVariable(const TVariable& type)
  {
    os << "`" << type.name << "`";
  }

  void VisitTList(const TList& type)
  {
    os << "(list";
    base::VisitTList(type);
    os << ")";
  }

  void VisitTFunction(const TFunction& type)
  {
    os << "(fn";
    base::VisitTFunction(type);
    os << ")";
  }

  void Visit(const Base* base)
  {
    if(firstVisit)
      firstVisit = false;
    else
      os << " ";
    base::Visit(base);
  }
};

ostream& operator<<(ostream& os, const Type& type)
{
  TypeToStringVisitor visitor(os);
  visitor.Visit(&type);
  return os;
}

} // namespace xra

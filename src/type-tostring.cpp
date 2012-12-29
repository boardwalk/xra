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

  void VisitTBoolean(const TBoolean&)
  {
    os << "bool";
  }

  void VisitTInteger(const TInteger&)
  {
    os << "int";
  }

  void VisitTFloat(const TFloat&)
  {
    os << "float";
  }

  void VisitTString(const TString&)
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

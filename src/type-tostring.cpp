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

  void VisitTInteger(const TInteger& type)
  {
    os << "int ";
    os << (type._signed ? "signed " : "unsigned ");
    os << type.width;
  }

  void VisitTFloat(const TFloat& type)
  {
    os << "float ";
    os << type.width;
  }

  void VisitTString(const TString&)
  {
    os << "str";
  }

  void VisitTVariable(const TVariable& type)
  {
    os << type.name;
  }

  void VisitTList(const TList& type)
  {
    os << "(";

    int i = 0;
    for(auto& f : type.fields)
    {
      if(i++ != 0)
        os << ", ";

      if(!f.name.empty())
        os << f.name << "\\";

      Visit(f.type.get());
    }

    os << ")";
  }

  void VisitTFunction(const TFunction& type)
  {
    Visit(type.parameter.get());
    os << " -> ";
    Visit(type.result.get());
  }
};

ostream& operator<<(ostream& os, const Type& type)
{
  TypeToStringVisitor visitor(os);
  visitor.Visit(&type);
  return os;
}

} // namespace xra

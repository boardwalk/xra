#include "common.hpp"
#include "type.hpp"
#include "type-visitor.hpp"

namespace xra {

struct TypeToStringVisitor : TypeVisitor<TypeToStringVisitor, const Type>
{
  stringstream& ss;
  bool firstVisit;

  TypeToStringVisitor(stringstream& ss_) :
    ss(ss_), firstVisit(true)
  {}

  void VisitVoid(const TVoid& type)
  {
    ss << "()";
  }

  void VisitBoolean(const TBoolean& type)
  {
    ss << "bool";
  }

  void VisitInteger(const TInteger& type)
  {
    ss << "int";
  }

  void VisitFloat(const TFloat& type)
  {
    ss << "float";
  }

  void VisitString(const TString& type)
  {
    ss << "str";
  }

  void VisitVariable(const TVariable& type)
  {
    ss << "`" << type.name << "`";
  }

  void VisitList(const TList& type)
  {
    ss << "(list";
    base::VisitList(type);
    ss << ")";
  }

  void VisitFunction(const TFunction& type)
  {
    ss << "(fn";
    base::VisitFunction(type);
    ss << ")";
  }

  void Visit(const Type* type)
  {
    if(firstVisit)
      firstVisit = false;
    else
      ss << " ";
    base::Visit(type);
  }
};

void ToString(const Type& type, stringstream& ss)
{
  TypeToStringVisitor visitor(ss);
  visitor.Visit(&type);
}

} // namespace xra

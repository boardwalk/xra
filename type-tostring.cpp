#include "common.hpp"
#include "type.hpp"
#include "type-visitor.hpp"
#include <sstream>

namespace xra {

struct TypeToStringVisitor : TypeVisitor<TypeToStringVisitor, const Type>
{
  stringstream ss;

  void VisitError(const TError& type)
  {
    ss << " <" << type.what << ">";
  }

  void VisitVoid(const TVoid& type)
  {
    ss << " ()";
  }

  void VisitVariable(const TVariable& type)
  {
    ss << " " << type.name;
  }

  void VisitList(const TList& type)
  {
    ss << " (list";
    base::VisitList(type);
    ss << ")";
  }

  void VisitFunction(const TFunction& type)
  {
    ss << " (fn";
    base::VisitFunction(type);
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

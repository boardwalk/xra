#include "common.hpp"
#include "type.hpp"
#include "type-visitor.hpp"

namespace xra {

struct TypeGetVariablesVisitor : TypeVisitor<TypeGetVariablesVisitor, const Type>
{
  set<string>& variables;

  TypeGetVariablesVisitor(set<string>& variables_) :
    variables(variables_)
  {}

  void VisitVariable(const TVariable& type)
  {
    variables.insert(type.name);
  }
};

void GetVariables(const Type& type, set<string>& variables)
{
  TypeGetVariablesVisitor visitor(variables);
  visitor.Visit(type);
}

} // namespace xra
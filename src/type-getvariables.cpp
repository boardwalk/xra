#include "common.hpp"
#include "visitor.hpp"

namespace xra {

struct TypeGetVariablesVisitor : Visitor<TypeGetVariablesVisitor, const Type>
{
  set<string>& variables;

  TypeGetVariablesVisitor(set<string>& variables_) :
    variables(variables_)
  {}

  void VisitTVariable(const TVariable& type)
  {
    variables.insert(type.name);
  }
};

void GetVariables(const Type& type, set<string>& variables)
{
  TypeGetVariablesVisitor(variables).Visit(&type);
}

} // namespace xra

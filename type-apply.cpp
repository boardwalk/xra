#include "common.hpp"
#include "type.hpp"
#include "type-visitor.hpp"

namespace xra {

struct TypeApplyVisitor : TypeVisitor<TypeApplyVisitor, Type>
{
  const TypeSubst& subst;
  TypePtr result;

  TypeApplyVisitor(const TypeSubst& subst_) :
    subst(subst_)
  {}

  void VisitError(TError& type)
  {
    result = &type;
  }

  void VisitVoid(TVoid& type)
  {
    result = &type;
  }

  void VisitVariable(TVariable& type)
  {
    auto it = subst.find(type.name);
    if(it != subst.end())
      result = it->second;
    else
      result = &type;
  }

  void VisitList(TList& type)
  {
    auto list = make_unique<TList>();
    for(auto& t : type.types) {
      VisitAny(*t);
      list->types.push_back(result);
    }
    result = list.release();
  }

  void VisitFunction(TFunction& type)
  {
    auto function = make_unique<TFunction>();
    VisitAny(*type.parameter);
    function->parameter = result;
    VisitAny(*type.result);
    function->result = result;
    result = function.release();
  }

  void VisitBuiltin(TBuiltin& type)
  {
    result = &type;
  }
};

TypePtr Apply(const TypeSubst& subst, Type& type)
{
  TypeApplyVisitor visitor(subst);
  visitor.VisitAny(type);
  return visitor.result;
}
  
} // namespace xra

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

  void VisitVoid(TVoid& type)
  {
    result = &type;
  }

  void VisitBoolean(TBoolean& type)
  {
    result = &type;
  }

  void VisitInteger(TInteger& type)
  {
    result = &type;
  }

  void VisitFloat(TFloat& type)
  {
    result = &type;
  }

  void VisitString(TString& type)
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
      Visit(t.get());
      list->types.push_back(result);
    }
    result = list.release();
  }

  void VisitFunction(TFunction& type)
  {
    auto function = make_unique<TFunction>();
    Visit(type.parameter.get());
    function->parameter = result;
    Visit(type.result.get());
    function->result = result;
    result = function.release();
  }
};

TypePtr Apply(const TypeSubst& subst, Type& type)
{
  TypeApplyVisitor visitor(subst);
  visitor.Visit(&type);
  return visitor.result;
}

} // namespace xra

#include "common.hpp"
#include "visitor.hpp"

namespace xra {

struct TypeApplyVisitor : Visitor<TypeApplyVisitor, Type>
{
  const TypeSubst& subst;
  TypePtr result;

  TypeApplyVisitor(const TypeSubst& subst_) :
    subst(subst_)
  {}

  void VisitTBoolean(TBoolean& type)
  {
    result = &type;
  }

  void VisitTInteger(TInteger& type)
  {
    result = &type;
  }

  void VisitTFloat(TFloat& type)
  {
    result = &type;
  }

  void VisitTString(TString& type)
  {
    result = &type;
  }

  void VisitTVariable(TVariable& type)
  {
    auto it = subst.find(type.name);
    if(it != subst.end())
      result = it->second;
    else
      result = &type;
  }

  void VisitTList(TList& type)
  {
    auto list = make_unique<TList>();
    for(auto& f : type.fields) {
      Visit(f.type.get());
      list->fields.push_back({f.name, result});
    }
    result = list.release();
  }

  void VisitTFunction(TFunction& type)
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

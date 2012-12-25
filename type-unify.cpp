#include "common.hpp"
#include "type.hpp"
#include "type-visitor.hpp"

namespace xra {

static void BindVariable(const string& name, Type& type, TypeSubst& subst)
{
  set<string> variables;
  GetVariables(type, variables);

  if(variables.find(name) == variables.end())
    subst[name] = &type;
  else
    Error() << "occur check fails for " << name;
}

struct TypeUnifyVisitor : TypeVisitor<TypeUnifyVisitor, Type>
{
  Type& other;
  TypeSubst subst;

  TypeUnifyVisitor(Type& other_) :
    other(other_)
  {}

  void VisitVariable(TVariable& type)
  {
    if(type.name != static_cast<TVariable&>(other).name)
      BindVariable(type.name, other, subst);
  }

  void VisitList(TList& type)
  {
    auto& otherList = static_cast<TList&>(other);

    if(type.types.size() != otherList.types.size()) {
      Error() << "lists differ in length";
      return;
    }

    auto otherIt = otherList.types.begin();
    for(auto& t : type.types) {
      Compose(Unify(*t, **otherIt), subst);
      ++otherIt;
    }
  }

  void VisitFunction(TFunction& type)
  {
    auto& otherFunc = static_cast<TFunction&>(other);

    // s1 <- mgu l l'
    subst = Unify(*type.parameter, *otherFunc.parameter);

    // s2 <- mgu (apply s1 r) (apply s1 r')
    auto result = Apply(subst, *type.result);
    auto otherResult = Apply(subst, *otherFunc.result);
    auto unifySubst = Unify(*result, *otherResult);

    // return s1 `composeSubst` s2
    Compose(unifySubst, subst);
  }
};

TypeSubst Unify(Type& left, Type& right)
{
  TypeSubst subst;

  if(left.kind == right.kind) {
    TypeUnifyVisitor visitor(right);
    visitor.Visit(&left);
    subst = move(visitor.subst);
  }
  else if(left.kind == Type::Kind_TVariable) {
    BindVariable(static_cast<TVariable&>(left).name, right, subst);
  }
  else if(right.kind == Type::Kind_TVariable) {
    BindVariable(static_cast<TVariable&>(right).name, left, subst);
  }
  else {
    Error() << "expected equal types or at least one variable: " << left << "; " << right;
  }

  return subst;
}
  
} // namespace xra

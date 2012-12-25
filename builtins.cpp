#include "common.hpp"
#include "expr.hpp"
#include "env.hpp"
#include <iostream>

namespace xra {

/*
 * ";" is a list of expressions where value of the last is returned
 */
class BSequence : public VBuiltin
{
public:
  ValuePtr Infer(Env& env, TypeSubst& subst, Expr& argument)
  {
    if(!isa<EList>(argument))
      return new VError("expected list argument to BAssign");

    auto& list = static_cast<EList&>(argument);

    for(auto& e : list.exprs)
    {
      TypeSubst lastSubst;
      subst.swap(lastSubst);

      e->Infer(env, subst);

      Compose(lastSubst, subst);
    }

    return list.exprs.back()->value;
  }
};

class BAssign : public VBuiltin
{
public:
  ValuePtr Infer(Env& env, TypeSubst& subst, Expr& argument)
  {
    if(!isa<EList>(argument))
      return new VError("expected list argument to BAssign");

    auto& list = static_cast<EList&>(argument);

    if(list.exprs.size() != 2)
      return new VError("expected two arguments to BAssign");

    Expr& lhs = *list.exprs[0];
    Expr& rhs = *list.exprs[1];

    rhs.Infer(env, subst);

    // if the left side is a plain variable not in the environment, create a fresh local
    if(isa<EVariable>(lhs))
    {
      auto& name = static_cast<EVariable&>(lhs).name;
      if(!env[name]) {
        lhs.value.reset(new VLocal);
        lhs.value->type = MakeTypeVar();
        env.AddValue(name, lhs.value);
      }
    }

    if(!lhs.value) {
      TypeSubst lastSubst;
      subst.swap(lastSubst);
      lhs.Infer(env, subst);
      Compose(lastSubst, subst);
    }

    auto unifySubst = Unify(*lhs.value->type, *rhs.value->type);

    lhs.value->type = xra::Apply(unifySubst, *lhs.value->type);

    Compose(unifySubst, subst);
    return lhs.value;
  }
};

void AddBuiltins(Env& env)
{
  env.AddValue(";", new BSequence);
  env.AddValue("=", new BAssign);
}
  
} // namespace xra

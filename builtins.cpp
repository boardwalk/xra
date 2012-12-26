#include "common.hpp"
#include "compiler.hpp"
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
    if(!isa<EList>(argument)) {
      Error() << "expected list argument to BSequence";
      return {};
    }
    
    auto& list = static_cast<EList&>(argument);

    for(auto& e : list.exprs)
    {
      TypeSubst lastSubst;
      subst.swap(lastSubst);

      e->Infer(env, subst);

      if(!e->value)
        return {};

      Compose(lastSubst, subst);
    }

    return list.exprs.back()->value;
  }

  void Compile(Compiler& compiler, const vector<ExprPtr>& argument)
  {
    for(auto& e : argument)
      compiler.Visit(e.get());
  }
};

class BAssign : public VBuiltin
{
public:
  ValuePtr Infer(Env& env, TypeSubst& subst, Expr& argument)
  {
    if(!isa<EList>(argument)) {
      Error() << "expected list argument to BAssign";
      return {};
    }

    auto& list = static_cast<EList&>(argument);

    if(list.exprs.size() != 2) {
      Error() << "expected two arguments to BAssign";
      return {};
    }

    ExprPtr& left = list.exprs[0];
    ExprPtr& right = list.exprs[1];

    right->Infer(env, subst);

    if(!right->value)
      return {};

    // if the left side is a plain variable not in the environment, create a fresh local
    if(isa<EVariable>(left.get()))
    {
      auto& name = static_cast<EVariable&>(*left).name;
      if(!env[name]) {
        left->value = new VLocal;
        left->value->type = MakeTypeVar();
        env.AddValue(name, left->value);
      }
    }

    if(!left->value)
    {
      TypeSubst leftSubst;
      left->Infer(env, leftSubst);
      if(!left->value)
        return {};
      Compose(leftSubst, subst);
    }

    auto unifySubst = Unify(*left->value->type, *right->value->type);
    left->value->type = xra::Apply(unifySubst, *left->value->type);
    Compose(unifySubst, subst);
    return left->value;
  }

  void Compile(Compiler& compiler, const vector<ExprPtr>& args)
  {
  }
};

void AddBuiltins(Env& env)
{
  env.AddValue(";", new BSequence);
  env.AddValue("=", new BAssign);
}
  
} // namespace xra

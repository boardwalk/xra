#include "common.hpp"
#include "compiler.hpp"
#include "env.hpp"
#include <iostream>

namespace xra {

class BSequence : public VBuiltin
{
public:
  ValuePtr Infer(Env&, TypeSubst&, Expr&);
  void Compile(Compiler&, const vector<ExprPtr>&);
};

class BAssign : public VBuiltin
{
public:
  ValuePtr Infer(Env& env, TypeSubst& subst, Expr& argument);
  void Compile(Compiler&, const vector<ExprPtr>&);
};

void AddBuiltins(Env& env)
{
  env.AddValue(";", new BSequence);
  env.AddValue("=", new BAssign);
}

ValuePtr BSequence::Infer(Env& env, TypeSubst& subst, Expr& argument)
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

ValuePtr BAssign::Infer(Env& env, TypeSubst& subst, Expr& argument)
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

void BSequence::Compile(Compiler& compiler, const vector<ExprPtr>& args)
{
  for(auto& e : args) {
    compiler.result = nullptr;
    compiler.Visit(e.get());
  }
}

void BAssign::Compile(Compiler& compiler, const vector<ExprPtr>& args)
{
  compiler.Visit(args[1].get());
  if(!compiler.result)
    return;

  auto rightResult = compiler.result;
  compiler.result = nullptr;

  compiler.Visit(args[0].get());
  if(!compiler.result)
    return;

  compiler.builder.CreateStore(rightResult, compiler.result);
}

} // namespace xra

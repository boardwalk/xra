#include "common.hpp"
#include "typechecker.hpp"

namespace xra {

struct ParamCollector : Visitor<ParamCollector, const Expr>
{
  TypeSubst subst;

  void VisitEVariable(const EVariable& expr)
  {
    if(!subst.insert({expr.name, expr.type}).second)
      Error() << "duplicate parameter: " << expr.name;
  }

  void Visit(Base* base)
  {    
    if(base && !isa<EVariable>(base) && !isa<EList>(base))
      Error() << "only variables and lists allowed in function parameter";
    else
      base::Visit(base);
  }
};

void TypeChecker::VisitEVariable(EVariable& expr)
{
  expr.value = env[expr.name];
  if(!expr.value)
    Error() << "unbound variable " << expr.name;
}

void TypeChecker::VisitEBoolean(EBoolean& expr)
{
  expr.value = new VConstant;
  expr.value->type = BooleanType;
}

void TypeChecker::VisitEInteger(EInteger& expr)
{
  expr.value = new VConstant;
  expr.value->type = IntegerType;
}

void TypeChecker::VisitEFloat(EFloat& expr)
{
  expr.value = new VConstant;
  expr.value->type = FloatType;
}

void TypeChecker::VisitEString(EString& expr)
{
  expr.value = new VConstant;
  expr.value->type = StringType;
}

void TypeChecker::VisitEFunction(EFunction& expr)
{
  Scope scope(env);

  // tv <- newTyVar "a"
  ParamCollector paramCollector;
  paramCollector.Visit(expr.param.get());

  for(auto& param : paramCollector.subst) {
    if(!param.second)
      param.second = MakeTypeVar();
  }

  // TypeEnv env' = remove env n
  // env'' = TypeEnv (env' `Map.union` (Map.singleton n (Scheme [] tv)))
  for(auto& param : paramCollector.subst) {
    ValuePtr value = new VLocal;
    value->type = param.second;
    env.AddValue(param.first, value);
  }

  // ADDED
  Visit(expr.param.get());
  if(!expr.param->value)
    return;
  subst.clear();
  
  // (s1, t1) <- ti env'' e
  TypePtr lastReturnType = returnType;
  returnType = nullptr;
  bool lastInsideLoop = insideLoop;
  insideLoop = false;

  Visit(expr.body.get());
  if(!expr.body->value)
    return;

  if(returnType)
    Compose(Unify(*returnType, *expr.body->value->type), subst);

  returnType = lastReturnType;
  insideLoop = lastInsideLoop;

  // return (s1, TFun(apply s1 tv) t1)
  // MODIFIED (apply s1 tv) removed, unneeded
  expr.value = new VTemporary;
  expr.value->type = new TFunction(expr.param->value->type, expr.body->value->type);
}

void TypeChecker::VisitECall(ECall& expr)
{
  // (s1, t1) <- ti env e1
  Visit(expr.function.get());

  if(!expr.function->value)
    return;

  TypeSubst functionSubst;
  subst.swap(functionSubst);

  // apply s1 env
  env.Apply(functionSubst);

  auto builtin = dyn_cast<VBuiltin>(expr.function->value.get());
  if(builtin) {
    assert(isa<EList>(expr.argument.get()));
    auto& args = static_cast<EList&>(*expr.argument).exprs;
    expr.value = builtin->Infer(*this, args);
  }
  else {
    // tv <- newTyVar "a"
    expr.value = new VTemporary;
    expr.value->type = MakeTypeVar();

    // (s2, t2) <- ti (apply s1 env) e2
    Visit(expr.argument.get());

    if(!expr.argument->value)
      return;

    TypeSubst argumentSubst;
    subst.swap(argumentSubst);

    // s3 <- mgu (apply s2 t1) (TFun t2 tv)
    TypePtr leftType = Apply(argumentSubst, *expr.function->value->type);
    TypePtr rightType = new TFunction(expr.argument->value->type, expr.value->type);
    subst = Unify(*leftType, *rightType);

    // apply s3 tv
    expr.value->type = Apply(subst, *expr.value->type);

    // s3 `composeSubst` s2
    Compose(subst, argumentSubst);
  }

  // s2 `composeSubst` s1
  Compose(subst, functionSubst);
}

void TypeChecker::VisitEList(EList& expr)
{
  if(expr.exprs.empty()) {
    expr.value = new VConstant;
    expr.value->type = VoidType;
    return;
  }

  auto r = make_unique<TList>();

  for(auto& e : expr.exprs)
  {
    TypeSubst lastSubst;
    subst.swap(lastSubst);

    Visit(e.get());

    if(!e->value)
      return;

    Compose(lastSubst, subst);

    r->types.push_back(e->value->type);
  }

  expr.value = new VTemporary;
  expr.value->type = r.release();
}

void TypeChecker::VisitEExtern(EExtern& expr)
{
  ValuePtr value = new VExtern;
  value->type = expr.externType;
  env.AddValue(expr.name, value);

  expr.value = VoidValue;
}

void TypeChecker::Visit(Base* base)
{
  base::Visit(base);
  auto expr = static_cast<Expr*>(base);
  if(expr && expr->value && expr->type)
    Compose(Unify(*expr->value->type, *expr->type), subst);
}

} // namespace xra

#include "common.hpp"
#include "visitor.hpp"
#include "env.hpp"

namespace xra {

struct ParamCollector : Visitor<ParamCollector, const Expr>
{
  TypeSubst subst;

  void VisitVariable(const EVariable& expr)
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

struct InferVisitor : Visitor<InferVisitor, Expr>
{
  Env& env;
  TypeSubst subst;

  InferVisitor(Env& env_) :
    env(env_)
  {}

  void VisitVoid(EVoid& expr)
  {
    expr.value = new VConstant();
  }

  void VisitVariable(EVariable& expr)
  {
    expr.value = env[expr.name];
    if(!expr.value)
      Error() << "unbound variable " << expr.name;
  }

  void VisitBoolean(EBoolean& expr)
  {
    expr.value = new VConstant(expr.literal);
  }

  void VisitInteger(EInteger& expr)
  {
    expr.value = new VConstant(expr.literal);
  }

  void VisitFloat(EFloat& expr)
  {
    expr.value = new VConstant(expr.literal);
  }

  void VisitString(EString& expr)
  {
    expr.value = new VConstant(expr.literal);
  }

  void VisitFunction(EFunction& expr)
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
    InferVisitor paramVisitor(env);
    paramVisitor.Visit(expr.param.get());

    if(!expr.param->value)
      return;

    // (s1, t1) <- ti env'' e
    InferVisitor bodyVisitor(env);
    bodyVisitor.Visit(expr.body.get());

    if(!expr.body->value)
      return;

    // return (s1, TFun(apply s1 tv) t1)
    // MODIFIED (apply s1 tv) removed, unneeded
    Visit(expr.body.get());
    expr.value = new VTemporary;
    expr.value->type = new TFunction(expr.param->value->type, expr.body->value->type);
  }

  void VisitCall(ECall& expr)
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
      expr.value = builtin->Infer(env, subst, *expr.argument);
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

  void VisitList(EList& expr)
  {
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

  void VisitExtern(EExtern& expr)
  {
    ValuePtr value = new VExtern;
    value->type = expr.externType;
    env.AddValue(expr.name, value);

    expr.value = new VConstant;
  }

  void Visit(Base* base)
  {
    base::Visit(base);
    auto expr = static_cast<Expr*>(base);
    if(expr && expr->value && expr->type)
      Compose(Unify(*expr->value->type, *expr->type), subst);
  }
};

void Expr::Infer(Env& env, TypeSubst& subst)
{
  InferVisitor visitor(env);
  visitor.Visit(this);
  subst.swap(visitor.subst);
}

} // namespace xra

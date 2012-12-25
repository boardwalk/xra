#include "common.hpp"
#include "expr.hpp"
#include "expr-visitor.hpp"
#include "env.hpp"

namespace xra {

struct ParamCollector : ExprVisitor<ParamCollector, const Expr>
{
  TypeSubst subst;
  string error;

  void VisitVariable(const EVariable& expr)
  {
    if(!subst.insert({expr.name, expr.type}).second)
      error = "duplicate parameter";
  }

  void Visit(Expr& expr)
  {
    base::Visit(expr);
    if(!isa<EVariable>(&expr) && !isa<EList>(&expr))
      error = "expression not allowed in function parameter";
  }
};

struct InferVisitor : ExprVisitor<InferVisitor, Expr>
{
  Env& env;
  TypeSubst subst;

  InferVisitor(Env& env_) :
    env(env_)
  {}

  void VisitError(EError& expr)
  {
    expr.value = new VError("expression error");
  }

  void VisitVoid(EVoid& expr)
  {
    expr.value = new VConstant();
  }

  void VisitVariable(EVariable& expr)
  {
    expr.value = env[expr.name];
    if(!expr.value)
      expr.value = new VError("unbound variable");
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
    paramCollector.Visit(*expr.param);

    if(!paramCollector.error.empty()) {
      expr.value = new VError(paramCollector.error);
      return;
    }

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
    paramVisitor.Visit(*expr.param);

    // (s1, t1) <- ti env'' e
    InferVisitor bodyVisitor(env);
    bodyVisitor.Visit(*expr.body);

    // return (s1, TFun(apply s1 tv) t1)
    // MODIFIED (apply s1 tv) removed, unneeded
    Visit(*expr.body);
    expr.value = new VTemporary;
    expr.value->type = new TFunction(expr.param->value->type, expr.body->value->type);
  }

  void VisitCall(ECall& expr)
  {
    // (s1, t1) <- ti env e1
    InferVisitor functionVisitor(env);
    functionVisitor.Visit(*expr.function);

    // apply s1 env
    env.Apply(functionVisitor.subst);

    auto builtin = dyn_cast<VBuiltin>(expr.function->value.get());
    if(builtin) {
      expr.value = builtin->Infer(env, subst, *expr.argument);
    }
    else {
      // tv <- newTyVar "a"
      expr.value = new VTemporary;
      expr.value->type = MakeTypeVar();

      // (s2, t2) <- ti (apply s1 env) e2
      InferVisitor argumentVisitor(env);
      argumentVisitor.Visit(*expr.argument);

      // s3 <- mgu (apply s2 t1) (TFun t2 tv)
      TypePtr leftType = Apply(argumentVisitor.subst, *expr.function->value->type);
      TypePtr rightType = new TFunction(expr.argument->value->type, expr.value->type);
      subst = Unify(*leftType, *rightType);

      // apply s3 tv
      expr.value->type = Apply(subst, *expr.value->type);

      // s3 `composeSubst` s2
      Compose(subst, argumentVisitor.subst);
    }

    // s2 `composeSubst` s1
    Compose(subst, functionVisitor.subst);
  }

  void VisitList(EList& expr)
  {
    auto r = make_unique<TList>();

    for(auto& e : expr.exprs)
    {
      TypeSubst lastSubst;
      subst.swap(lastSubst);

      Visit(*e);

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

  void Visit(Expr& expr)
  {
    base::Visit(expr);
    if(expr.type)
      Compose(Unify(*expr.value->type, *expr.type), subst);
  }
};

void Expr::Infer(Env& env, TypeSubst& subst)
{
  InferVisitor visitor(env);
  visitor.Visit(*this);
  subst.swap(visitor.subst);
}

} // namespace xra

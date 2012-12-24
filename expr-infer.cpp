#include "common.hpp"
#include "expr.hpp"
#include "expr-visitor.hpp"
#include "env.hpp"

namespace xra {

struct ParamCollector : ExprVisitor<ParamCollector, const Expr>
{
  TypeSubst subst;
  TypePtr error;

  void VisitVariable(const EVariable& expr)
  {
    if(!subst.insert({expr.name, expr.type}).second)
      error.reset(new TError("duplicate parameter"));
  }

  void Visit(Expr& expr)
  {
    base::Visit(expr);
    if(!isa<EVariable>(&expr) && !isa<EList>(&expr))
      error.reset(new TError("expression not allowed in function parameter"));
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
    expr.finalType.reset(new TError("expression error"));
  }

  void VisitVoid(EVoid& expr)
  {
    expr.finalType = VoidType;
  }

  void VisitVariable(EVariable& expr)
  {
    auto value = env[expr.name];
    if(value)
      expr.finalType = value->typeScheme.type;
    else
      expr.finalType.reset(new TError("unbound variable"));
  }

  void VisitBoolean(EBoolean& expr)
  {
    expr.finalType = BooleanType;
  }

  void VisitInteger(EInteger& expr)
  {
    expr.finalType = IntegerType;
  }

  void VisitFloat(EFloat& expr)
  {
    expr.finalType = FloatType;
  }

  void VisitString(EString& expr)
  {
    expr.finalType = StringType;
  }

  void VisitFunction(EFunction& expr)
  {
    Scope scope(env);

    // tv <- newTyVar "a"
    ParamCollector paramCollector;
    paramCollector.Visit(*expr.param);

    if(paramCollector.error) {
      expr.finalType = paramCollector.error;
      return;
    }

    for(auto& param : paramCollector.subst) {
      if(!param.second)
        param.second = MakeTypeVar();
    }

    // TypeEnv env' = remove env n
    // env'' = TypeEnv (env' `Map.union` (Map.singleton n (Scheme [] tv)))
    for(auto& param : paramCollector.subst) {
      ValuePtr value(new VLocal);
      value->typeScheme.type = param.second;
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
    assert(expr.param->finalType);
    assert(expr.body->finalType);
    expr.finalType.reset(new TFunction(expr.param->finalType, expr.body->finalType));
  }

  void VisitCall(ECall& expr)
  {
    Scope scope(env);

    // (s1, t1) <- ti env e1
    InferVisitor functionVisitor(env);
    functionVisitor.Visit(*expr.function);

    // (s2, t2) <- ti (apply s1 env) e2
    env.Apply(functionVisitor.subst);

    InferVisitor argumentVisitor(env);
    argumentVisitor.Visit(*expr.argument);

    // tv <- newTyVar "a"
    expr.finalType = MakeTypeVar();
    // s3 <- mgu (apply s2 t1) (TFun t2 tv) 
    TypePtr leftType(Apply(argumentVisitor.subst, *expr.function->finalType));
    TypePtr rightType(new TFunction(expr.argument->finalType, expr.finalType));
    subst = Unify(*leftType, *rightType);

    // s3 `composeSubst` s2 `composeSubst` s1
    Compose(subst, argumentVisitor.subst);
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

      Compose(subst, lastSubst);
      subst.swap(lastSubst);

      r->types.push_back(e->finalType);
    }

    expr.finalType.reset(r.release());
  }

  void VisitExtern(EExtern& expr)
  {
    ValuePtr value(new VExtern);
    value->typeScheme.type = expr.externType;
    env.AddValue(expr.name, value);
    expr.finalType = VoidType;
  }

  void Visit(Expr& expr)
  {
    base::Visit(expr);
    if(expr.type)
      Compose(Unify(*expr.finalType, *expr.type), subst);
  }
};

void Expr::Infer(Env& env)
{
  InferVisitor visitor(env);
  visitor.Visit(*this);
}

} // namespace xra

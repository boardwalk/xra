#include "common.hpp"
#include "expr.hpp"
#include "expr-visitor.hpp"

namespace xra {

struct InferVisitor : ExprVisitor<InferVisitor, Expr>
{
  TypeEnv& env;
  TypeSubst subst;

  InferVisitor(TypeEnv& env_) :
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
    auto it = env.find(expr.name);
    if(it != env.end())
      expr.finalType = it->second.type;
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
    // TODO
    base::VisitFunction(expr);
  }

  void VisitCall(ECall& expr)
  {
    TypeEnv::lock lock(env);

    // (s1, t1) <- t1 env e1
    InferVisitor functionVisitor(env);
    functionVisitor.VisitAny(*expr.function);

    // (s2, t2) <- t1 (apply s1 env) e2
    Apply(functionVisitor.subst, env);

    InferVisitor argumentVisitor(env);
    argumentVisitor.VisitAny(*expr.argument);

    auto builtin = dyn_cast<TBuiltin>(expr.function->finalType.get());
    if(builtin) {
      expr.finalType = builtin->Infer(expr.argument->finalType, subst);
    }
    else {
      // tv <- newTyVar "a"
      expr.finalType = MakeTypeVar();
      // s3 <- mgu (apply s2 t1) (TFun t2 tv) 
      TypePtr leftType(Apply(argumentVisitor.subst, *expr.function->finalType));
      TypePtr rightType(new TFunction(expr.argument->finalType, expr.finalType));
      subst = Unify(*leftType, *rightType);
    }

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

      VisitAny(*e);
      Apply(subst, env);

      Compose(subst, lastSubst);
      subst.swap(lastSubst);

      // TODO children must infer something!
      if(e->finalType)
        r->types.push_back(e->finalType);
      else
        r->types.push_back(VoidType);
    }

    expr.finalType.reset(r.release());
  }

  void VisitExtern(EExtern& expr)
  {
    auto it = env.find(expr.name);
    if(it != env.end()) {
      env[expr.name].type = expr.externType;
      expr.finalType = VoidType;
    }
    else {
      expr.finalType.reset(new TError("redefinition of symbol"));
    }
  }

  void VisitAny(Expr& expr)
  {
    base::VisitAny(expr);
    if(expr.type)
      Compose(Unify(*expr.finalType, *expr.type), subst);
  }
};

void Expr::Infer()
{
  TypeEnv env;
  SetBuiltins(env);
  InferVisitor visitor(env);
  visitor.VisitAny(*this);
}

} // namespace xra
